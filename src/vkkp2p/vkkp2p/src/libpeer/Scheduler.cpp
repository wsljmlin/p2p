#include "Scheduler.h"
#include "IOReactor.h"
#include "Timer.h"
#include "Interface.h"
#include "uac_SocketSelector.h"

Scheduler::Scheduler(void)
:m_brun(false)
{
}

Scheduler::~Scheduler(void)
{
}
int Scheduler::run()
{
	if(m_brun)
		return 1;
	m_brun = true;
	this->activate();
	return 0;
}
void Scheduler::end()
{
	if(!m_brun)
		return;
	m_brun = false;
	wait();
}
int Scheduler::work(int e)
{
	ULONGLONG delay_usec = 0;
	int ret;
	while(m_brun)
	{
		TimerSngl::instance()->handle_root();
		delay_usec = TimerSngl::instance()->get_remain_us();
		delay_usec = delay_usec/2;
		if(delay_usec>1000) delay_usec=1000;

		UAC_SocketSelectorSngl::instance()->handle_accept();
		ret = UAC_SocketSelectorSngl::instance()->handle_readwrite();
		if(-1!=ret) delay_usec = 0;
		IOReactorSngl::instance()->handle_root(delay_usec);
		PIF::instance()->handle_root();
		//Sleep(0);
	}
	return 0;
}

