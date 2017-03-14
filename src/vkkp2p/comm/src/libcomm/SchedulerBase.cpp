#include "SchedulerBase.h"
#include "IOReactor.h"
#include "Timer.h"

SchedulerBase::SchedulerBase(void)
:m_brun(false)
,m_i(0)
{
}

SchedulerBase::~SchedulerBase(void)
{
	assert(0==m_i);
}
int SchedulerBase::run()
{
	if(m_brun)
		return 1;
	m_brun = true;
	this->activate();
	return 0;
}
void SchedulerBase::end()
{
	if(!m_brun)
		return;
	m_brun = false;
	wait();
}
int SchedulerBase::work(int e)
{
	ULONGLONG delay_usec = 0;
	bool bhave_task,bhave_delay;
	int ret;
	while(m_brun)
	{
		bhave_task = false;
		bhave_delay = false;
		for(int i=0;i<m_i;++i)
		{
			ret = m_handles[i]->handle_root();
			if(0==ret) bhave_task = true;
			else if(1==ret) bhave_delay = true;
		}

		TimerSngl::instance()->handle_root();
		delay_usec = TimerSngl::instance()->get_remain_us();
		delay_usec = delay_usec/2;
		if(delay_usec>30000) delay_usec=30000;
		if(bhave_task) 
			delay_usec = 0;
		else if(bhave_delay) 
		{
			if(delay_usec>1000) delay_usec = 1000;
		}
		IOReactorSngl::instance()->handle_root(delay_usec);	

	}
	return 0;
}

int SchedulerBase::add_handle(SchedulerHandler* h)
{
	if(m_i>=SCHEDULER_MAX_HANDLE)
		return -1;
	for(int i=0;i<m_i;++i)
	{
		if(m_handles[i]==h)
			return 0;
	}
	m_handles[m_i] = h;
	m_i++;
	return 0;
}
int SchedulerBase::remove_handle(SchedulerHandler* h)
{
	for(int i=0;i<m_i;++i)
	{
		if(m_handles[i]==h)
		{
			m_handles[i] = m_handles[m_i-1];
			m_i--;
			return 0;
		}
	}
	return 0;
}
