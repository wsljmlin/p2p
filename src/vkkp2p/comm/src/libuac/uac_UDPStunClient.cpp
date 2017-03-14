#include "uac_UDPStunClient.h"
#include "uac_Util.h"

namespace UAC
{

UDPStunClient::UDPStunClient(SOCKET fd,const char* stun_ip,unsigned short stun_port)
: Speaker<UDPStunClientListener>(1)
, m_fd(fd)
, m_binding_ip(0)
, m_binding_port(0)
, m_nat(6)
, m_state(REQS_FINI)
, m_resend_count(0)
, m_last_check_nat_tick(0)
, m_reqbindaddr_count(0)
{
	assert(INVALID_SOCKET!=fd);
	assert(stun_ip || stun_port>0);
	strcpy(m_stunconf.stunA_ipstr,stun_ip);
	m_stunconf.stunA_ip = Util::ip_atoh(Util::ip_explain_ex(m_stunconf.stunA_ipstr).c_str());
	m_stunconf.stunA_port1 = stun_port;
	m_stunconf.stunB_ip = 0;
	m_stunconf.stunB_port1 = 0;
	UACLOG("#stun : %s(%s):%d \n",m_stunconf.stunA_ipstr,Util::ip_htoa(m_stunconf.stunA_ip),m_stunconf.stunA_port1);
	ptl_request_binding_addr();
	start_check_nat_type();
	//1分钟更新一次NAT CHECK
	//30秒探一次端口映射
	TimerSngl::instance()->register_timer(this,TIMER_CHECK_BINDING_ADDR,20000); //所有类型都要探测绑定端口，以便stun能够转发,nat4类型不到30秒就会变掉端口
	TimerSngl::instance()->register_timer(this,TIMER_CHECK_NAT,120000); //未探测成功的才会继续探测
}

UDPStunClient::~UDPStunClient(void)
{
	m_state = REQS_FINI;
	TimerSngl::instance()->unregister_all(this);
}


int UDPStunClient::start_check_nat_type()
{
	UACLOG("# UDPStunClient::start_check_nat_type... \n");
	m_last_check_nat_tick = GetTickCount();
	m_state = REQS_STUNB_ADDR;
	m_resend_count = 0;
	m_nat4_port1 = 0;
	m_nat4_port2 = 0;
	TimerSngl::instance()->register_timer(this,TIMER_NAT_RESEND,600);
	return 0;
}
unsigned int UDPStunClient::stunA_ip()
{
	if(0==m_stunconf.stunA_ip || (unsigned int)-1==m_stunconf.stunA_ip)
		m_stunconf.stunA_ip = Util::ip_atoh(Util::ip_explain_ex(m_stunconf.stunA_ipstr).c_str());
	return m_stunconf.stunA_ip;
}
void UDPStunClient::on_nat_ok(int nat_type)
{
	//不是初次检查，而且检查到是5型，不更新
	if(REQS_FINI==m_state)
		return;
	m_state = REQS_FINI;
	m_resend_count = 0;
	TimerSngl::instance()->unregister_timer(this,TIMER_NAT_RESEND);
	if(6==m_nat || 5!=nat_type)
	{
		//如果之前未获得过nattype(6),则所有类似都通知，否则只有不等5时才会响应处理和通知
		m_nat = nat_type;
		fire(UDPStunClientListener::NatOk(),m_nat);
		PTL_STUN_ReportNattype_t inf;
		inf.nattype = m_nat;
		ptl_report_nattype(inf);
	}
}
void UDPStunClient::on_timer(int e)
{
	switch(e)
	{
	case TIMER_CHECK_BINDING_ADDR:
		{
			ptl_request_binding_addr();
		}
		break;
	case TIMER_NAT_RESEND:
		send_nat_request();
		break;
	case TIMER_CHECK_NAT:
		{
			if(REQS_FINI==m_state && (m_nat<1||m_nat>4))
				start_check_nat_type();
		}
		break;
	default:
		assert(false);
		break;
	}
}
void UDPStunClient::on_data(char* buf,int size,sockaddr_in& addr)
{
	PTL_STUN_Header_t head;
	PTLStream ss(buf,size,size);
	if(0!=ss>>head)
		return;
	
	switch(head.cmd)
	{
	case PTL_STUN_RSP_BINDING_ADDR:
		{
			PTL_STUN_RspBindingAddr_t rsp;
			if(0==(ss>>rsp))
			{
				//在此可以检查是否为一个本地IP
				//Util::is_private_ip(Util::ip_htoa(rsp.eyeIP));
				//UACLOG("#:-udp acceptor binding addr: %s:%d \n",Util::ip_htoa(rsp.eyeIP),rsp.eyePort);
				if(m_binding_ip!=rsp.eyeIP || m_binding_port!=rsp.eyePort)
				{
					//UACLOG("#:-udp acceptor binding addr changed to: %s:%d \n",Util::ip_htoa(rsp.eyeIP),rsp.eyePort);
					m_binding_ip = rsp.eyeIP;
					m_binding_port = rsp.eyePort;
					fire(UDPStunClientListener::BindingAddrChanged(),m_binding_ip,m_binding_port);
					if(6!=m_nat)
					{
						PTL_STUN_ReportNattype_t inf;
						inf.nattype = m_nat+10; //这个记录表示端口发生了变化
						ptl_report_nattype(inf);
					}
				}
			}
		}
		break;
	case PTL_STUN_RSP_STUNB_ADDR:
		{
			PTL_STUN_RspStunBAddr_t rsp;
			if(0==ss>>rsp)
			{
				m_stunconf.stunB_ip = rsp.stunB_ip;
				m_stunconf.stunB_port1 = rsp.stunB_port1;
				m_stunconf.stunC_ip = rsp.stunC_ip;
				m_stunconf.stunC_port1 = rsp.stunC_port1;
				string ipa = Util::ip_htoa(m_stunconf.stunA_ip);
				string ipb = Util::ip_htoa(m_stunconf.stunB_ip);
				string ipc = Util::ip_htoa(m_stunconf.stunC_ip);
				UACLOG("# on rsp stun addr: stunA(%s:%d),stunB(%s:%d) stunC(%s:%d) \n"
					,ipa.c_str(),m_stunconf.stunA_port1
					,ipb.c_str(),m_stunconf.stunB_port1
					,ipc.c_str(),m_stunconf.stunC_port1);
				m_resend_count = 0;
				m_state = REQS_NAT1;
			}
		}
		break;
	case PTL_STUN_RSP_NAT1:
		{
			uint32 ip = ntohl(addr.sin_addr.s_addr);
			//uint16 port = ntohs(addr.sin_port);
			PTL_STUN_RspNat1_t rsp;
			if(0!=ss >> rsp)
				break;
			if(ip == m_stunconf.stunB_ip)
			{
				on_nat_ok(1);
			}
		}
		break;
	case PTL_STUN_RSP_NAT4:
		{
			//目前发现虽然可能两stun 返回的端口是一样. 但再向其它IP:PORT发送则端口不一样了.即会将NAT4误判为NAT3
			uint32 ip = ntohl(addr.sin_addr.s_addr);
			uint16 port = ntohs(addr.sin_port);
			PTL_STUN_RspNat4_t rsp;
			if(0!=ss >> rsp)
				break;
			if(ip==m_stunconf.stunA_ip && port==m_stunconf.stunA_port1)
				m_nat4_port1 = rsp.eyePort;
			if(ip==m_stunconf.stunB_ip && port==m_stunconf.stunB_port1)
				m_nat4_port2 = rsp.eyePort;
			if(m_nat4_port1 && m_nat4_port2)
			{
				if(m_nat4_port1!=m_nat4_port2)
				{
					on_nat_ok(4);
				}
				else
				{
					m_resend_count = 0;
					m_state = REQS_NAT2;
				}
			}
		}
		break;
	case PTL_STUN_RSP_NAT2:
		{
			uint32 ip = ntohl(addr.sin_addr.s_addr);
			//uint16 port = ntohs(addr.sin_port);
			PTL_STUN_RspNat2_t rsp;
			if(0!=ss >> rsp)
				break;
			if(ip == m_stunconf.stunA_ip)
			{
				on_nat_ok(2);
			}
		}
		break;
	case PTL_STUN_REQ_HOLE:
		{
			PTL_STUN_ReqHole_t req;
			PTL_STUN_RspHole_t rsp;
			if(0!=ss>>req)
				break;
			rsp.connid = req.connid;
			rsp.des_nip = req.src_nip;
			rsp.des_nport = req.src_nport;
			rsp.src_nip = req.des_nip;
			rsp.src_nport = req.des_nport;
			ptl_stun_send_packet_T1(m_fd,PTL_STUN_RSP_HOLE,rsp,addr);
			fire(UDPStunClientListener::ReqHole(),req.connid,req.src_nip,req.src_nport);
		}
		break;
	case PTL_STUN_RSP_HOLE:
		{
			PTL_STUN_RspHole_t rsp;
			if(0!=ss>>rsp)
				break;
			fire(UDPStunClientListener::RspHole(),rsp.connid,rsp.src_nip,rsp.src_nport);
		}
		break;
	default:
		assert(0);
		break;
	}
	assert(0==ss.length());
}

int UDPStunClient::send_nat_request()
{
	switch(m_state)
	{
	case REQS_STUNB_ADDR:
		{
			if(m_resend_count>=4)
			{
				on_nat_ok(5);
				return 0;
			}
			m_resend_count++;
			return ptl_request_stunb_addr();
		}
		break;
	case REQS_NAT1:
		{
			if(m_resend_count>=4)
			{
				m_resend_count = 0;
				m_state = REQS_NAT4;
				return send_nat_request();
			}
			m_resend_count++;
			return ptl_request_nat1();
		}
		break;
	case REQS_NAT4:
		{
			if(m_resend_count>=4)
			{
				on_nat_ok(5);
				return 0;
			}
			m_resend_count++;
			return ptl_request_nat4();
		}
		break;
	case REQS_NAT2:
		{
			if(m_resend_count>=4)
			{
				on_nat_ok(3);
				return 0;
			}
			m_resend_count++;
			return ptl_request_nat2();
		}
		break;
	default:
		assert(0);
		break;
	}
	return -1;
}
int UDPStunClient::ptl_request_binding_addr()
{
	unsigned int ip;
	unsigned short port;
	m_reqbindaddr_count++;
	//int n = m_reqbindaddr_count%3;
	//if(1==n)
	//{
	//	ip = stunA_ip();
	//	port = m_stunconf.stunA_port1;
	//}
	//else if(2==n)
	//{
	//	ip = m_stunconf.stunB_ip;
	//	port = m_stunconf.stunB_port1;
	//}
	//else if(0==n)
	//{
	//	ip = m_stunconf.stunC_ip;
	//	port = m_stunconf.stunC_port1;
	//}
	//if(0==ip)
	{
		ip = stunA_ip();
		port = m_stunconf.stunA_port1;
	}
	//UACLOG("# UDPStunClient::ptl_request_binding_addr(%s,%d) \n",Util::ip_htoa(ip),port);
	return ptl_stun_send_packet3(m_fd,PTL_STUN_REQ_BINDING_ADDR,ip,port);
}


int UDPStunClient::ptl_request_stunb_addr()
{
	//UACLOG("# UDPStunClient::ptl_request_stunb_addr() \n");
	return ptl_stun_send_packet3(m_fd,PTL_STUN_REQ_STUNB_ADDR,stunA_ip(),m_stunconf.stunA_port1);
}
int UDPStunClient::ptl_request_nat1()
{
	//UACLOG("# UDPStunClient::ptl_request_nat1() \n");
	return ptl_stun_send_packet3(m_fd,PTL_STUN_REQ_NAT1,stunA_ip(),m_stunconf.stunA_port1);
}

int UDPStunClient::ptl_request_nat4()
{
	if(0==m_nat4_port1)
	{
		//UACLOG("# UDPStunClient::ptl_request_nat4() 1 \n");
		ptl_stun_send_packet3(m_fd,PTL_STUN_REQ_NAT4,stunA_ip(),m_stunconf.stunA_port1);
	}
	if(0==m_nat4_port2)
	{
		//UACLOG("# UDPStunClient::ptl_request_nat4() 2 \n");
		ptl_stun_send_packet3(m_fd,PTL_STUN_REQ_NAT4,m_stunconf.stunB_ip,m_stunconf.stunB_port1);
	}
	return 0;
}
int UDPStunClient::ptl_request_nat2()
{
	//UACLOG("# UDPStunClient::ptl_request_nat2() \n");
	return ptl_stun_send_packet3(m_fd,PTL_STUN_REQ_NAT2,stunA_ip(),m_stunconf.stunA_port1);
}
int UDPStunClient::ptl_request_hole(unsigned int connid,unsigned int nip,unsigned short nport)
{
	//UACLOG("# UDPStunClient::ptl_request_hole() \n");
	PTL_STUN_ReqHole_t req;
	req.connid = connid;
	req.des_nip = nip;
	req.des_nport = nport;
	return ptl_stun_send_packet_T3(m_fd,PTL_STUN_REQ_HOLE,req,stunA_ip(),m_stunconf.stunA_port1);
}
int UDPStunClient::ptl_report_nattype(const PTL_STUN_ReportNattype_t& inf)
{
	return ptl_stun_send_packet_T3(m_fd,PTL_STUN_REPORT_NATTYPE,inf,stunA_ip(),m_stunconf.stunA_port1);
}
int UDPStunClient::ptl_report_connsf(PTL_STUN_ReportConnSF_t& inf)
{
	inf.src_nattype = m_nat;
	return ptl_stun_send_packet_T3(m_fd,PTL_STUN_REPORT_CONN_SF,inf,stunA_ip(),m_stunconf.stunA_port1);
}
int UDPStunClient::ptl_report_connstat(PTL_STUN_ReportConnStat_t& inf)
{
	inf.src_nattype = m_nat;
	return ptl_stun_send_packet_T3(m_fd,PTL_STUN_REPORT_CONN_STAT,inf,stunA_ip(),m_stunconf.stunA_port1);
}


}

