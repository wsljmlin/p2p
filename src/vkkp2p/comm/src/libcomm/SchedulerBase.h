#pragma once

#include "comms.h"

#define SCHEDULER_MAX_HANDLE  20
class SchedulerHandler
{
public:
	virtual ~SchedulerHandler(){}
	//unblocking root
	virtual int handle_root()=0; //-1:��ʾ�ڲ�������У�0:��ʾ���������,1:��ʾ�������Ӻ���,������Ϣһ��
};

//SchedulerBase: Ĭ�ϵ���timer��reactor,reactor�����30���루30000΢�룩������ͨ��add_handle()��ӷ���������ѭ��,
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

