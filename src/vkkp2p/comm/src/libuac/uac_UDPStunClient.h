#pragma once
#include "uac_Speaker.h"
#include "uac_UDPStunProtocol.h"
#include "uac_Timer.h"
#include "uac.h"

namespace UAC
{
class UDPStunClientListener
{
public:
	virtual ~UDPStunClientListener(void){}

	template<int I>
	struct S{enum{T=I};};
	
	typedef S<1> NatOk;
	typedef S<2> BindingAddrChanged;
	typedef S<3> ReqHole;
	typedef S<4> RspHole;

	virtual void on(NatOk,int nat_type){}
	virtual void on(BindingAddrChanged,unsigned int ip,unsigned short port){}
	virtual void on(ReqHole,unsigned int connid,unsigned int nip,unsigned short nport){}
	virtual void on(RspHole,unsigned int connid,unsigned int nip,unsigned short nport){}
};

class UDPStunClient : public TimerHandler
	,public Speaker<UDPStunClientListener>
{
public:
	UDPStunClient(SOCKET fd,const char* stun_ip,unsigned short stun_port);
	~UDPStunClient(void);

	enum {TIMER_CHECK_BINDING_ADDR=1,TIMER_CHECK_NAT=2,TIMER_NAT_RESEND=3};
	enum {REQS_FINI=0,REQS_STUNB_ADDR,REQS_NAT1,REQS_NAT4,REQS_NAT2};
	typedef struct tagStunsvrConfig
	{
		char stunA_ipstr[64];
		unsigned int stunA_ip;
		unsigned short stunA_port1;
		unsigned int stunB_ip;
		unsigned short stunB_port1;
		unsigned int stunC_ip;
		unsigned short stunC_port1;
	}StunsvrConfig_t;
public:
	int start_check_nat_type();

	virtual void on_timer(int e);
	void on_data(char* buf,int size,sockaddr_in& addr);
	int ptl_request_hole(unsigned int connid,unsigned int nip,unsigned short nport);
	int ptl_report_nattype(const PTL_STUN_ReportNattype_t& inf);
	int ptl_report_connsf(PTL_STUN_ReportConnSF_t& inf);
	int ptl_report_connstat(PTL_STUN_ReportConnStat_t& inf);
	unsigned int get_binding_ip() const {return m_binding_ip;}
private:
	unsigned int stunA_ip(); //检查是否需要解释IP
	void on_nat_ok(int nat_type);
	int send_nat_request();
	int ptl_request_binding_addr();
	int ptl_request_stunb_addr();
	int ptl_request_nat1();
	int ptl_request_nat4();
	int ptl_request_nat2();
	
private:
	SOCKET m_fd;
	StunsvrConfig_t m_stunconf;
	unsigned int m_binding_ip;
	unsigned short m_binding_port;
	int m_nat;
	int m_state;        //记录该发什么包
	int m_resend_count; //记录当前要发的包已经重复了多少次
	int m_nat4_port1,m_nat4_port2;
	unsigned int m_last_check_nat_tick;
	int m_reqbindaddr_count;
};

}
