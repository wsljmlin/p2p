#include "uac_Socket.h"
#include <assert.h>
#include "uac_SocketSelector.h"

UAC_Socket::UAC_Socket(void)
:m_uac_fd(-1)
,m_uac_state(UAC_DISCONNECTED)
,m_uac_blastsend(false)
,m_uac_bregsend(false)
{
}

UAC_Socket::~UAC_Socket(void)
{
	assert(UAC_DISCONNECTED == m_uac_state);
}
int UAC_Socket::uac_attach(UAC_SOCKET uac_fd,const UAC_sockaddr& uac_addr)
{
	if(uac_fd<0 || uac_fd>=UAC_FD_SIZE || UAC_DISCONNECTED!=m_uac_state)
	{
		assert(0);
		return -1;
	}
	m_uac_fd = uac_fd;
	m_uac_addr = uac_addr;
	uac_on_connected();
	return 0;
}
int UAC_Socket::uac_connect(unsigned int ip,unsigned short port,int nattype/*=0*/)
{
	if(UAC_DISCONNECTED!=m_uac_state||0==ip||0==port||nattype>4||nattype<0)
	{
		assert(0);
		return -1;
	}
	m_uac_addr.ip = ip;
	m_uac_addr.port = port;
	m_uac_addr.nattype = nattype;
	m_uac_fd = ::uac_connect(&m_uac_addr);
	if(-1==m_uac_fd)
		return -1;
	m_uac_state = UAC_CONNECTING;
	UAC_SocketSelectorSngl::instance()->register_socket(this,UAC_WRITE);
	return 0;
}
int UAC_Socket::uac_disconnect()
{
	if(UAC_DISCONNECTED!=m_uac_state)
	{
		UAC_SocketSelectorSngl::instance()->unregister_socket(this,UAC_READ|UAC_WRITE);
		::uac_closesocket(m_uac_fd);
		m_uac_fd = -1;
		m_uac_state = UAC_DISCONNECTED;
		m_uac_blastsend = false;
		m_uac_bregsend = false;
	}
	return 0;
}
int UAC_Socket::uac_send(const char* buf,int size)
{
	if(UAC_CONNECTED!=m_uac_state)
		return -1;
	m_uac_blastsend = true;
	if(!m_uac_bregsend)
	{
		m_uac_bregsend = true;
		UAC_SocketSelectorSngl::instance()->register_socket(this,UAC_WRITE);
	}
	if(size!=::uac_send(m_uac_fd,buf,size))
		return -1; //发不完有问题
	return size;
}
int UAC_Socket::uac_recv(char* buf,int size)
{
	if(UAC_CONNECTED!=m_uac_state)
		return -1;
	return ::uac_recv(m_uac_fd,buf,size);
}
int UAC_Socket::uac_setsendbuf(int size)
{
	if(m_uac_fd>=0)
		return ::uac_setsendbuf(m_uac_fd,size);
	return -1;
}
int UAC_Socket::uac_setrecvbuf(int size)
{
	if(m_uac_fd>=0)
		return ::uac_setrecvbuf(m_uac_fd,size);
	return -1;
}
//UAC_SocketSelector响应函数
void UAC_Socket::uac_on_write()
{
	if(UAC_CONNECTING==m_uac_state)
	{
		UAC_SocketSelectorSngl::instance()->unregister_socket(this,UAC_WRITE);
		uac_on_connected();
	}
	else
	{
		if(!m_uac_blastsend)
		{
			m_uac_bregsend = false;
			UAC_SocketSelectorSngl::instance()->unregister_socket(this,UAC_WRITE);
		}
		else
		{
			m_uac_blastsend = false;
		}
	}
}
void UAC_Socket::uac_on_connected()
{
	m_uac_state = UAC_CONNECTED;
	UAC_SocketSelectorSngl::instance()->register_socket(this,UAC_READ);
}

