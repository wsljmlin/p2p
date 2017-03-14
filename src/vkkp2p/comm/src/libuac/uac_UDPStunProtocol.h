#pragma once

#include "uac_basetypes.h"
#include "uac_SerialStream.h"

namespace UAC
{
//NAT 的UDP简单穿越
//STUN --- Simple Traversal of UDP over NATs
//STUN --- Simple Traversal of User Datagram Protocol (UDP) Through Network Address Translators (NATs) 
//**********************************
//nat类型可连接通性匹配
//
//        nat1   nat2   nat3   nat4
// nat1    1      1      1      1
// nat2    1      1      1      1
// nat3    1      1      1      0
// nat4    1      1      0      0
//
//**********************************
//nat1: 完全透明NAT(Full Cone NAT)
//nat2: 受限NAT(Restricted Cone)
//nat3: 端口受限NAT(Port Restricted Cone
//nat4: 对称NAT(Symmetric
//nat5: 无法接收外部包
//nat6: 末知类型

//注意: 
//1.有这样的NAT，当绑定的端口是7108，而外网也实际也只能向这个机器7108发送数据才收得到，而从这机器发出的数据包到外面，外面看到的却是另一
//个端口如12345，而外面直接回复12345结果收不到。
//2.有一种nat4,它同时向两个IP探NA4包时，结果回来是一个端口结果误判为NAT3，但真实连接时又分配不同端口，即实质是NAT4类型
//3.有些客户连IP1与连IP2，外部所看到的IP不一样

//包标志: STUN_HEAD_STX   0xd0
//注意: 如果是UDP其它命令辅助协议前4位一定不能与此包标志相同

/**********************************************************************************************************
协议说明：
	stun server由两个不同IP地址的服务器对组成,设A、B服务器。
	A、B分别绑定两个端口 --- S1(SAIP:P1),S2(SAIP:P2),S3(SBIP:P1),S4(SBIP:P2)
协议：
1.req-binding-addr / rsp-binding-addr				：获取本地绑定的外网IP：PORT
2.req-stuns-addr / rsp-stuns-addr					：获取stun server :A、B的信息S1,S4
3.req-full-conn / rsp-full-conn 					：探NAT1。 发S1，S4回，收到即是NAT1
4.req-symmetric / rsp-symmetric						：探NAT4。 发S1、S3，S1、S3回IP：PORT，不相同即是NA4
5.req-restricted-conn /rsp-restricted-conn			：探NAT2。 发S1、S2回，收到即NA2，不然为NAT3
**********************************************************************************************************/

#define PTL_STUN_HEAD_STX 0x8f
//包命令类型
enum PTL_STUN_COMMAND{
	PTL_STUN_CMD_BEGIN	= 80,
	PTL_STUN_REQ_UID				=(PTL_STUN_CMD_BEGIN + 1),  //获取uid,用于后续命令的认证
	PTL_STUN_RSP_UID				=(PTL_STUN_CMD_BEGIN + 2),  //返回uid,用于后续命令的认证
	PTL_STUN_REQ_BINDING_ADDR		=(PTL_STUN_CMD_BEGIN + 3),  //获取绑定IP：PORT
	PTL_STUN_RSP_BINDING_ADDR		=(PTL_STUN_CMD_BEGIN + 4),  //
	PTL_STUN_REQ_STUNB_ADDR			=(PTL_STUN_CMD_BEGIN + 5),  //获取S3(SBIP:P1)
	PTL_STUN_RSP_STUNB_ADDR			=(PTL_STUN_CMD_BEGIN + 6),  //
	PTL_STUN_REQ_STUNSVR_CONFIG		=(PTL_STUN_CMD_BEGIN + 7),  //获取SB的配置
	PTL_STUN_RSP_STUNSVR_CONFIG		=(PTL_STUN_CMD_BEGIN + 8),  //
	PTL_STUN_REQ_NAT1				=(PTL_STUN_CMD_BEGIN + 9),  //测NAT1
	PTL_STUN_RSP_NAT1				=(PTL_STUN_CMD_BEGIN + 10),  //
	PTL_STUN_REQ_NAT4				=(PTL_STUN_CMD_BEGIN + 11),  //测NAT4
	PTL_STUN_RSP_NAT4				=(PTL_STUN_CMD_BEGIN + 12),  //
	PTL_STUN_REQ_NAT2				=(PTL_STUN_CMD_BEGIN + 13),  //测NAT2
	PTL_STUN_RSP_NAT2				=(PTL_STUN_CMD_BEGIN + 14),  //
	PTL_STUN_REQ_HOLE				=(PTL_STUN_CMD_BEGIN + 15),  //请求目标发NAT_HOLE包,stun 转发给另一端
	PTL_STUN_RSP_HOLE				=(PTL_STUN_CMD_BEGIN + 16),  //请求目标发NAT_HOLE包,stun 转发给另一端

