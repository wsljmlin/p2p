#pragma once
#include "commons.h"
#include "Peer.h"

class NattypeMgr : public PeerListener
	,public TimerHandler
{
	enum {IDLE=0,TCP_CHECKING};
public:
	NattypeMgr(void);
	~NattypeMgr(void);

	static void callback_udp_onnatok(int nattype);
	static void callback_udp_ipportchanged(unsigned int ip,unsigned short port);

	int init();
	void fini();

	virtual void on_timer(int e);
	virtual void on(Connected,Peer* peer);
	virtual void on(Disconnected,Peer* peer);
	virtual void on(Data,Peer* peer,char* buf,int len);
	void on_check_nat_ok();
private:
	int start_check_tcp_nattype();

	void timer_tryget_udp_nattype();
private:
	int m_state;
	Peer* m_peer; //tcp check

	int m_tryget_udp_nattype_times;
};
typedef Singleton<NattypeMgr> NattypeMgrSngl;
