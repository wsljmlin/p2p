#pragma once
#include "commons.h"

class Scheduler : public Thread
{
public:
	Scheduler(void);
	virtual ~Scheduler(void);

	int run();
	void end();
	virtual int work(int e);
public:
	bool m_brun;
};
typedef Singleton<Scheduler> SchedulerSngl;
