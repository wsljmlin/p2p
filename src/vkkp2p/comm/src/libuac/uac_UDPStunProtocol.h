#pragma once

#include "uac_basetypes.h"
#include "uac_SerialStream.h"

namespace UAC
{
//NAT ��UDP�򵥴�Խ
//STUN --- Simple Traversal of UDP over NATs
//STUN --- Simple Traversal of User Datagram Protocol (UDP) Through Network Address Translators (NATs) 
//**********************************
//nat���Ϳ�����ͨ��ƥ��
//
//        nat1   nat2   nat3   nat4
// nat1    1      1      1      1
// nat2    1      1      1      1
// nat3    1      1      1      0
// nat4    1      1      0      0
//
//**********************************
//nat1: ��ȫ͸��NAT(Full Cone NAT)
//nat2: ����NAT(Restricted Cone)
//nat3: �˿�����NAT(Port Restricted Cone
//nat4: �Գ�NAT(Symmetric
//nat5: �޷������ⲿ��
//nat6: ĩ֪����

//ע��: 
//1.��������NAT�����󶨵Ķ˿���7108��������Ҳʵ��Ҳֻ�����������7108�������ݲ��յõ���������������������ݰ������棬���濴����ȴ����һ
//���˿���12345��������ֱ�ӻظ�12345����ղ�����
//2.��һ��nat4,��ͬʱ������IP̽NA4��ʱ�����������һ���˿ڽ������ΪNAT3������ʵ����ʱ�ַ��䲻ͬ�˿ڣ���ʵ����NAT4����
//3.��Щ�ͻ���IP1����IP2���ⲿ��������IP��һ��

//����־: STUN_HEAD_STX   0xd0
//ע��: �����UDP���������Э��ǰ4λһ��������˰���־��ͬ

/**********************************************************************************************************
Э��˵����
	stun server��������ͬIP��ַ�ķ����������,��A��B��������
	A��B�ֱ�������˿� --- S1(SAIP:P1),S2(SAIP:P2),S3(SBIP:P1),S4(SBIP:P2)
Э�飺
1.req-binding-addr / rsp-binding-addr				����ȡ���ذ󶨵�����IP��PORT
2.req-stuns-addr / rsp-stuns-addr					����ȡstun server :A��B����ϢS1,S4
3.req-full-conn / rsp-full-conn 					��̽NAT1�� ��S1��S4�أ��յ�����NAT1
4.req-symmetric / rsp-symmetric						��̽NAT4�� ��S1��S3��S1��S3��IP��PORT������ͬ����NA4
5.req-restricted-conn /rsp-restricted-conn			��̽NAT2�� ��S1��S2�أ��յ���NA2����ȻΪNAT3
**********************************************************************************************************/

#define PTL_STUN_HEAD_STX 0x8f
//����������
enum PTL_STUN_COMMAND{
	PTL_STUN_CMD_BEGIN	= 80,
	PTL_STUN_REQ_UID				=(PTL_STUN_CMD_BEGIN + 1),  //��ȡuid,���ں����������֤
	PTL_STUN_RSP_UID				=(PTL_STUN_CMD_BEGIN + 2),  //����uid,���ں����������֤
	PTL_STUN_REQ_BINDING_ADDR		=(PTL_STUN_CMD_BEGIN + 3),  //��ȡ��IP��PORT
	PTL_STUN_RSP_BINDING_ADDR		=(PTL_STUN_CMD_BEGIN + 4),  //
	PTL_STUN_REQ_STUNB_ADDR			=(PTL_STUN_CMD_BEGIN + 5),  //��ȡS3(SBIP:P1)
	PTL_STUN_RSP_STUNB_ADDR			=(PTL_STUN_CMD_BEGIN + 6),  //
	PTL_STUN_REQ_STUNSVR_CONFIG		=(PTL_STUN_CMD_BEGIN + 7),  //��ȡSB������
	PTL_STUN_RSP_STUNSVR_CONFIG		=(PTL_STUN_CMD_BEGIN + 8),  //
	PTL_STUN_REQ_NAT1				=(PTL_STUN_CMD_BEGIN + 9),  //��NAT1
	PTL_STUN_RSP_NAT1				=(PTL_STUN_CMD_BEGIN + 10),  //
	PTL_STUN_REQ_NAT4				=(PTL_STUN_CMD_BEGIN + 11),  //��NAT4
	PTL_STUN_RSP_NAT4				=(PTL_STUN_CMD_BEGIN + 12),  //
	PTL_STUN_REQ_NAT2				=(PTL_STUN_CMD_BEGIN + 13),  //��NAT2
	PTL_STUN_RSP_NAT2				=(PTL_STUN_CMD_BEGIN + 14),  //
	PTL_STUN_REQ_HOLE				=(PTL_STUN_CMD_BEGIN + 15),  //����Ŀ�귢NAT_HOLE��,stun ת������һ��
	PTL_STUN_RSP_HOLE				=(PTL_STUN_CMD_BEGIN + 16),  //����Ŀ�귢NAT_HOLE��,stun ת������һ��

