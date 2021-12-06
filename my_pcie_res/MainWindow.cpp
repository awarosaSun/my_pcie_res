#include "MainWindow.h"
#include <QMessageBox>
#include "pcie_fun.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)

{
    ui->setupUi(this);


	this->setWindowTitle(tr("My_Test_Wave(V1.01)"));

	//setupRealtimeDataDemo(ui->qwtPlot);

	if (pcie_init() < 0)
	{
		QMessageBox::information(this, "ERROR", "pcie init error");
		return;
	}

	connect(&pcie_c2h_event0, SIGNAL(sig_inform_disp()), this, SLOT(updatedataSlot()));
	connect(&pcie_c2h_event1, SIGNAL(sig_inform_disp()), this, SLOT(updatedataSlot()));

	if (!pcie_c2h_event0.isRunning()) pcie_c2h_event0.start();
	if (!pcie_c2h_event1.isRunning()) pcie_c2h_event1.start();

	pcie_c2h_event0.isplay = true;
	pcie_c2h_event1.isplay = true;
	ad_cave = ((double)20.0) / 65536;

	first_time = 1;
	cnt = 0;
}

MainWindow::~MainWindow()
{
	pcie_c2h_event0.isplay = false;
	pcie_c2h_event1.isplay = false;

	if (pcie_c2h_event0.isRunning())
	{
		pcie_c2h_event0.terminate();
		pcie_c2h_event0.wait();
	}
	if (pcie_c2h_event1.isRunning())
	{
		pcie_c2h_event1.terminate();
		pcie_c2h_event1.wait();
	}
	pcie_deinit();
	delete ui;
}


//¸üÐÂÉ¢µãÍ¼
void MainWindow::updatedataSlot() {

	double val1, val2;
	//double val1,val2,val3,val4;
	QString str;
	if (cnt == 0)
	{

		c2h_transfer(16384, c2h_align_mem_tmp);
		daq7606 *ptmp = (daq7606 *)c2h_align_mem_tmp;
		buf = ptmp;
		val1 = 0;
		val2 = 0;
		ydata.erase(ydata.begin(), ydata.begin() + 4096);
	}

	while (cnt <= 4095)
	{
		val1 = (buf->ad0)*ad_cave;
		val2 = (buf->ad1)*ad_cave;
		if (cnt == 0)
		{
			str = QString("%1 %2").arg("ad0 = ").arg(val1);
			ui->label->setText(str);
			str = QString("%1 %2").arg("ad1 = ").arg(val2);
			ui->label_2->setText(str);
		}

		ydata.append(val2);
		buf++;
		cnt++;
	}
	cnt = 0;
	curve->setSamples(xdata, ydata);
	curve->attach(ui->qwtPlot);
	ui->qwtPlot->replot();

}
