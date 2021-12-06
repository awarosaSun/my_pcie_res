#ifdef __cplusplus
extern "C" {
#endif
#include <Windows.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <strsafe.h>
#include <stdint.h>
#include <SetupAPI.h>
#include <INITGUID.H>
#include <WinIoCtl.h>
#include "pcie_fun.h"
	//#include <AtlBase.h>
#include <io.h>
#include "xdma_public.h"

#pragma comment(lib, "setupapi.lib")

#define FPGA_DDR_START_ADDR 0x00000000
#define MAX_BYTES_PER_TRANSFER 0x800000

	static HANDLE h_c2h0;
	static HANDLE h_h2c0;
	static HANDLE h_user;
	static HANDLE h_event0;
	static HANDLE h_event1;
	static HANDLE h_event2;
	static HANDLE h_event3;

	static DWORD  user_irq_ack;
	static char   base_path[MAX_PATH + 1] = "";
	static unsigned int c2h_axi4_addr[2];

	unsigned char *h2c_align_mem_tmp;
	unsigned char *c2h_align_mem_tmp;

	unsigned int image_h;
	unsigned int image_v;

	static unsigned char fbuf;

	LARGE_INTEGER start;
	LARGE_INTEGER stop;
	LARGE_INTEGER freq;


	static int verbose_msg(const char* const fmt, ...) {

		int ret = 0;
		va_list args;
		if (1) {
			va_start(args, fmt);
			ret = vprintf(fmt, args);
			va_end(args);
		}
		return ret;

	}
	static BYTE* allocate_buffer(size_t size, size_t alignment) {

		if (size == 0) {
			size = 4;
		}

		if (alignment == 0) {
			SYSTEM_INFO sys_info;
			GetSystemInfo(&sys_info);
			alignment = sys_info.dwPageSize;
			//printf("alignment = %d\n",alignment);
		}
		verbose_msg("Allocating host-side buffer of size %d, aligned to %d bytes\n", size, alignment);
		return (BYTE*)_aligned_malloc(size, alignment);

	}

	static int get_devices(GUID guid, char* devpath, size_t len_devpath) {

		SP_DEVICE_INTERFACE_DATA device_interface;
		PSP_DEVICE_INTERFACE_DETAIL_DATA dev_detail;
		DWORD index;
		HDEVINFO device_info;
		wchar_t tmp[256];
		device_info = SetupDiGetClassDevs((LPGUID)&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
		if (device_info == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "GetDevices INVALID_HANDLE_VALUE\n");
			exit(-1);
		}

		device_interface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		// enumerate through devices

		for (index = 0; SetupDiEnumDeviceInterfaces(device_info, NULL, &guid, index, &device_interface); ++index) {

			// get required buffer size
			ULONG detailLength = 0;
			if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_interface, NULL, 0, &detailLength, NULL) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
				fprintf(stderr, "SetupDiGetDeviceInterfaceDetail - get length failed\n");
				break;
			}

			// allocate space for device interface detail
			dev_detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, detailLength);
			if (!dev_detail) {
				fprintf(stderr, "HeapAlloc failed\n");
				break;
			}
			dev_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			// get device interface detail
			if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_interface, dev_detail, detailLength, NULL, NULL)) {
				fprintf(stderr, "SetupDiGetDeviceInterfaceDetail - get detail failed\n");
				HeapFree(GetProcessHeap(), 0, dev_detail);
				break;
			}

			StringCchCopy(tmp, len_devpath, dev_detail->DevicePath);
			wcstombs(devpath, tmp, 256);
			HeapFree(GetProcessHeap(), 0, dev_detail);
		}

		SetupDiDestroyDeviceInfoList(device_info);

		return index;
	}


	HANDLE open_devices(char* device_base_path, char* device_name, DWORD accessFlags)
	{
		char device_path[MAX_PATH + 1] = "";
		wchar_t device_path_w[MAX_PATH + 1];
		HANDLE h;

		// extend device path to include target device node (xdma_control, xdma_user etc)
		verbose_msg("Device base path: %s\n", device_base_path);
		strcpy_s(device_path, sizeof device_path, device_base_path);
		strcat_s(device_path, sizeof device_path, device_name);
		verbose_msg("Device node: %s\n", device_name);
		// open device file
		mbstowcs(device_path_w, device_path, sizeof(device_path));
		h = CreateFile(device_path_w, accessFlags, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE)
		{
			fprintf(stderr, "Error opening device, win32 error code: %ld\n", GetLastError());
		}

		return h;
	}

	static int read_device(HANDLE device, long address, DWORD size, BYTE *buffer)
	{
		DWORD rd_size = 0;

		unsigned int transfers;
		unsigned int i;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(device, address, NULL, FILE_BEGIN)) {
			fprintf(stderr, "Error setting file pointer, win32 error code: %ld\n", GetLastError());
			return -3;
		}
		transfers = (unsigned int)(size / MAX_BYTES_PER_TRANSFER);
		for (i = 0; i < transfers; i++)
		{
			if (!ReadFile(device, (void *)(buffer + i * MAX_BYTES_PER_TRANSFER), (DWORD)MAX_BYTES_PER_TRANSFER, &rd_size, NULL))
			{
				return -1;
			}
			if (rd_size != MAX_BYTES_PER_TRANSFER)
			{
				return -2;
			}
		}
		if (!ReadFile(device, (void *)(buffer + i * MAX_BYTES_PER_TRANSFER), (DWORD)(size - i * MAX_BYTES_PER_TRANSFER), &rd_size, NULL))
		{
			return -1;
		}
		if (rd_size != (size - i * MAX_BYTES_PER_TRANSFER))
		{
			return -2;
		}

		return size;
	}

	static int write_device(HANDLE device, long address, DWORD size, BYTE *buffer)
	{
		DWORD wr_size = 0;
		unsigned int transfers;
		unsigned int i;
		transfers = (unsigned int)(size / MAX_BYTES_PER_TRANSFER);

		if (INVALID_SET_FILE_POINTER == SetFilePointer(device, address, NULL, FILE_BEGIN)) {
			fprintf(stderr, "Error setting file pointer, win32 error code: %ld\n", GetLastError());
			return -3;
		}
		for (i = 0; i < transfers; i++)
		{
			if (!WriteFile(device, (void *)(buffer + i * MAX_BYTES_PER_TRANSFER), MAX_BYTES_PER_TRANSFER, &wr_size, NULL))
			{
				return -1;
			}
			if (wr_size != MAX_BYTES_PER_TRANSFER)
			{
				return -2;
			}
		}
		if (!WriteFile(device, (void *)(buffer + i * MAX_BYTES_PER_TRANSFER), (DWORD)(size - i * MAX_BYTES_PER_TRANSFER), &wr_size, NULL))
		{
			return -1;
		}
		if (wr_size != (size - i * MAX_BYTES_PER_TRANSFER))
		{
			return -2;
		}
		return size;
	}


	void c2h_transfer(unsigned int size, unsigned char *buffer)
	{
		read_device(h_c2h0, c2h_axi4_addr[fbuf], size, buffer);
	}


	int open_event()
	{
		char* event0_name = "\\event_0";
		char* event1_name = "\\event_1";

		h_event0 = open_devices(base_path, event0_name, GENERIC_READ);
		if (h_event0 == INVALID_HANDLE_VALUE) return 0;
		h_event1 = open_devices(base_path, event1_name, GENERIC_READ);
		if (h_event1 == INVALID_HANDLE_VALUE) return 0;

		return 1;

	}

	int wait_for_event0()
	{
		BYTE val;
		read_device(h_event0, 0, 1, (BYTE *)&val);//waite irq
		fbuf = 0;
		Sleep(1);
		return 1;
	}
	int wait_for_event1()
	{
		BYTE val;
		read_device(h_event1, 0, 1, (BYTE *)&val);//waite irq
		fbuf = 1;
		Sleep(1);
		return 1;
	}


	int pcie_init()
	{
		char* user_name = "\\user";
		char* c2h0_name = "\\c2h_0";
		char* h2c0_name = "\\h2c_0";

		int res = 1;
		fbuf = 0;

		DWORD num_devices = get_devices(GUID_DEVINTERFACE_XDMA, base_path, sizeof(base_path));
		verbose_msg("Devices found: %d\n", num_devices);
		if (num_devices < 1)
		{
			printf("error\n");
			return 0;
		}

		h_user = open_devices(base_path, user_name, GENERIC_READ | GENERIC_WRITE);
		if (h_user == INVALID_HANDLE_VALUE) return 0;

		h_h2c0 = open_devices(base_path, h2c0_name, GENERIC_READ | GENERIC_WRITE);
		if (h_c2h0 == INVALID_HANDLE_VALUE) return 0;

		h_c2h0 = open_devices(base_path, c2h0_name, GENERIC_READ | GENERIC_WRITE);
		if (h_c2h0 == INVALID_HANDLE_VALUE) return 0;


		h2c_align_mem_tmp = allocate_buffer(0x800000, 4096);
		c2h_align_mem_tmp = allocate_buffer(0x800000, 4096);
		c2h_axi4_addr[0] = 0;
		c2h_axi4_addr[1] = 16384;

		if (NULL == h2c_align_mem_tmp || NULL == c2h_align_mem_tmp) return 0;


		open_event();
		user_irq_ack = 0xffff0000;
		write_device(h_user, 0x04, 4, (BYTE *)&user_irq_ack);//start irq
		return res;
	}
	void pcie_deinit()
	{
		user_irq_ack = 0x00000000;
		write_device(h_user, 0x04, 4, (BYTE *)&user_irq_ack);//clear irq
		CloseHandle(h_user);
		CloseHandle(h_c2h0);
		CloseHandle(h_h2c0);
		CloseHandle(h_event0);
		CloseHandle(h_event1);
	}



#ifdef __cplusplus
}
#endif
