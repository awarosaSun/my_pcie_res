#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCharts>
#include <QChartView>
#include "ui_MainWindow.h"
#include "EventThread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();


private:
    Ui::MainWindowClass *ui;


	//my code
private:
	QLineSeries *curSeries; //当前序列
	QValueAxis *curAxis; //当前坐标轴

	int maxSize;
	QList<int> xdatas;                 //存储坐标数据，使用list可以很方便的增删
	double ydata;
	//QList<double> ydata;                 //存储坐标数据，使用list可以很方便的增删

	EventThread eventThread;



	void    createChart(); //创建图表
	//void    updataChart(); //更新数据

private slots:

	void updataChart();

	//工具栏按键功能调用函数
	void on_act_start_triggered();

};
