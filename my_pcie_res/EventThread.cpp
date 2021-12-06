#include "EventThread.h"

void EventThread::run()
{
	
	while (this->isplay == true)
	{
		if (this->event_id == 0)
			wait_for_event0();
		else if (this->event_id == 1)
			wait_for_event1();
		emit sig_inform_disp();
	}

	quit();
}
