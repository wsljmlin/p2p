#include "SocketBase.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#pragma warning(disable:4996)
//#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
//#include <windows.h>
#include <winsock2.h>
typedef int socklen_t;
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#endif


SocketBase::SocketBase(void)
{
}

SocketBase::~SocketBase(void)
{
}
int SocketBase::socket_tcp()
{
	return (int)socket(AF_INET,SOCK_STREAM,0);
}
int SocketBase::socket_udp()
{
	return (int)socket(AF_INET,SOCK_DGRAM,0);
}
int SocketBase::close(int fd)
{
#ifdef _WIN32
	return ::closesocket(fd);
#else
	return ::close(fd);
#endif
}

int SocketBase::set_timeout(int fd,int timeo_ms)
{
	//设置超时:
	int ret;
#ifdef _WIN32
	int x = timeo_ms;
#else
	struct timeval x;  
	x.tv_sec = timeo_ms/1000;
	x.tv_usec = (timeo_ms%1000) * 1000;
#endif
	if(0!=(ret=setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&x,sizeof(x))))
	{
		perror("setsockopt SO_RCVTIMEO");
	}
	if(0!=(ret=setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(char*)&x,sizeof(x))))
	{
		perror("setsockopt SO_SNDTIMEO");
	}
	return ret;
}
int SocketBase::set_udp_broadcast(int fd)
{
#ifdef _WIN32
	bool isbroadcast = true;
#else
	int isbroadcast = 1;
#endif
	int ret = 0;
	if(0!=(ret=setsockopt(fd,SOL_SOCKET,SO_BROADCAST,(const char*)&isbroadcast,sizeof(isbroadcast))))
	{
		perror("*** setsockopt(SO_BROADCAST): ");
	}
	return ret;
}
const char* SocketBase::ip_htoa(unsigned int ip)
{
	//非线程安全
	static char buf[32] = {0,};
	unsigned char ip_n[4];
	ip_n[0] = ip >> 24;
	ip_n[1] = ip >> 16;
	ip_n[2] = ip >> 8;
	ip_n[3] = ip;
	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
	return buf;

}
int SocketBase::sendto(int fd,const char* buf,int size,unsigned int ip,unsigned short port)
{
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	return ::sendto(fd,buf,size,0,(const sockaddr*)&addr,sizeof(addr));
}
int SocketBase::recvfrom(int fd,char* buf,int size,unsigned int* ip,unsigned short* port)
{
	sockaddr_in from_addr;
	socklen_t from_len;
	memset(&from_addr,0,sizeof(from_addr));
	from_len = sizeof(from_addr);
	int ret = ::recvfrom(fd,buf,size,0,(sockaddr*)&from_addr,&from_len);
	if(ret>0)
	{
		*ip = ntohl(from_addr.sin_addr.s_addr);
		*port = ntohs(from_addr.sin_port);
	}
	return ret;
}

