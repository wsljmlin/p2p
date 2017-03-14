#include "SockAcceptor.h"
#include "Util.h"

bool SockAcceptor::is_ip(const char* ip)
{
	unsigned int ip_n[4];
	if(ip && ip[0]!='\0' && 4==sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
		return true;
	return false;
}
bool SockAcceptor::is_dev(const char* ip)
{
	if(ip && ip[0]!='\0' && !is_ip(ip))
		return true;
	return false;
}

SockAcceptor::SockAcceptor(void)
:m_fd(INVALID_SOCKET)
,m_reactor(NULL)
,m_hip(0)
,m_hport(0)
{
}

SockAcceptor::~SockAcceptor(void)
{
	assert(INVALID_SOCKET==m_fd);
	close_sock();
}

int SockAcceptor::open_sock(unsigned short port,const char* ip,int iptype,IOReactor* reactor)
{
	assert(INVALID_SOCKET==m_fd && reactor);
	if(NULL==reactor)
		return -1;
	close_sock();
	
	m_reactor = reactor;
	int flag = 0;
	do
	{
		//0:tcp, 1:udp
		if(0==iptype)
			m_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		else
			m_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
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
		if(is_dev(ip))
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
		if(1==iptype)
		{
			int nBuf = 1024*1024;
			setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, ( const char* )&nBuf, sizeof(int)); 
			nBuf = 1024*1024;
			setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, ( const char* )&nBuf, sizeof(int)); 
		}
		unsigned int nip = INADDR_ANY;
		if(is_ip(ip))
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

		if(0==iptype)
		{
			if( SOCKET_ERROR == listen(m_fd,5))
			{
				flag = 2;
				break;
			}
		}

		if(NULL!=m_reactor && 0 != m_reactor->register_handler(this,SE_READ))
		{
			flag = 3;
			break;
		}

		m_hip = ntohl(nip);
		m_hport = port;
		DEBUGMSG("#SockAcceptor open (%d --> %s:%d) \n",iptype,Util::ip_htoa(m_hip),(int)m_hport);
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
		if(m_reactor)
			m_reactor->unregister_handler(this,SE_READ);
		closesocket(m_fd);
		m_fd = INVALID_SOCKET;
		m_hip = 0;
		m_hport = 0;
		m_reactor = NULL;
	}
}

int SockAcceptor::handle_output()
{
	assert(false);
	return 0;
}
int SockAcceptor::handle_error()
{
	assert(false);
	return 0;
}

