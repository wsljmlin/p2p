#include "uac_Scheduler.h"
#include "uac_Timer.h"
#include "uac_UDPAcceptor.h"
#include "uac_sockpool.h"

namespace UAC
{
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
	while(m_brun)
	{
		TimerSngl::instance()->handle_root();

		delay_usec = TimerSngl::instance()->get_remain_us();
		delay_usec = delay_usec/2;
		if(delay_usec>30000) delay_usec=30000;
		
		sockpoolsngl::instance()->handle_root((long)delay_usec);
	}
	return 0;
}

}

