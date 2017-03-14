#pragma once
#include "IOReactor.h"


class SockAcceptor : public SockHandler
{
public:
	SockAcceptor(void);
	virtual ~SockAcceptor(void);

	static bool is_ip(const char* ip);
	static bool is_dev(const char* ip);
public:
	int open_sock(unsigned short port,const char* ip,int iptype,IOReactor* reactor);
	void close_sock();

	virtual int sock(){return (int)m_fd;}
	//virtual int handle_input(); //不实现接收
	virtual int handle_output();
	virtual int handle_error();

protected:
	SOCKET m_fd;
	IOReactor *m_reactor;
	GETSET(unsigned int, m_hip,_hip)
	GETSET(unsigned short, m_hport,_hport)
};
