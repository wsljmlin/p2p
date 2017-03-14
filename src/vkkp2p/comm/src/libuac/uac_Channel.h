#pragma once
#include "uac_basetypes.h"
#include "uac_mempool.h"
#include "uac_Speaker.h"

namespace UAC
{

enum {DISCONNECTED=0,CONNECTING=1,CONNECTED=2};
#define IPTYPE_TCP 0
#define IPTYPE_UDP 1

class Channel;
class ChannelListener
{
public:
	virtual ~ChannelListener(void){}

	template<int I>
	struct S{enum{T=I};};
	
	typedef S<1> Connecting;
	typedef S<2> Connected;
	typedef S<3> Disconnected;
	typedef S<4> Data;
	typedef S<5> Writable;
	typedef S<6> ReadLimit;

	virtual void on(Connecting,Channel* ch){}
	virtual void on(Connected,Channel* ch){}
	virtual void on(Disconnected,Channel* ch){}
	virtual void on(Data,Channel* ch,memblock *b){}
	virtual void on(Writable,Channel* ch){}
};

class Channel : public Speaker<ChannelListener>
{
public:
	Channel(void)
		:Speaker<ChannelListener>(2)
		,m_last_active_tick(0)
		,m_state(DISCONNECTED)
		,m_hip(0)
		,m_hport(0)
		,m_is_accept(false){}
	virtual ~Channel(void){assert(m_state == DISCONNECTED);}
	unsigned int m_last_active_tick;
public:
	virtual int attach(SOCKET s,sockaddr_in& addr)=0;
	virtual int connect(const char* ip,unsigned short port,int nattype=0)=0;
	virtual int connect(unsigned int ip,unsigned short port,int nattype=0)=0;
	virtual int disconnect()=0;
	virtual int send(memblock *b,bool more=false)=0;  //-1:false; 0:send ok; 1:put int sendlist
	virtual int recv(char *b,int size)=0;
	virtual unsigned int get_my_ip(){return 0;}
	virtual int set_sndbuf(int bufsize){return 0;}
protected:
	virtual void reset()
	{
		m_state = DISCONNECTED;
		//m_hip = 0; //断开时上层有时候需要查IP
		//m_hport = 0;
		m_is_accept = false;
	}

	GETSET(int,m_state,_state)
	GETSET(unsigned int, m_hip,_hip)
	GETSET(unsigned short, m_hport,_hport)
	GETSET(bool,m_is_accept,_is_accept)
};

}

