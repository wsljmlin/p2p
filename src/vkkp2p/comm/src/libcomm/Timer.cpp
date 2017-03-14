#include "Timer.h"
#include <assert.h>

//*******************************QueueDelay*******************************
DelayQueue::DelayQueue()
:m_last_tick(0)
,m_size(0)
{
	m_head.remain_us = (ULONGLONG)-1l;
}
DelayQueue::~DelayQueue()
{
	assert(0==m_size);
	assert(&m_head==m_head.next);
}

ULONGLONG DelayQueue::get_remain_us()
{
	return m_head.next->remain_us;
}
void DelayQueue::add_node(TNode_t* node)
{
	assert(node==node->next);
	synchronize();
	node->remain_us = node->us;
	TNode_t *next = m_head.next;
	while(next!=&m_head && node->remain_us > next->remain_us)
	{
		node->remain_us -= next->remain_us;
		next = next->next;
	}
	if(next!=&m_head)
		next->remain_us -= node->remain_us;
	//inset befor next;
	assert(next->pre->next==next);
	node->next = next;
	node->pre = next->pre;
	next->pre = node->pre->next = node;
	m_size++;
}
void DelayQueue::del_node(TNode_t* node)
{
	assert(node!=node->next);
	if(node->next!=&m_head)
		node->next->remain_us += node->remain_us;
	node->pre->next = node->next;
	node->next->pre = node->pre;
	node->pre = node->next = node;
	m_size--;
}
TNode_t* DelayQueue::get_zero_delay()
{
	if(m_head.next->remain_us==0)
		return m_head.next;
	return NULL;
}
void DelayQueue::synchronize()
{
	ULONGLONG tick = GetUTickCount();
	//重置
	if(tick<m_last_tick)
	{
		m_last_tick = tick;
		return;
	}
	ULONGLONG tmp = tick - m_last_tick;
	m_last_tick = tick;
	TNode_t *next = m_head.next;
	while(next!=&m_head && tmp > next->remain_us)
	{
		tmp -= next->remain_us;
		next->remain_us = 0;
		next = next->next;
	}
	if(next!=&m_head)
		next->remain_us -= tmp;
}

//*******************************Timer**************************************

Timer::Timer(void)
{
	m_nl_size = 2048;
	m_nl = new TNode_t[m_nl_size];
	m_cursor = 0;
	m_is_free = false;
}

Timer::~Timer(void)
{
	assert(0==m_cursor);
	delete[] m_nl;
}

void Timer::handle_root()
{
	//DWORD tick = GetUTickCount();
	//int i=0;
	//while(i<m_cursor)
	//{
	//	for(;i<m_cursor;++i)
	//	{
	//		if(m_nl[i].next_us<=tick)
	//		{
	//			//DEBUGMSG("#...Timer(tick:%d / nextt:%d / h:%d / id:%d / us:%d)\n",(unsigned int)tick,(unsigned int)m_nl[i].next_us,(unsigned int)m_nl[i].h,m_nl[i].e,m_nl[i].us);
	//			m_is_free = false;
	//			m_nl[i].h->on_timer(m_nl[i].e);
	//			//如果回调过程中有删除,跳出重跑
	//			if(m_is_free)
	//				break;
	//			m_nl[i].next_us += m_nl[i].us;
	//		}
	//	}
	//}
	synchronize();
	TNode_t *node;
	while((node=get_zero_delay()))
	{
		node->h->on_timer(node->e); //on_timer()中可能执行del_node(),并且再执行add_node()
		if(node->h)
		{
			del_node(node);
			add_node(node);
		}
	}

}
int Timer::register_timer(TimerHandler *h,int e,DWORD ms)
{
	return register_utimer(h,e,((ULONGLONG)ms)*1000l);
}
int Timer::register_utimer(TimerHandler *h,int e,ULONGLONG us)
{
	if(NULL==h || us<=0)
		return -1;
	int i = find(h,e);
	if(-1==i)
		i = allot();
	else
		del_node(&m_nl[i]);
	if(-1==i)
		return -1;
	m_nl[i].h = h;
	m_nl[i].e = e;
	m_nl[i].us = us;
	//m_nl[i].next_us = GetUTickCount()+us;
	add_node(&m_nl[i]);
	return 0;
}
int Timer::unregister_timer(TimerHandler *h,int e)
{
	int i = find(h,e);
	if(-1!=i)
	{
		del_node(&m_nl[i]);
		free(i);
		return 0;
	}
	else
		return -1;
}
int Timer::unregister_all(TimerHandler *h)
{
	int i=0;
	while(i<m_cursor)
	{
		for(;i<m_cursor;++i)
		{
			if(m_nl[i].h==h)
			{
				del_node(&m_nl[i]);
				free(i);//m_cursor值有改变，并且此是i不能递增
				break;
			}
		}
	}
	return 0;
}

int Timer::find(TimerHandler *h,int e)
{
	for(int i=0;i<m_cursor;++i)
	{
		if(m_nl[i].h==h && m_nl[i].e==e)
			return i;
	}
	return -1;
}
int Timer::allot()
{
	if(m_cursor>=m_nl_size)
		return -1;
	return m_cursor++; //注意测试是否为预定结果
}
void Timer::free(int i)
{
	m_is_free = true;
	if(i!=m_cursor-1)
	{
		m_nl[i] = m_nl[m_cursor-1];
		m_nl[i].next->pre = m_nl[i].pre->next = &m_nl[i]; //链节点也要移动
	}
	m_nl[m_cursor-1].reset();
	m_cursor--;
}

