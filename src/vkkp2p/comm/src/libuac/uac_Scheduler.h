#pragma once
#include "uac_Thread.h"
#include "uac_Singleton.h"
namespace UAC
{
class Scheduler : public Thread
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
typedef Singleton<Scheduler> SchedulerSngl;
}

