#include "uac_SockAcceptor.h"
#include "uac_Util.h"
#include <assert.h>


namespace UAC
{


int set_blocking(SOCKET s,bool is_blocking)
{
#ifdef _WIN32
	//NONBLOCKING=1
	u_long val = is_blocking?0:1;
	if(INVALID_SOCKET!=s)
		return ioctlsocket(s,FIONBIO,&val);
	return -1;
#elif defined(_ECOS_8203)
	int val = is_blocking?0:1;
	return ioctl(s,FIONBIO,&val);
#else
	int opts;
	opts = fcntl(s,F_GETFL);
	if(-1 == opts)
	{
		perror("fcntl(s,GETFL)");
		return -1;
	}
	if(!is_blocking)
		opts |= O_NONBLOCK;
	else
		opts &= ~O_NONBLOCK;
	if(-1 == fcntl(s,F_SETFL,opts))
	{
		UACLOG("***error s=%d ***\n",s);
		perror("fcntl(s,SETFL,opts); ");
		return -1;
	}
	return 0;
#endif
}

SockAcceptor::SockAcceptor(void)
:m_fd(INVALID_SOCKET)
,m_hip(0)
,m_hport(0)
{
	FD_ZERO(&m_rfd);
}

SockAcceptor::~SockAcceptor(void)
{
	assert(INVALID_SOCKET==m_fd);
	close_sock();
}

int SockAcceptor::open_sock(unsigned short port,const char* ip)
{
	assert(INVALID_SOCKET==m_fd);
	close_sock();

	int flag = 0;
	do
	{
		m_fd = (int)socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
		if(INVALID_SOCKET == m_fd)
		{
			flag = -1;
			break;
		}

#ifndef _WIN32
		////端口重用
		//int x = 1;
		//if(-1==setsockopt(m_fd,SOL_SOCKET,SO_REUSEADDR,(char*)&x,sizeof(x)))
		//{
		//	perror("setsockopt(reuseaddr)faild!");
		//}

#if !defined(_OS)
		//绑定设备
		if(Util::is_dev(ip))
		{
			struct ifreq ifr;
			memset(&ifr,0,sizeof(ifr));
			//strncpy(ifr.ifr_name,ip,IFNAMSIZ);
			strcpy(ifr.ifr_name,ip);
			if(SOCKET_ERROR==setsockopt(m_fd, SOL_SOCKET, SO_BINDTODEVICE, (char*)&ifr, sizeof(ifr)))
			{
				perror("#***bind device failed \n");
			}
			else
			{
				perror("#bind device ok \n");
			}
		}
#endif
#endif

		//设置接收和发送缓冲
		int nBuf = 1024*1024;
		setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, ( const char* )&nBuf, sizeof(int)); 
		nBuf = 1024*1024;
		setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, ( const char* )&nBuf, sizeof(int)); 

		unsigned int nip = INADDR_ANY;
		if(Util::is_ip(ip))
			nip = inet_addr(ip);
		sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = nip;
		
		if(SOCKET_ERROR == ::bind(m_fd, (sockaddr *)&addr, sizeof(addr)))
		{
			flag = 1;
			break;
		}

		m_hip = ntohl(nip);
		m_hport = port;
		FD_SET(m_fd,&m_rfd);
		set_blocking(m_fd,false);
		UACLOG("#SockAcceptor open (udp --> %s:%d) \n",Util::ip_htoa(m_hip),(int)m_hport);
	}while(0);

	if(0!=flag)
	{
		close_sock();
		return -1;
	}
	return 0;
}
void SockAcceptor::close_sock()
{
	if(INVALID_SOCKET!=m_fd)
	{
		closesocket(m_fd);
		FD_ZERO(&_rfd);
		m_fd = INVALID_SOCKET;
		m_hip = 0;
		m_hport = 0;
	}
}
int SockAcceptor::handle_select_read(unsigned long delay_usec/*=0*/)
{
	_timeout.tv_sec=(long)(delay_usec/1000000);
	_timeout.tv_usec=(long)(delay_usec%1000000);
	_rfd = m_rfd;
	_n = select(m_fd+1,&_rfd,NULL,NULL,&_timeout);
	if(_n>0)
	{
		if(FD_ISSET(m_fd,&_rfd))
		{
			handle_input();
		}
		else
		{
			assert(0);
		}
		return 0;
	}
	return -1;
}

}