	//统计命令
	PTL_STUN_REPORT_NATTYPE			=(PTL_STUN_CMD_BEGIN + 21), //上报NAT类型
	PTL_STUN_REPORT_CONN_SF			=(PTL_STUN_CMD_BEGIN + 22), //上报连接成功失败
	PTL_STUN_REPORT_CONN_STAT		=(PTL_STUN_CMD_BEGIN + 23), //上报
};

//注意:
//这里 des_nip:des_nport; src_nip:src_nport 使用网络序
//其它使用本机序


#define PTL_STUN_VERSION 1002
//版本1，目前不支持检验
//头结构  1字节
typedef struct tagPTL_STUN_Header
{
	uchar		stx;				//
	uchar		cmd;                //命令值
	uint32		uid;				//
	uint32		uid_checksum;
	tagPTL_STUN_Header(void):stx(PTL_STUN_HEAD_STX),cmd(0),uid(0),uid_checksum(0){}
}PTL_STUN_Header_t;

//BindingAddr
//typedef struct tagPTL_STUN_ReqBindingAddr
//{
//}PTL_STUN_ReqBindingAddr_t;
typedef struct tagPTL_STUN_RspBindingAddr
{
	uint32    eyeIP;
	uint16    eyePort;
}PTL_STUN_RspBindingAddr_t;

//StunsAddr
//typedef struct tagPTL_STUN_ReqStunBAddr
//{
//}PTL_STUN_ReqStunBAddr_t;
typedef struct tagPTL_STUN_RspStunBAddr
{
	uint32	stunB_ip;
	uint16	stunB_port1;
	uint32	stunC_ip;
	uint16	stunC_port1;
}PTL_STUN_RspStunBAddr_t;

//StunsConfig
//typedef struct tagPTL_STUN_ReqStunsvrConfig
//{
//}PTL_STUN_ReqStunsvrConfig_t;
typedef struct tagPTL_STUN_RspStunsvrConfig
{
	uint32          eyeIP;
	uint16          eyePort;

	uint32			accept_ip;
	uint16			accept_port1;
	uint16			accept_port2;
	uint32			stunB_ip;
	uint16			stunB_port1;
	uint16			stunB_port2;
	uint32			stunC_ip;
	uint16			stunC_port1;
}PTL_STUN_RspStunsvrConfig_t;

//Nat1
//typedef struct tagPTL_STUN_ReqNat1
//{
//}PTL_STUN_ReqNat1_t;
typedef struct tagPTL_STUN_RspNat1
{
	uint32    des_nip;   //stun server端用于转发，client端忽略
	uint16    des_nport; //
}PTL_STUN_RspNat1_t;

////Nat4
//typedef struct tagPTL_STUN_ReqNat4
//{
//}PTL_STUN_ReqNat4_t;
typedef struct tagPTL_STUN_RspNat4
{
	uint32  eyeIP;
	uint32  eyePort;
}PTL_STUN_RspNat4_t;

//Nat2
//typedef struct tagPTL_STUN_ReqNat2
//{
//}PTL_STUN_ReqNat2_t;
typedef struct tagPTL_STUN_RspNat2
{
	uint32    des_nip;   //stun server端用于转发，client端忽略
	uint16    des_nport; //
}PTL_STUN_RspNat2_t;

//Hole
typedef struct tagPTL_STUN_ReqHole
{
	uint32		connid;	 //标识连接号，回复同样标识以确定是哪个连接发出的包
	uint32		des_nip;   //呼叫des_nip:des_nport 发NAT_TEST包给src_nip:src_nport
	uint16		des_nport; //
	uint32		src_nip;		//代表发起连接方
	uint16		src_nport;
}PTL_STUN_ReqHole_t;
typedef struct tagPTL_STUN_RspHole
{
	uint32		connid;	 //标识连接号，回复同样标识以确定是哪个连接发出的包
	uint32		des_nip;   //回复，
	uint16		des_nport; //
	uint32		src_nip;		//代表发起连接的另一方
	uint16		src_nport;
}PTL_STUN_RspHole_t;

//report nattype
typedef struct tagPTL_STUN_ReportNattype
{
	//ip:port由stunsvr自己获取
	uchar		nattype;
}PTL_STUN_ReportNattype_t;

//conn sf信息只有发起方上报
//report conn sf
typedef struct tagPTL_STUN_ReportConnSF
{
	//ip:port由stunsvr自己获取
	uchar		bsucceed;
	uchar		src_nattype;
	uint32		des_nip;   //
	uint16		des_nport; //
	uchar		des_nattype;
}PTL_STUN_ReportConnSF_t;

//conn 信息只有发送数据比接收数据多时写记录
//report conn stat
typedef struct tagPTL_STUN_ReportConnStat
{
	//ip:port由stunsvr自己获取
	uchar		src_nattype;
	uint32		des_nip;   //
	uint16		des_nport; //
	uchar		des_nattype;

	uint32		sizeKB; //有效传输总数据量KB
	uint32		sec;	//传输总秒数
	uint32		ttlMS;	//只取最后的记录
	uint32		send_num; //总共发出多少个有效包
	uint32		resend_num; //总共重复发了多少个包
	uint32		resend_timeo_num; //总共超时重发出多少个包
	uint32		other_rerecv_num; //对方重复收了多少个包
}PTL_STUN_ReportConnStat_t;

int operator << (PTLStream& ss, const PTL_STUN_Header_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_Header_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_RspBindingAddr_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_RspBindingAddr_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_RspStunBAddr_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_RspStunBAddr_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_RspStunsvrConfig_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_RspStunsvrConfig_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_RspNat1_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_RspNat1_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_RspNat4_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_RspNat4_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_RspNat2_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_RspNat2_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_ReqHole_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_ReqHole_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_RspHole_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_RspHole_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_ReportNattype_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_ReportNattype_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_ReportConnSF_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_ReportConnSF_t& inf);

int operator << (PTLStream& ss, const PTL_STUN_ReportConnStat_t& inf);
int operator >> (PTLStream& ss, PTL_STUN_ReportConnStat_t& inf);


template<typename T>
static int ptl_stun_send_packet_T1(SOCKET fd,uchar cmd,const T& inf,const sockaddr_in& addr)
{
	PTLStream ss(1024);
	PTL_STUN_Header_t header;
	header.cmd = cmd;
	ss << header;
	ss << inf;
	return sendto(fd,ss.buffer(),ss.length(),0,(sockaddr*)&addr,sizeof(addr));
}

template<typename T>
static int ptl_stun_send_packet_T2(SOCKET fd,uchar cmd,const T& inf,const sockaddr_in& addr,PTL_STUN_Header_t& header,PTLStream& ss)
{
	header.cmd = cmd;
	ss.zero_rw();
	ss << header;
	ss << inf;
	return sendto(fd,ss.buffer(),ss.length(),0,(sockaddr*)&addr,sizeof(addr));
}

template<typename T>
static int ptl_stun_send_packet_T3(SOCKET fd,uchar cmd,const T& inf,unsigned int ip,unsigned short port)
{
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(ip);
	addr.sin_port = htons(port);
	return ptl_stun_send_packet_T1(fd,cmd,inf,addr);
}


int ptl_stun_send_packet1(SOCKET fd,uchar cmd,const sockaddr_in& addr);
int ptl_stun_send_packet2(SOCKET fd,uchar cmd,const sockaddr_in& addr,PTL_STUN_Header_t& header,PTLStream& ss);
int ptl_stun_send_packet3(SOCKET fd,uchar cmd,unsigned int ip,unsigned short port);


}
