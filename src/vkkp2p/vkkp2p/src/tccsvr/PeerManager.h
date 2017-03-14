#pragma once
#include "commons.h"
#include "Peer.h"

class PeerManager : public PeerListener,public TCPChannelFactory
{
public:
	PeerManager(void);
	virtual ~PeerManager(void);
	typedef struct tagPeerInfo{
		Peer*			peer;
		time_t			last_active_time;
		uint32			ip;
		short			port;
		int				psid;  //测试请求的ID
		int				csid;  //连接的ID
		short			test_port;

		tagPeerInfo(void): peer(NULL),last_active_time(0),ip(0),port(0),psid(-1),csid(-1),test_port(0)
		{
		}
	}PeerInfo;

public:
	int init();
	void fini();
	void handle_root();//处理收发线程
	virtual bool attach_tcp_socket(SOCKET fd,sockaddr_in& addr);

	int get_peer_num(){ return m_peers.count; }
	int get_test_num(int& test_num,int& test_succeed_num)
	{
		test_num = m_test_num;
		test_succeed_num = m_test_succeed_num;
		return 0;
	}
	void reset_test_num()
	{
		m_test_num = 0;
		m_test_succeed_num = 0;
	}
private:
	void on_line(Peer *peer,MemBlock *block);
	Peer* get_peer(int psid=-1);
	int put_peer(Peer *peer);
	void put_all_peer();

	void handle_send();
	void handle_timer();
	void handle_dead_peer();

	void on_ptl_request_tcpcheck(int sid,const PTL_P2S_RequestTcpCheck& req);
	void on_ptl_welcome(int sid,PTL_P2P_Welcome& req);
	void ptl_response_tcpcheck(int sid,int result,uint32 dataid=0);
public:
	virtual void on(PeerListener::Connected ,Peer* peer);
	virtual void on(PeerListener::Disconnected ,Peer* peer);
	virtual void on(PeerListener::Data ,Peer* peer,MemBlock *block);
private:
	bool m_binit;
	BaseArray<PeerInfo> m_peers;
	time_t m_curr_sec;
	time_t m_last_clear_dead_sec;
	int m_peer_num;
	int m_test_num;
	int m_test_succeed_num;

	//临时变量
	PTLStream					tmp_ss_read,tmp_ss_write;
	PTL_Head					tmp_head;
	PTL_P2P_Welcome				tmp_welcome;
	PTL_P2S_RequestTcpCheck		tmp_reqtcpcheck;
	PTL_P2S_ResponseTcpCheck	tmp_rsptcpcheck;
};
typedef Singleton<PeerManager> PeerManagerSngl;
typedef Singleton<TCPAcceptor> TCPAcceptorSngl;

