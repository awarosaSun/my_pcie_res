
#ifndef PCIE_FUNC_H
#define PCIE_FUNC_H
#ifdef __cplusplus
extern "C" {
#endif

	extern unsigned int image_h;
	extern unsigned int image_v;
	extern unsigned char *h2c_align_mem_tmp;
	extern unsigned char *c2h_align_mem_tmp;

	void c2h_transfer(unsigned int size, unsigned char *buffer);

	int  pcie_init();
	void pcie_deinit();
	int  open_event();
	int  wait_for_event0();
	int  wait_for_event1();

	struct daq7606
	{
		short ad0;
		short ad1;
	};

#ifdef __cplusplus
}
#endif
#endif



