#pragma once

#include "pcie_fun.h"
#include <QThread>


class EventThread:QThread
{
	/*Q_OBJECT


protected:
	void run()
		Q_DECL_OVERRIDE;


signals:*/
	Q_OBJECT

signals:
		void sig_inform_disp();

public:
	volatile bool isplay;
	volatile char event_id;
	EventThread();
	~EventThread();
	void run();

public slots:

};

