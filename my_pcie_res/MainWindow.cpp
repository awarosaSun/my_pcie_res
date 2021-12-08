#include "MainWindow.h"
#include <QMessageBox>
#include "pcie_fun.h"
#include <QCloseEvent>
#include <QDebug>

QT_CHARTS_USE_NAMESPACE


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass)

{
    ui->setupUi(this);

	this->setWindowTitle(tr("My_Test_Wave(V1.01)"));

	createChart();

	connect(&eventThread, SIGNAL(sig_inform_disp()), this, SLOT(updataChart()));

	//if (!eventThread.isRunning()) eventThread.start();

	eventThread.isplay = false;

}



MainWindow::~MainWindow()
{
	eventThread.isplay = false;

	if (eventThread.isRunning())
	{
		eventThread.terminate();
		eventThread.wait();
	}

	pcie_deinit();
	delete ui;
}

void MainWindow::createChart()
{
	qDebug() << "come into create";
	//创建图表的各个部件
	QChart *chart = new QChart();
	chart->setTitle(tr("simple curve"));
	//    chart->setAcceptHoverEvents(true);
	ui->chartView->setChart(chart);
	ui->chartView->setRenderHint(QPainter::Antialiasing);//防止曲线变形，保持曲线圆滑

	QLineSeries *series0 = new QLineSeries();
	series0->setName("data curve");

	curSeries = series0; //当前序列

	//线条样式设定
	QPen    pen;
	pen.setStyle(Qt::DotLine);//Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine
	pen.setWidth(2);
	pen.setColor(Qt::red);
	series0->setPen(pen); //折线序列的线条设置

	pen.setStyle(Qt::SolidLine);//Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine
	pen.setColor(Qt::blue);

	chart->addSeries(series0);

	QValueAxis *axisX = new QValueAxis;
	curAxis = axisX; //当前坐标轴
	axisX->setRange(0, 10); //设置坐标轴范围
	axisX->setLabelFormat("%.1f"); //标签格式
	axisX->setTickCount(11); //主分隔个数
	axisX->setMinorTickCount(4);
	axisX->setTitleText("time(secs)"); //标题
//    axisX->setGridLineVisible(false);

	QValueAxis *axisY = new QValueAxis;
	axisY->setRange(0, 20);
	axisY->setTitleText("value");
	axisY->setTickCount(5);
	axisY->setLabelFormat("%.2f"); //标签格式
//    axisY->setGridLineVisible(false);
	axisY->setMinorTickCount(4);

	chart->setAxisX(axisX, series0); //添加X坐标轴
	chart->setAxisY(axisY, series0); //添加Y坐标轴

	//以1为步长
	maxSize = 10;
	for (int i = 0; i < maxSize; i++) {
		xdatas.append(i);
	}

}

void MainWindow::updataChart()
{
	qDebug() << "come into update";
	//为序列生成数据
	QLineSeries *series0 = (QLineSeries *)ui->chartView->chart()->series().at(0);

	series0->clear(); //清除数据


	for (int i = 0; i < maxSize; i++)
	{
		ydata = (double)(i*i / 10.0);
		series0->append(xdatas.at(i), ydata);
	}
}


void MainWindow::on_act_start_triggered()
{
	if (!eventThread.isRunning()) eventThread.start();
	eventThread.isplay = true;
	qDebug() << "update triggered";
}

