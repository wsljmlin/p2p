#include "Scheduler.h"
#include "PeerManager.h"
#include "UserManager.h"
#include "Interface.h"

Scheduler::Scheduler(void)
:m_brun(false)
,m_threadnum(0)
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
	m_threadnum = 1;
	this->activate(m_threadnum);
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
	if(1==m_threadnum)
	{
		while(m_brun)
		{
			PeerManagerSngl::instance()->handle_root();
			UserManagerSngl::instance()->handle_root();
			TIF::instance()->handle_root();
			Sleep(0);
		}
	}
	else if(2==m_threadnum)
	{
		if(0==e)
		{
			while(m_brun)
			{
				PeerManagerSngl::instance()->handle_root();
				Sleep(0);
			}

		}
		else if(1==e)
		{
			while(m_brun)
			{
				UserManagerSngl::instance()->handle_root();
				TIF::instance()->handle_root();
				Sleep(8);
			}
		}
	}
	return 0;
}
