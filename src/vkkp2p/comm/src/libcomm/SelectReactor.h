#pragma once
#include "IOReactor.h"

class SelectReactor : public IOReactor
{
public:
	SelectReactor(void);
	virtual ~SelectReactor(void);

public:
	virtual void handle_root(ULONGLONG delay_usec=0);
	virtual int register_handler(SockHandler *h,int se,bool is_et=false);
	virtual int unregister_handler(SockHandler *h,int se);
	virtual int get_handler_num(){ return m_handler_num;}
private:
	void select_init(fd_set* prfd,fd_set* pwfd,fd_set* pefd,SOCKET& maxs);
	void select_finish(fd_set* prfd,fd_set* pwfd,fd_set* pefd,int n);

	int allot_nodei();
private:
	SNode_t *m_sn;
	fd_set m_rfd,m_wfd,m_efd;
	fd_set rfd,wfd,efd;
	int m_maxsock;
	int m_cursor;
};


