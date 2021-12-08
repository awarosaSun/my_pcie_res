#pragma once

#include "pcie_fun.h"
#include <QThread>
#include <QDebug>

class EventThread:public QThread
{

	Q_OBJECT

public:
	EventThread();
	~EventThread();

	volatile bool isplay;
	//volatile char event_id;

	void run();

signals:
	void sig_inform_disp();

};

