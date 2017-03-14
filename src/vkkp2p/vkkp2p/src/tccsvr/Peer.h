#pragma once
#include "TCPFWChannel.h"

class Peer;
class PeerListener
{
public:
	template<int I>
	struct S{enum{T=I};};
	
	typedef S<2> Connected;
	typedef S<3> Disconnected;
	typedef S<4> Data;

	virtual void on(Connected,Peer* peer){}
	virtual void on(Disconnected,Peer* peer){}
	virtual void on(Data,Peer* peer,MemBlock* b){}
};
class Peer : public ChannelListener
	,public Speaker<PeerListener>
{
public:
	Peer(void);
	virtual ~Peer(void);
public:
	int attach(SOCKET s,sockaddr_in& addr);
	int connect(unsigned int ip,unsigned short port);
	int disconnect();
	int send(MemBlock *b);
	TCPFWChannel* get_channel(){return &m_ch;}

	virtual void on(Connected,Channel* ch);
	virtual void on(Disconnected,Channel* ch);
	virtual void on(Readable,Channel* ch,const int& wait);

protected:
	void on_data();
	GETSET(int,sid,_sid) //session id
private:
	TCPFWChannel m_ch;
	MemBlock *m_block;
	int m_tmpsize;
};
