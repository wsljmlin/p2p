#pragma once
#include "Handler.h"
#include "basetypes.h"
#include "Singleton.h"

typedef struct tagTNode
{
	TimerHandler *h;
	int e; //eventid
	ULONGLONG us; //timeout
	//DWORD next_us;//下一次执行时间
	ULONGLONG remain_us; //剩余等待时间
	tagTNode *next;
	tagTNode *pre;
	tagTNode() {reset();}
	void reset() { h=NULL;e=0;us=0;/*next_us=0;*/remain_us=0;next=pre=this;}
}TNode_t;

class DelayQueue
{
public:
	DelayQueue();
	virtual ~DelayQueue();

public:
	ULONGLONG get_remain_us();
protected:
	void add_node(TNode_t* node);
	void del_node(TNode_t* node);
	TNode_t* get_zero_delay();
	void synchronize();

protected:
	TNode_t m_head;
	ULONGLONG m_last_tick;
	int m_size;
};

class Timer : public DelayQueue
{
	friend class Singleton<Timer>;
private:
	Timer(void);
	virtual ~Timer(void);

public:
	void handle_root();
	int register_timer(TimerHandler *h,int e,DWORD ms);
	int register_utimer(TimerHandler *h,int e,ULONGLONG us);
	int unregister_timer(TimerHandler *h,int e);
	int unregister_all(TimerHandler *h);

private:
	int find(TimerHandler *h,int e);
	int allot();
	void free(int i);

private:
	TNode_t *m_nl;
	int m_nl_size;
	int m_cursor;
	bool m_is_free;
};
typedef Singleton<Timer> TimerSngl;
