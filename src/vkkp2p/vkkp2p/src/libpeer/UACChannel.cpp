#include "UACChannel.h"
#include "Util.h"

UACChannel::UACChannel(void)
{
	m_last_active_tick = GetTickCount();
}

UACChannel::~UACChannel(void)
{
}
int UACChannel::attach(SOCKET s,sockaddr_in& addr)
{
	UAC_sockaddr uac_addr;
	uac_addr.ip = ntohl(addr.sin_addr.s_addr);
	uac_addr.port = ntohs(addr.sin_port);
	uac_addr.nattype = 0;

	m_hip = ntohl(addr.sin_addr.s_addr);
	m_hport = ntohs(addr.sin_port);
	m_is_accept = true;
	return uac_attach((int)s,uac_addr);
}
int UACChannel::connect(const char* ip,unsigned short port,const char* bindip/*=NULL*/,int nattype/*=0*/)
{
	return connect(Util::ip_atoh(ip),port,bindip,nattype);
}
int UACChannel::connect(unsigned int ip,unsigned short port,const char* bindip/*=NULL*/,int nattype/*=0*/)
{
	m_hip = ip;
	m_hport = port;
	m_state = CONNECTING;
	return this->uac_connect(ip,port,nattype);
}
int UACChannel::disconnect()
{
	if(DISCONNECTED!=m_state)
	{
		uac_disconnect();
		m_state = DISCONNECTED;
		reset();
		fire(ChannelListener::Disconnected(),this);
	}
	return 0;
}
int UACChannel::send(MemBlock *b,bool more/*=false*/)  //-1:false; 0:send ok; 1:put int sendlist
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
	int sendsize = b->datalen - b->datapos;
	int ret = uac_send(b->buf + b->datapos,sendsize);
	b->free();
	if(ret!=sendsize)
	{
		disconnect();
		return -1;
	}
	return uac_is_write()?0:1; //0¼ÌÐø¿ÉÒÔÐ´
}
int UACChannel::recv(char *b,int size)
{
	m_last_active_tick = GetTickCount();
	int ret = uac_recv(b,size);
	if(-1==ret)
	{
		disconnect();
		return -1;
	}
	return ret;
}

//uac
int UACChannel::uac_on_read()
{
	int wait = 0;
	fire(ChannelListener::Readable(),this,wait);
	return 0;
}
void UACChannel::uac_on_write()
{
	if(UAC_CONNECTED==m_uac_state)
	{	
		fire(ChannelListener::Writable(),this);
	}
	else
	{
		UAC_Socket::uac_on_write();
	}
}
void UACChannel::uac_on_connected()
{
	m_state = CONNECTED;
	uac_setsendbuf(102400);
	uac_setrecvbuf(1024000);
	UAC_Socket::uac_on_connected();
	fire(ChannelListener::Connected(),this);
}

