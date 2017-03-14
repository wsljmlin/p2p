#include "TCPChannel.h"
#include "IOReactor.h"
#include "Util.h"
#define SA_CONNECT 1
#define SA_ATTACH 2
#define SA_HANDLE_CONNECTING 3
#define SA_HANDLE_WRITE 4
#define SA_HANDLE_ERROR 5
#define SA_SEND 6
#define SA_RECV 7

bool is_ip(const char* ip)
{
	unsigned int ip_n[4];
	if(NULL!=ip && 4==sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
		return true;
	return false;
}
bool is_dev(const char* ip)
{
	if(ip && ip[0]!='\0' && !is_ip(ip))
		return true;
	return false;
}

TCPChannel::TCPChannel(void)
:m_fd(INVALID_SOCKET)
,m_smore(false)
,m_is_regwrite(false)
{
	m_iptype=IPTYPE_TCP;
	m_last_active_tick = GetTickCount();
}

TCPChannel::~TCPChannel(void)
{
}

void TCPChannel::reset()
{
	for(SendIter it=m_slist.begin();it!=m_slist.end();++it)
	{
		(*it)->free();
	}
	m_slist.clear();
	m_fd = INVALID_SOCKET;
	m_is_regwrite = false;
	m_smore = false;
	Channel::reset();
}
void TCPChannel::close_socket()
{
	if(INVALID_SOCKET!=m_fd)
	{
		closesocket(m_fd);
		m_fd = INVALID_SOCKET;
	}
}
int TCPChannel::on_connected()
{
	m_last_error = 0;
	m_state = CONNECTED;
	IOReactorSngl::instance()->register_handler(this,SE_READ);
	on_connected_ex(); //可能里面调用已经close掉
	return 0;
}
void TCPChannel::on_connected_ex()
{
	if(CONNECTED==m_state)
		fire(ChannelListener::Connected(),this);
}
int TCPChannel::attach(SOCKET s,sockaddr_in& addr)
{
	//如果失败，此处不关闭s
	assert(INVALID_SOCKET==m_fd);
	m_last_action = SA_ATTACH;
	m_last_error = 0;

	m_fd = s;
	m_hip = ntohl(addr.sin_addr.s_addr);
	m_hport = ntohs(addr.sin_port);
	m_is_accept = true;
	on_connected();
	return 0;
}
int TCPChannel::connect(const char* ip,unsigned short port,const char* bindip/*=NULL*/,int nattype/*=0*/)
{
	return connect(ntohl(inet_addr(ip)),port,bindip,nattype);
}
int TCPChannel::connect(unsigned int ip,unsigned short port,const char* bindip/*=NULL*/,int nattype/*=0*/)
{
	//参数为host ip，host port
	m_last_action = SA_CONNECT;
	m_last_error = 0;
	if(DISCONNECTED!=m_state)
	{
		assert(0);
		m_last_error = -1;
		return -1;
	}

	m_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(INVALID_SOCKET==m_fd)
	{
		DEBUGMSG("#***socket() failed! \n");
		m_last_error = -2;
		return -1;
	}
	if(0!=IOReactorSngl::instance()->register_handler(this,SE_WRITE))
	{
		DEBUGMSG("#***IOReactorSngl::instance()->register_handler() failed! \n");
		close_socket();
		m_last_error = -3;
		return -1;
	}
	
	sockaddr_in addr;
	if(is_ip(bindip))
	{
		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = inet_addr(bindip);
		if(SOCKET_ERROR==::bind(m_fd,(sockaddr*)&addr,sizeof(addr)))
		{
			DEBUGMSG("#***bind(%s:%d) failed \n",inet_ntoa(addr.sin_addr),0);
		}
		else
		{
			memset(&addr,0,sizeof(addr));
			socklen_t len = sizeof(addr);
			if(0==getsockname(m_fd,(sockaddr*)&addr,&len))
			{
				DEBUGMSG("#bind(%s:%d) ok \n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
			}
		}
	}
	else if(bindip && bindip[0]!='\0')
	{
#if !defined(_WIN32) && !defined(_OS)
		struct ifreq ifr;
		memset(&ifr,0,sizeof(ifr));
		//strncpy(ifr.ifr_name,bindip,IFNAMSIZ);
		strcpy(ifr.ifr_name,bindip);
		if(SOCKET_ERROR==setsockopt(m_fd, SOL_SOCKET, SO_BINDTODEVICE, (char*)&ifr, sizeof(ifr)))
		{
			DEBUGMSG("#***bind device(%s) failed \n",bindip);
		}
		else
		{
			DEBUGMSG("#bind device(%s) ok \n",bindip);
		}
#endif
	}

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(ip);
	if(SOCKET_ERROR == ::connect(m_fd,(sockaddr*)&addr,sizeof(addr)))
	{
#ifdef _WIN32
		int err = WSAGetLastError();
		if(WSAEWOULDBLOCK != err)
#else
		if(EINPROGRESS != errno)
#endif
		{
			DEBUGMSG("#***connect() failed! \n");
			IOReactorSngl::instance()->unregister_handler(this,SE_WRITE);
			close_socket();
			m_last_error = -4;
			return -1;
		}
		m_state = CONNECTING;
		fire(ChannelListener::Connecting(),this);
	}
	else
	{
		on_connected(); //先注册读，再注销写
		IOReactorSngl::instance()->unregister_handler(this,SE_WRITE);
	}
	m_hip = ip;
	m_hport = port;

	return 0;
}
int TCPChannel::disconnect()
{
	if(DISCONNECTED!=m_state)
	{
		IOReactorSngl::instance()->unregister_handler(this,SE_BOTH);
		closesocket(m_fd);
		reset();
		fire(ChannelListener::Disconnected(),this);
	}
	return 0;
}
int TCPChannel::send(MemBlock *b,bool more/*=false*/)
{
	m_last_active_tick = GetTickCount();
	assert(b);
	if(CONNECTED != m_state)
	{
		b->free();
		return -1;
	}
	if(b->datalen <= b->datapos)
	{
		b->free();
		return 0;
	}
	//可能peer同时被download 与 share占用.这样download调用的send很容易会使share无法发数据.所以发送时总执行一下注册可写
	m_smore = true;
	if(!m_slist.empty())
	{
		m_slist.push_back(b);
		return 1;
	}
	int ret = ::send(m_fd, b->buf + b->datapos, b->datalen - b->datapos,0);
	if(ret > 0)
	{
		b->datapos += ret;
		if(b->datapos < b->datalen)
		{
			//DEBUGMSG("#send blocking...\n");
			m_slist.push_front(b);
		}
		else
		{
			b->free();
		}
	}
	else
	{
#ifdef _WIN32
		m_last_error = WSAGetLastError();
		if(-1 == ret && WSAEWOULDBLOCK == m_last_error)
#else
		m_last_error = errno;
		if(-1 == ret && EAGAIN == errno)
#endif
		{
			//DEBUGMSG("#send blocking...\n");
			m_slist.push_front(b);
		}
		else
		{
			b->free();
			m_last_action = SA_SEND;
			disconnect();
			return -1;
		}
	}
	//不能使用more参数，因为可能share发完要求more,而跟着download发请求不需要more，此时会导致share的more无效
	//注意：使用边缘触发时，可写只提醒一次
	if(!m_is_regwrite && 1==m_slist.size())
	{
		m_is_regwrite = true;
		IOReactorSngl::instance()->register_handler(this,SE_WRITE);
	}
	//1表示发送阻塞
	return m_slist.empty()?0:1;
}
int TCPChannel::recv(char *b,int size)
{
	m_last_active_tick = GetTickCount();
	int ret = ::recv(m_fd,b,size,0);
	if(ret > 0)
	{
		return ret;
	}
	else
	{
#ifdef _WIN32
		m_last_error = WSAGetLastError();
		if(-1 == ret && WSAEWOULDBLOCK == m_last_error)
#else
		m_last_error = errno;
		if(-1 == ret && EAGAIN == errno)
#endif
			return 0;
		else
		{
			m_last_action = SA_RECV;
			disconnect();
			return -1;
		}
	}
}
unsigned int TCPChannel::get_my_ip()
{
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	socklen_t len = sizeof(addr);
	if(0!=getsockname(m_fd,(sockaddr*)&addr,&len))
		return 0;
	//printf("# sock name (%s:%d) \n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
	//获取目标IP代码
	//if(0!=getpeername(m_fd,(sockaddr*)&addr,&len))
	//	return 0;
	//printf("# peer name (%s:%d) \n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
	return ntohl((unsigned int)addr.sin_addr.s_addr);
}
int TCPChannel::set_sndbuf(int bufsize)
{
	if(-1==setsockopt(m_fd,SOL_SOCKET,SO_SNDBUF,(const char*)&bufsize,sizeof(int)))
	{
		perror("setsockopt SO_RCVBUF");
		return -1;
	}
	return 0;
}
int TCPChannel::handle_input()
{
	//assert(CONNECTED==m_state);
	if(CONNECTED != m_state)
		return 0;
	//tcpchannel考虑提交上面去读
	int wait = 0;
	fire(ChannelListener::Readable(),this,wait);
	return wait;
}
int TCPChannel::handle_output()
{
	//epool 边缘触发只提示一次
	if(CONNECTED == m_state)
	{
		while(!m_slist.empty())
		{
			MemBlock *b = m_slist.front();
			int ret = ::send(m_fd,b->buf + b->datapos,b->datalen - b->datapos,0);
			if(ret > 0)
			{
				b->datapos += ret;
				if(b->datapos >= b->datalen)
				{
					m_slist.pop_front();
					b->free();
				}
				else
				{
					//DEBUGMSG("#send blocking...\n");
					break;
				}
			}
			else
			{
#ifdef _WIN32
				m_last_error = WSAGetLastError();
				if(-1 == ret && WSAEWOULDBLOCK == m_last_error)
#else
				m_last_error = errno;
				if(-1 == ret && EAGAIN == m_last_error)
#endif
					break;
				else
				{
					m_last_action = SA_HANDLE_WRITE;
					disconnect();
					return -1;
				}

			}
		}
		if(m_slist.empty())
		{
			if(m_smore)
			{
				//注意避免无发送也总是监听
				m_smore = false;
				fire(ChannelListener::Writable(),this); //这里会可能导致多次发送到阻塞，所以避免在里面重复注册写事件
			}
			else
			{
				//对于边缘触发其实是不会跑到这里，因为只提示一次，而且那一次肯定是m_smore=true
				IOReactorSngl::instance()->unregister_handler(this,SE_WRITE);
				m_is_regwrite = false;
			}
		}

	}
	else if(CONNECTING == m_state)
	{
		int err = 0;
		socklen_t len = sizeof(err);
		getsockopt(m_fd,SOL_SOCKET,SO_ERROR,(char*)&err,&len);
		if(err)
		{
			DEBUGMSG("***TCPChannel::handle_output()::CONNECTING::errno=0x%x\n",err);
			m_last_error = err;
			m_last_action = SA_HANDLE_CONNECTING;
			disconnect();
			return -1;
		}
		sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		len = sizeof(addr);
		if(0==getsockname(m_fd,(sockaddr*)&addr,&len))
		{
			//DEBUGMSG("#TCP connect() ok my ipport=%s:%d \n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
		}
		DEBUGMSG("#TCP connect() ok desip(%s:%d) \n",Util::ip_htoa(m_hip),(int)m_hport);
		IOReactorSngl::instance()->unregister_handler(this,SE_WRITE);
		on_connected(); //有可能在里面已经有发送到阻塞成注册了写事件的情况，所以在此先去掉写事件再调用到此
	}
	else
	{
		assert(0);
	}
	return 0;
}
int TCPChannel::handle_error()
{
	int err=0;
	socklen_t len = sizeof(err);
	getsockopt(m_fd,SOL_SOCKET,SO_ERROR,(char*)&err,&len);
	DEBUGMSG("***TcpConnection::OnError()::errno=0x%x\n",err);
	m_last_error = err;
	m_last_action = SA_HANDLE_ERROR;
	disconnect();
	return 0;
}

