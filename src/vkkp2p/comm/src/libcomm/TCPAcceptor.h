#pragma once
#include "SockAcceptor.h"

class TCPChannelFactory
{
public:
	virtual ~TCPChannelFactory(void){}
public:
	virtual bool attach_tcp_socket(SOCKET fd,sockaddr_in& addr)=0;
};
class TCPAcceptor : public SockAcceptor
{
public:
	TCPAcceptor(void);
	virtual ~TCPAcceptor(void);

	int open(unsigned short port,const char* ip,TCPChannelFactory* chf,IOReactor* reactor);
	void close();
	virtual int handle_input();
private:
	TCPChannelFactory *m_chf;
};
