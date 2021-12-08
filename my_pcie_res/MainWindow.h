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
	QLineSeries *curSeries; //��ǰ����
	QValueAxis *curAxis; //��ǰ������

	int maxSize;
	QList<int> xdatas;                 //�洢�������ݣ�ʹ��list���Ժܷ������ɾ
	double ydata;
	//QList<double> ydata;                 //�洢�������ݣ�ʹ��list���Ժܷ������ɾ

	EventThread eventThread;



	void    createChart(); //����ͼ��
	//void    updataChart(); //��������

private slots:

	void updataChart();

	//�������������ܵ��ú���
	void on_act_start_triggered();

};
