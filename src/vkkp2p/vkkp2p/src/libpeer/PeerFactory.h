#pragma once
#include "Peer.h"
#include "uac_SocketSelector.h"

class PeerFactory : public TimerHandler
	, public PeerListener
	, public TCPChannelFactory
	, public UAC_SocketFactory
{
public:
	PeerFactory(void);
	virtual ~PeerFactory(void);
	typedef struct tagPeerData
	{
		Peer *peer;   //peer!=NULL 表示占用
		bool bturn;   //返连接
		void* dref;   //下载引用
		bool bshare;  //共享
		void reset()
		{
			peer = 0;
			bturn = false;
			dref    = NULL;
			bshare = false;
		}
		tagPeerData(void){ reset();}
	}PeerData;
public:
	int init();
	int fini();

	Peer *get_peer(char ip_type,char conn_type,void* dref,bool turn);
	int put_peer(Peer* peer);
	
	virtual void on_timer(int e);
	virtual bool attach_tcp_socket(SOCKET fd,sockaddr_in& addr);
	virtual bool uac_attach_socket(UAC_SOCKET fd,const UAC_sockaddr& addr);

	virtual void on(PeerListener::Connecting,Peer* peer){}
	virtual void on(PeerListener::Connected,Peer* peer){}
	virtual void on(PeerListener::Data,Peer* peer,char* buf,int len){}
	virtual void on(PeerListener::Writable,Peer* peer){}
	virtual void on(PeerListener::Disconnected,Peer* peer);

	int get_new_peer_times() const {return m_new_peer_times;}
	int get_online_peers() const {return m_peers.count;}

protected:
	void put_all_peer();
	void deal_pending();
	void deal_deadpeer();
	virtual void on_disconnected(Peer* peer){}
protected:
	unsigned int m_curr_tick;
	BaseArray<PeerData> m_peers;
	list<Peer*> m_pending_peerls;
	unsigned int m_new_peer_times;
};
