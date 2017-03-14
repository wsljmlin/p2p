#pragma once
#include "IOReactor.h"
#include "BaseArray.h"

#ifdef _WIN32
class ProReactor : public IOReactor
{
public:
	ProReactor(int fdsize){}
	virtual ~ProReactor(void){}
};

#else
#ifndef NO_EPOLL
class ProReactor : public IOReactor
{
public:
	ProReactor(int fdsize);
	virtual ~ProReactor(void);
public:
	virtual void handle_root(ULONGLONG delay_usec=0);
	virtual int register_handler(SockHandler *h,int se,bool is_et=false);
	virtual int unregister_handler(SockHandler *h,int se);
private:
	int m_max_fdsize;
	epoll_event *events;
	int nfds;
	int m_epfd;
	BaseArray<SNode_t> m_sn;
};
#endif
#endif

