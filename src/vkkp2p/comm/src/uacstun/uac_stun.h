#pragma once

#include "uac_Thread.h"
#include "uac_Singleton.h"

class Scheduler : public UAC::Thread
{
public:
	Scheduler(void);
	virtual ~Scheduler(void);

	int run();
	void end();
	virtual int work(int e);
private:
	bool	m_brun;
};
typedef UAC::Singleton<Scheduler> SchedulerSngl;
