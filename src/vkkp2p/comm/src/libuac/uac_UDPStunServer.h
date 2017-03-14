#pragma once

#include "uac_UDPStunProtocol.h"
#include "uac_SockAcceptor.h"

namespace UAC
{
class StunServer : public SockAcceptor
{
private:
	StunServer(void);
public:
	virtual ~StunServer(void);

public:
	static int stun_open(const PTL_STUN_RspStunsvrConfig_t& config);
	static void stun_close();
	static int stun_check_config(PTL_STUN_RspStunsvrConfig_t& local_config,PTL_STUN_RspStunsvrConfig_t& remote_config);
	static int handle_root(unsigned long delay_usec=0);
private:
	int open(unsigned short port,const char* ip,const PTL_STUN_RspStunsvrConfig_t& config,StunServer* stun2);
	void close();

public:
	int get_cmd_num(int& cmd_num,int& wrong_cmd_num)
	{
		cmd_num = m_cmd_num;
		wrong_cmd_num = m_wrong_cmd_num;
		return 0;
	}
	void reset_cmd_num()
	{
		m_cmd_num = 0;
		m_wrong_cmd_num = 0;
	}
	virtual int handle_input();
private:
	void on_line(PTLStream& ss,sockaddr_in& addr);
	void on_ptl_request_binding_addr(sockaddr_in& addr);
	void on_ptl_request_stunb_addr(sockaddr_in& addr);
	void on_ptl_request_stunsvr_config(const sockaddr_in& addr);
	void on_ptl_response_stunsvr_config(const sockaddr_in& addr,const PTL_STUN_RspStunsvrConfig_t& rsp);
	void on_ptl_request_nat1(const sockaddr_in& addr);
	void on_ptl_response_nat1(const sockaddr_in& addr,const PTL_STUN_RspNat1_t& rsp);
	void on_ptl_request_nat4(const sockaddr_in& addr);
	void on_ptl_request_nat2(const sockaddr_in& addr);
	void on_ptl_request_hole(const sockaddr_in& addr,PTL_STUN_ReqHole_t& req);
	void on_ptl_response_hole(const sockaddr_in& addr,const PTL_STUN_RspHole_t& req);
	void on_ptl_report_nattype(const sockaddr_in& addr,const PTL_STUN_ReportNattype_t& inf);
	void on_ptl_report_connsf(const sockaddr_in& addr,const PTL_STUN_ReportConnSF_t& inf);
	void on_ptl_report_connstat(const sockaddr_in& addr,const PTL_STUN_ReportConnStat_t& inf);
	
	int ptl_request_stuns_config();

private:
	int					m_cmd_num,m_wrong_cmd_num;
	StunServer			*m_stun2;
	PTL_STUN_RspStunsvrConfig_t		m_local_config;
	PTL_STUN_RspStunsvrConfig_t		m_remote_config;
	int								m_get_config_state; //0:ø’œ–£¨1£∫ªÒ»°ΩÙ£¨2£∫OK

	PTLStream			read_ss,write_ss;
	sockaddr_in			tmp_addr;
	sockaddr_in			tmp_recv_addr;
	socklen_t			tmp_recv_addrlen;
	int					tmp_ret;
	char				tmp_buf[2048];

	sockaddr_in			nat1_addr;

	PTL_STUN_Header_t			head;
	PTL_STUN_RspBindingAddr_t	rsp_binding;
	PTL_STUN_RspStunBAddr_t		rsp_stunb;
	PTL_STUN_RspStunsvrConfig_t	rsp_config;
	PTL_STUN_RspNat1_t			rsp_nat1;
	PTL_STUN_RspNat2_t			rsp_nat2;
	PTL_STUN_RspNat4_t			rsp_nat4;
	PTL_STUN_ReqHole_t			req_hole;
	PTL_STUN_RspHole_t			rsp_hole;
	PTL_STUN_ReportNattype_t	inf_nattype;
	PTL_STUN_ReportConnSF_t		inf_connsf;
	PTL_STUN_ReportConnStat_t	inf_connstat;
};

}



