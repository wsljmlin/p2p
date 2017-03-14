#pragma once

#include "comms.h"

#define SCHEDULER_MAX_HANDLE  20
class SchedulerHandler
{
public:
	virtual ~SchedulerHandler(){}
	//unblocking root
	virtual int handle_root()=0; //-1:表示内部处理空闲，0:表示处理过任务,1:表示有任务延后处理,即让休息一下
};

//SchedulerBase: 默认调度timer和reactor,reactor最长阻塞30毫秒（30000微秒），可以通过add_handle()添加非阻塞调度循环,
class SchedulerBase : public Thread
{
public:
	SchedulerBase(void);
	virtual ~SchedulerBase(void);
	
	int run();
	void end();
	virtual int work(int e);

public:
	int add_handle(SchedulerHandler* h);
	int remove_handle(SchedulerHandler* h);

private:
	bool	m_brun;
	int		m_i;
	SchedulerHandler* m_handles[SCHEDULER_MAX_HANDLE];
};

typedef Singleton<SchedulerBase> SchedulerBaseSngl;

