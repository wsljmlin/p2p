#pragma once

#ifdef _WIN32

#else
#endif

class SocketBase
{
public:
	SocketBase(void);
	~SocketBase(void);

public:
	static int socket_tcp();
	static int socket_udp();
	static int close(int fd);

	static int set_timeout(int fd,int timeo_ms);
	static int set_udp_broadcast(int fd);
	static const char* ip_htoa(unsigned int ip);

	static int sendto(int fd,const char* buf,int size,unsigned int ip,unsigned short port);
	static int recvfrom(int fd,char* buf,int size,unsigned int* ip,unsigned short* port);
};
