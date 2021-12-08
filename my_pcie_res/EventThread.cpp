#include "EventThread.h"



EventThread::EventThread()
{

}

EventThread::~EventThread()
{

}


void EventThread::run()
{
	
	qDebug() << "come into run";
	if (this->isplay == true)
	{
		qDebug() << "come into run_if";
		emit sig_inform_disp();
		this->isplay = false;
	}

}

