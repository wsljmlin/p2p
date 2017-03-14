#include "Scheduler.h"
#include "PeerManager.h"

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
	this->activate(1);
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
	while(m_brun)
	{
		PeerManagerSngl::instance()->handle_root();
		Sleep(0);
	}
	return 0;
}

