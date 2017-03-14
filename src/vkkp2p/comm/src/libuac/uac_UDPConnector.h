#pragma once
#include "uac.h"
#include "uac_UDPProtocol.h"
#include "uac_Timer.h"
#include "uac_UDPStunClient.h"
#include "uac_cyclist.h"
#include "uac_mempool.h"
#include "uac_UDPTestSpeed.h"

namespace UAC
{
//含一个UDPStunClient指针
//短时间测速并不准确,所以取消连接测速过程避免连接延迟

enum {UDP_DISCONNECTED=0,UDP_CONNECTING=1,UDP_ACCEPTING=2,UDP_CONNECTED=3,UDP_ACCEPTED=4,UDP_CONN_TESTSPEEDING=5,UDP_ACCEP_TESTSPEEDING=6};
class UDPConnector : public TimerHandler
{
	friend class UDPAcceptor;

	typedef struct tagUDPSession
	{
		bool is_used;
		int state;
		unsigned int des_nip;
		unsigned short des_nport;
		uint32 my_nip;
		uint16 my_nport;
		unsigned char des_nattype; //目标的nattype类型
		unsigned char des_nattype2; //临时记录，用于判断是否还需要发req_hole发
		int src_sessionid;
		int des_sessionid;
		UDPChannelHandler* handle;
		DWORD last_send_tick;
		DWORD last_recv_tick;
		DWORD begin_tick;

		//测速
		UDPTestSpeed ts;

		void reset()
		{
			is_used = false;
			state = UDP_DISCONNECTED;
			des_nip = 0;
			des_nport = 0;
			des_nattype = 0;
			des_nattype2 = 0;
			my_nip = 0;
			my_nport = 0;
			src_sessionid = -1;
			des_sessionid = -1;
			handle = 0;
			last_send_tick = 0;
			last_recv_tick = 0;
			begin_tick = (DWORD)-60000; //间隔60秒以内不使用
			
			ts.reset();
		}
		tagUDPSession(void){reset();}
	}UDPSession_t;

	typedef struct tagNatInfo{
		unsigned int nip;
		unsigned short nport;
		DWORD last_send_tick;
		DWORD begin_tick;
		tagNatInfo(void) : nip(0),nport(0),last_send_tick(0),begin_tick(0){}
	}NatInfo_t;
	typedef list<NatInfo_t> NatList;
	typedef NatList::iterator NatIter;

private:
	UDPConnector(SOCKET fd,UDPChannelFactory* chf,UDPStunClient* stun);
	~UDPConnector();
public:
	virtual void on_timer(int e);
	int connect(UDPChannelHandler* ch,unsigned int ip,unsigned short port,int nattype);
	int disconnect(UDPChannelHandler* ch);
	int nat(unsigned int nip,unsigned short nport);
	int on_rsp_nat_hole(unsigned int connid,unsigned int nip,unsigned short nport);
	UDPStunClient* get_stun() const {return m_stun;}
	void set_stun(UDPStunClient* stun) {m_stun = stun;}
	int get_socknum()const {return m_socknum;}
private:
	int register_channel(UDPChannelHandler *ch,unsigned int ip,unsigned short port,int nattype);
	void unregister_channel(UDPChannelHandler *ch);
	int send_head(const UDPSConnHeader_t& head,sockaddr_in& addr,int send_times=1);
	void remove_nat(unsigned int nip,unsigned short nport);

	int on_data(memblock* block,sockaddr_in& addr);
	bool check_ipport_ok(UDPSConnHeader_t& head,sockaddr_in& addr);
	bool check_ipport_session_ok(UDPSConnHeader_t& head,sockaddr_in& addr);
	
	
	int handle_cmd_nat(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr);
	int handle_cmd_conn(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps);
	int handle_cmd_ok(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps);
	void on_connecting_ok(int i,sockaddr_in& addr);
	int handle_cmd_live(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr);
	int handle_cmd_close(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr);
	int handle_cmd_channel_packet(UDPSConnHeader_t& head,memblock* block,sockaddr_in& addr);
	int handle_cmd_testspeed_data(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps);
	int handle_cmd_testspeed_ack(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps);
	void send_conn(int i);
	void send_conn_ok(int i);
	void send_testspeed_data(int i);
	
	void timer_resend();
	void timer_channel_send();
	void check_update_port(int i,unsigned short nport);
private:
	SOCKET m_fd;
	UDPChannelFactory* m_chf;
	UDPStunClient *m_stun;
	UDPSession_t **m_udps;
	NatList m_natl;
	int m_socknum; //在线连接数
	int m_max_i;
	DWORD m_tick;
	uint32 m_mtu;

	char _tmp_sbuf[512];
	char *_tmp_ts_sbuf;
	UDPSConnHeader_t _tmp_shead;
	PTLStream _tmp_sps;
	UDPSConnHeader_t _tmp_rhead;
	PTLStream _tmp_rps;
};

}

