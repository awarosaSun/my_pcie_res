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
	//����ͼ��ĸ�������
	QChart *chart = new QChart();
	chart->setTitle(tr("simple curve"));
	//    chart->setAcceptHoverEvents(true);
	ui->chartView->setChart(chart);
	ui->chartView->setRenderHint(QPainter::Antialiasing);//��ֹ���߱��Σ���������Բ��

	QLineSeries *series0 = new QLineSeries();
	series0->setName("data curve");

	curSeries = series0; //��ǰ����

	//������ʽ�趨
	QPen    pen;
	pen.setStyle(Qt::DotLine);//Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine
	pen.setWidth(2);
	pen.setColor(Qt::red);
	series0->setPen(pen); //�������е���������

	pen.setStyle(Qt::SolidLine);//Qt::SolidLine, Qt::DashLine, Qt::DotLine, Qt::DashDotLine
	pen.setColor(Qt::blue);

	chart->addSeries(series0);

	QValueAxis *axisX = new QValueAxis;
	curAxis = axisX; //��ǰ������
	axisX->setRange(0, 10); //���������᷶Χ
	axisX->setLabelFormat("%.1f"); //��ǩ��ʽ
	axisX->setTickCount(11); //���ָ�����
	axisX->setMinorTickCount(4);
	axisX->setTitleText("time(secs)"); //����
//    axisX->setGridLineVisible(false);

	QValueAxis *axisY = new QValueAxis;
	axisY->setRange(0, 20);
	axisY->setTitleText("value");
	axisY->setTickCount(5);
	axisY->setLabelFormat("%.2f"); //��ǩ��ʽ
//    axisY->setGridLineVisible(false);
	axisY->setMinorTickCount(4);

	chart->setAxisX(axisX, series0); //���X������
	chart->setAxisY(axisY, series0); //���Y������

	//��1Ϊ����
	maxSize = 10;
	for (int i = 0; i < maxSize; i++) {
		xdatas.append(i);
	}

}

void MainWindow::updataChart()
{
	qDebug() << "come into update";
	//Ϊ������������
	QLineSeries *series0 = (QLineSeries *)ui->chartView->chart()->series().at(0);

	series0->clear(); //�������


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

