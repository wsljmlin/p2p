#pragma once
#include "commons.h"
#include "Peer.h"
#include "PacketList.h"

class PeerManager : public PeerListener,public TCPChannelFactory
{
public:
	PeerManager(void);
	virtual ~PeerManager(void);
	typedef struct tagPeerInfo
	{
		Peer*			peer;
		time_t			last_active_time;
		uint32          ip;
		unsigned short  port;
		tagPeerInfo(void): peer(NULL),last_active_time(0),ip(0),port(0) {}
	}PeerInfo;
public:
	int init();
	void fini();
	void handle_root();//处理收发线程
	virtual bool attach_tcp_socket(SOCKET fd,sockaddr_in& addr);
	bool put_packet(const Packet& pack)
	{
		return m_packs.put_packet(pack);//如果已经fini，会失败。因此在此不必要判断是否已经fini
	}
	int get_peer_num(){ return m_peers.count; }
	int get_max_peer_num_and_reset(){
		int n = m_max_peer_num;
		m_max_peer_num = m_peers.count;
		return n;
	}
	int get_max_pack_num_and_reset() { 
		int n = m_max_pack_num;
		m_max_pack_num = 0;
		return n; 
	}
private:
	Peer* get_peer();
	int put_peer(Peer *peer,bool tell=true);
	void put_all_peer();

	void handle_send();
	void handle_timer();
	void handle_dead_peer();

public:
	virtual void on(PeerListener::Connected ,Peer* peer);
	virtual void on(PeerListener::Disconnected ,Peer* peer);
	virtual void on(PeerListener::Data ,Peer* peer,MemBlock *block);
private:
	bool m_binit;
	BaseArray<PeerInfo> m_peers;
	PacketList m_packs;
	int m_max_peer_num;
	int m_max_pack_num;
	time_t m_curr_sec;
	time_t m_last_clear_dead_sec;

	//临时变量
	Packet _tmp_pack;
};
typedef Singleton<PeerManager> PeerManagerSngl;
typedef Singleton<TCPAcceptor> TCPAcceptorSngl;

