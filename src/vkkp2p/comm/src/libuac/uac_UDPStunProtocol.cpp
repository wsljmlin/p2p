#include "uac_UDPStunProtocol.h"

namespace UAC
{
int operator << (PTLStream& ss, const PTL_STUN_Header_t& inf)
{
	ss << inf.stx;
	ss << inf.cmd;
	//ss << inf.uid;
	//ss << inf.uid_checksum;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_Header_t& inf)
{
	ss >> inf.stx;
	ss >> inf.cmd;
	//ss >> inf.uid;
	//ss >> inf.uid_checksum;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_RspBindingAddr_t& inf)
{
	ss << inf.eyeIP;
	ss << inf.eyePort;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_RspBindingAddr_t& inf)
{
	ss >> inf.eyeIP;
	ss >> inf.eyePort;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_RspStunBAddr_t& inf)
{
	ss << inf.stunB_ip;
	ss << inf.stunB_port1;
	ss << inf.stunC_ip;
	ss << inf.stunC_port1;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_RspStunBAddr_t& inf)
{
	ss >> inf.stunB_ip;
	ss >> inf.stunB_port1;
	ss >> inf.stunC_ip;
	ss >> inf.stunC_port1;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_RspStunsvrConfig_t& inf)
{
	ss << inf.eyeIP;
	ss << inf.eyePort;
	ss << inf.accept_ip;
	ss << inf.accept_port1;
	ss << inf.accept_port2;
	ss << inf.stunB_ip;
	ss << inf.stunB_port1;
	ss << inf.stunB_port2;
	ss << inf.stunC_ip;
	ss << inf.stunC_port1;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_RspStunsvrConfig_t& inf)
{
	ss >> inf.eyeIP;
	ss >> inf.eyePort;
	ss >> inf.accept_ip;
	ss >> inf.accept_port1;
	ss >> inf.accept_port2;
	ss >> inf.stunB_ip;
	ss >> inf.stunB_port1;
	ss >> inf.stunB_port2;
	ss >> inf.stunC_ip;
	ss >> inf.stunC_port1;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_RspNat1_t& inf)
{
	ss << inf.des_nip;
	ss << inf.des_nport;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_RspNat1_t& inf)
{
	ss >> inf.des_nip;
	ss >> inf.des_nport;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_RspNat4_t& inf)
{
	ss << inf.eyeIP;
	ss << inf.eyePort;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_RspNat4_t& inf)
{
	ss >> inf.eyeIP;
	ss >> inf.eyePort;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_RspNat2_t& inf)
{
	ss << inf.des_nip;
	ss << inf.des_nport;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_RspNat2_t& inf)
{
	ss >> inf.des_nip;
	ss >> inf.des_nport;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_ReqHole_t& inf)
{
	ss << inf.connid;
	ss << inf.des_nip;
	ss << inf.des_nport;
	ss << inf.src_nip;
	ss << inf.src_nport;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_ReqHole_t& inf)
{
	ss >> inf.connid;
	ss >> inf.des_nip;
	ss >> inf.des_nport;
	ss >> inf.src_nip;
	ss >> inf.src_nport;
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_STUN_RspHole_t& inf)
{
	ss << inf.connid;
	ss << inf.des_nip;
	ss << inf.des_nport;
	ss << inf.src_nip;
	ss << inf.src_nport;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_RspHole_t& inf)
{
	ss >> inf.connid;
	ss >> inf.des_nip;
	ss >> inf.des_nport;
	ss >> inf.src_nip;
	ss >> inf.src_nport;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_ReportNattype_t& inf)
{
	ss << inf.nattype;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_ReportNattype_t& inf)
{
	ss >> inf.nattype;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_ReportConnSF_t& inf)
{
	ss << inf.bsucceed;
	ss << inf.src_nattype;
	ss << inf.des_nip;
	ss << inf.des_nport;
	ss << inf.des_nattype;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_ReportConnSF_t& inf)
{
	ss >> inf.bsucceed;
	ss >> inf.src_nattype;
	ss >> inf.des_nip;
	ss >> inf.des_nport;
	ss >> inf.des_nattype;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_STUN_ReportConnStat_t& inf)
{
	ss << inf.src_nattype;
	ss << inf.des_nip;
	ss << inf.des_nport;
	ss << inf.des_nattype;

	ss << inf.sizeKB;
	ss << inf.sec;
	ss << inf.ttlMS;
	ss << inf.send_num;
	ss << inf.resend_num;
	ss << inf.resend_timeo_num;
	ss << inf.other_rerecv_num;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_STUN_ReportConnStat_t& inf)
{
	ss >> inf.src_nattype;
	ss >> inf.des_nip;
	ss >> inf.des_nport;
	ss >> inf.des_nattype;

	ss >> inf.sizeKB;
	ss >> inf.sec;
	ss >> inf.ttlMS;
	ss >> inf.send_num;
	ss >> inf.resend_num;
	ss >> inf.resend_timeo_num;
	ss >> inf.other_rerecv_num;
	return ss.ok();
}


int ptl_stun_send_packet1(SOCKET fd,uchar cmd,const sockaddr_in& addr)
{
	PTLStream ss(1024);
	PTL_STUN_Header_t header;
	header.cmd = cmd;
	ss << header;
	return sendto(fd,ss.buffer(),ss.length(),0,(sockaddr*)&addr,sizeof(addr));
}
int ptl_stun_send_packet2(SOCKET fd,uchar cmd,const sockaddr_in& addr,PTL_STUN_Header_t& header,PTLStream& ss)
{
	header.cmd = cmd;
	ss.zero_rw();
	ss << header;
	return sendto(fd,ss.buffer(),ss.length(),0,(sockaddr*)&addr,sizeof(addr));
}
int ptl_stun_send_packet3(SOCKET fd,uchar cmd,unsigned int ip,unsigned short port)
{
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	return ptl_stun_send_packet1(fd,cmd,addr);
}

}

