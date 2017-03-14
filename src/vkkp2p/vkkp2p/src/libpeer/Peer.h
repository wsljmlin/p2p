#pragma once
#include "Channel.h"
#include "commons.h"
#include "PeerListener.h"



class Peer : public ChannelListener
	,public Speaker<PeerListener>
{

public:
	Peer(char ip_type,char conn_type,int index);
	virtual ~Peer(void);

public:
	unsigned int m_last_active_tick;
public:
	int attach(SOCKET s,sockaddr_in& addr);
	int connect(const char* ip,unsigned short port,int nattype);
	int connect(unsigned int ip,unsigned short port,int nattype);
	int disconnect();
	int send(MemBlock *b);
	Channel* get_channel(){return m_ch;}
	int get_index(){return __i;}
	void set_PeerReadLimitSinker(PeerReadLimitSinker* pPeerReadLimitSinker){ m_pPeerReadLimitSinker = pPeerReadLimitSinker;}
public:
	virtual void on(Connecting,Channel* ch);
	virtual void on(Connected,Channel* ch);
	virtual void on(Disconnected,Channel* ch);
	virtual void on(Data,Channel* ch,char* buf,int size);
	virtual void on(Readable,Channel* ch,const int& wait);
	virtual void on(Writable,Channel* ch);

	virtual void on_data();
protected:
	GETSET(char,m_user_type,_user_type)
	GETSET(char,m_ip_type,_ip_type)
	GETSET(char,m_conn_type,_conn_type)
	GETSET(char,m_nat_type,_nat_type)
	GETSET(uint32,m_sessionID,_sessionID)
	GETSET(bool,m_conn_success,_conn_success)
protected:
	Channel* m_ch;
	MemBlock* m_block;
	int __i;
	PeerReadLimitSinker* m_pPeerReadLimitSinker;
};
