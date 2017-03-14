#pragma once
#include "comms.h"
#include "Handler.h"

class IOReactor
{
public:
	IOReactor(void);
	virtual ~IOReactor(void);

	typedef struct tagSNode
	{
		SockHandler *h;//handler
		SOCKET s;
		int se; //select_event
		bool is_et; //是否使用et边缘触发
		tagSNode() { reset();}
		void reset() { h=NULL;s=INVALID_SOCKET;se=0;is_et=false; }
	}SNode_t;

public:
	//virtual int init(){return 0;}
	//virtual void fini(){}
	virtual void handle_root(ULONGLONG delay_usec=0){}
	//is_et=true 时只是对epoll的边缘触发使用，其它无效
	virtual int register_handler(SockHandler *h,int se,bool is_et=false){return 0;}
	virtual int unregister_handler(SockHandler *h,int se){return 0;}
	unsigned int get_handler_num(){ return m_handler_num;}
protected:
	int set_blocking(SOCKET s,bool is_blocking);
protected:
	unsigned int m_handler_num;
};
typedef Singleton<IOReactor> IOReactorSngl;