	//ͳ������
	PTL_STUN_REPORT_NATTYPE			=(PTL_STUN_CMD_BEGIN + 21), //�ϱ�NAT����
	PTL_STUN_REPORT_CONN_SF			=(PTL_STUN_CMD_BEGIN + 22), //�ϱ����ӳɹ�ʧ��
	PTL_STUN_REPORT_CONN_STAT		=(PTL_STUN_CMD_BEGIN + 23), //�ϱ�
};

//ע��:
//���� des_nip:des_nport; src_nip:src_nport ʹ��������
//����ʹ�ñ�����


#define PTL_STUN_VERSION 1002
//�汾1��Ŀǰ��֧�ּ���
//ͷ�ṹ  1�ֽ�
typedef struct tagPTL_STUN_Header
{
	uchar		stx;				//
	uchar		cmd;                //����ֵ
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
	uint32    des_nip;   //stun server������ת����client�˺���
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
	uint32    des_nip;   //stun server������ת����client�˺���
	uint16    des_nport; //
}PTL_STUN_RspNat2_t;

//Hole
typedef struct tagPTL_STUN_ReqHole
{
	uint32		connid;	 //��ʶ���Ӻţ��ظ�ͬ����ʶ��ȷ�����ĸ����ӷ����İ�
	uint32		des_nip;   //����des_nip:des_nport ��NAT_TEST����src_nip:src_nport
	uint16		des_nport; //
	uint32		src_nip;		//���������ӷ�
	uint16		src_nport;
}PTL_STUN_ReqHole_t;
typedef struct tagPTL_STUN_RspHole
{
	uint32		connid;	 //��ʶ���Ӻţ��ظ�ͬ����ʶ��ȷ�����ĸ����ӷ����İ�
	uint32		des_nip;   //�ظ���
	uint16		des_nport; //
	uint32		src_nip;		//���������ӵ���һ��
	uint16		src_nport;
}PTL_STUN_RspHole_t;

//report nattype
typedef struct tagPTL_STUN_ReportNattype
{
	//ip:port��stunsvr�Լ���ȡ
	uchar		nattype;
}PTL_STUN_ReportNattype_t;

//conn sf��Ϣֻ�з����ϱ�
//report conn sf
typedef struct tagPTL_STUN_ReportConnSF
{
	//ip:port��stunsvr�Լ���ȡ
	uchar		bsucceed;
	uchar		src_nattype;
	uint32		des_nip;   //
	uint16		des_nport; //
	uchar		des_nattype;
}PTL_STUN_ReportConnSF_t;

//conn ��Ϣֻ�з������ݱȽ������ݶ�ʱд��¼
//report conn stat
typedef struct tagPTL_STUN_ReportConnStat
{
	//ip:port��stunsvr�Լ���ȡ
	uchar		src_nattype;
	uint32		des_nip;   //
	uint16		des_nport; //
	uchar		des_nattype;

	uint32		sizeKB; //��Ч������������KB
	uint32		sec;	//����������
	uint32		ttlMS;	//ֻȡ���ļ�¼
	uint32		send_num; //�ܹ��������ٸ���Ч��
	uint32		resend_num; //�ܹ��ظ����˶��ٸ���
	uint32		resend_timeo_num; //�ܹ���ʱ�ط������ٸ���
	uint32		other_rerecv_num; //�Է��ظ����˶��ٸ���
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
