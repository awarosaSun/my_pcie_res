#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "EventThread.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();

	int first_time;
	int cnt;

	EventThread		pcie_c2h_event0;
	EventThread		pcie_c2h_event1;

private:
    Ui::MainWindowClass *ui;

	QVector<double> xdata;
	QVector<double> ydata;

private slots:
	void plot_figure();
};
