#pragma once
#include "SerialStream.h"

//*************************
//peer to stun
enum PTLP2S
{
	PTL_P2S_                             = 17207 
	,PTL_P2S_REQUEST_TCP_CHECK           = PTL_P2S_ + 1
	,PTL_P2S_RESPONSE_TCP_CHECK          = PTL_P2S_ + 2
	,PTL_P2S_REQUEST_SERVER              = PTL_P2S_ + 3
	,PTL_P2S_RESPONSE_SERVER             = PTL_P2S_ + 4
	,PTL_P2S_REQUEST_NAT1                = PTL_P2S_ + 5
	,PTL_P2S_RESPONSE_NAT1               = PTL_P2S_ + 6
	,PTL_P2S_REQUEST_NAT2                = PTL_P2S_ + 7
	,PTL_P2S_RESPONSE_NAT2               = PTL_P2S_ + 8

	,PTL_P2S_REQUEST_NAT4                = PTL_P2S_ + 11
	,PTL_P2S_RESPONSE_NAT4               = PTL_P2S_ + 12
	,PTL_P2S_REQUEST_PULSE               = PTL_P2S_ + 13
	,PTL_P2S_RESPONSE_PULSE              = PTL_P2S_ + 14
	,PTL_P2S_REQUEST_CONFIG              = PTL_P2S_ + 15
	,PTL_P2S_RESPONSE_CONFIG             = PTL_P2S_ + 16
};


typedef struct tagPTL_P2S_RequestTcpCheck
{
	uint16    tcpPort;
}PTL_P2S_RequestTcpCheck;

typedef struct tagPTL_P2S_ResponseTcpCheck
{
	uint32    result;  //0:�ɹ���-1ʧ��
	uint32    id;    //��ͨ���յ�������(�ڴ�ȡwelcome�е�uid/sessionID),���Է��ظ��ͻ�����֤�Ƿ�������Ӷ�
	uint32    eyeIP;
}PTL_P2S_ResponseTcpCheck;

////�հ�
//typedef struct tagPTL_P2S_RequestServer
//{
//}PTL_P2S_RequestServer;

typedef struct tagPTL_P2S_ResponseServer
{
	uint32    result;
	uint32    eyeIP;
	uint32    eyePort;
	uint32    stunIPA;
	uint32    stunIPB;
	uint16    stunPortA;
	uint16    stunPortB;
}PTL_P2S_ResponseServer;

////�հ�
//typedef struct tagPTL_P2S_RequestNat1
//{
//}PTL_P2S_RequestNat1;


typedef struct tagPTL_P2S_ResponseNat1
{
	uint32    desIP;   //����ת��
	uint16    desPort; //����ת��
}PTL_P2S_ResponseNat1;

////�հ�
//typedef struct tagPTL_P2S_RequestNat2
//{
//}PTL_P2S_RequestNat2;


typedef struct tagPTL_P2S_ResponseNat2
{
	uint32    desIP;   //����ת��
	uint16    desPort; //����ת��
}PTL_P2S_ResponseNat2;


typedef struct tagPTL_P2S_RequestNat4
{
	uint32   flag; //
}PTL_P2S_RequestNat4;

typedef struct tagPTL_P2S_ResponseNat4
{
	uint32  flag;
	uint32  eyeIP;
	uint32  eyePort;
}PTL_P2S_ResponseNat4;

//typedef struct tagPTL_P2S_RequestPulse
//{
//}PTL_P2S_RequestPulse;

typedef struct tagPTL_P2S_ResponsePulse
{
	uint32          eyeIP;
	uint32          eyePort;
}PTL_P2S_ResponsePulse;

//typedef struct tagPTL_P2S_RequestConfig
//{
//}PTL_P2S_RequestConfig;

typedef struct tagPTL_P2S_ResponseConfig
{
	uint32          eyeIP;
	uint32          eyePort;

	uint32			accept_ip;
	uint16			accept_port;
	uint16			accept_port2;
	uint32			stunA_ip;
	uint16			stunA_port;
	uint32			stunB_ip;
	uint16			stunB_port;
	uint16			stunB_port2;
	uint16			is_stunB_port_rspnat1;

}PTL_P2S_ResponseConfig;


//˵��
//nat1: ��ͬIP��ͬPORT����ֱ�����ӽ���
//nat2: ��ͬIP��ͬPORT�������ӽ���,���ҷ��Ͳ�ͬIP:PORT,�Է��ɼ��˿���ͬ
//nat3: ��ͬIP��ͬPORT�������ӽ���,���ҷ��Ͳ�ͬIP:PORT,�Է��ɼ��˿���ͬ
//nat4: �Բ�ͬIP���Ǳ�˿�

//��������
//      mat1,nat2,nat3,nat4
//nat1   1    1    1    1
//nat2   1    1    1    1  
//nat3   1    1    1    0
//nat4   1    1    0    0

//���������ص�ʱ,������Ҫ��һ�����Ǳ�˿ڵ�

//����˳����: ��nat1,��nat4,��nat2,nat3. �ղ���UDP�ľ���nat5

//*************************
int operator << (PTLStream& ss, const PTL_P2S_RequestTcpCheck& inf);
int operator >> (PTLStream& ss, PTL_P2S_RequestTcpCheck& inf);
int operator << (PTLStream& ss, const PTL_P2S_ResponseTcpCheck& inf);
int operator >> (PTLStream& ss, PTL_P2S_ResponseTcpCheck& inf);
int operator << (PTLStream& ss, const PTL_P2S_ResponseServer& inf);
int operator >> (PTLStream& ss, PTL_P2S_ResponseServer& inf);
int operator << (PTLStream& ss, const PTL_P2S_ResponseNat1& inf);
int operator >> (PTLStream& ss, PTL_P2S_ResponseNat1& inf);
int operator << (PTLStream& ss, const PTL_P2S_ResponseNat2& inf);
int operator >> (PTLStream& ss, PTL_P2S_ResponseNat2& inf);
int operator << (PTLStream& ss, const PTL_P2S_RequestNat4& inf);
int operator >> (PTLStream& ss, PTL_P2S_RequestNat4& inf);
int operator << (PTLStream& ss, const PTL_P2S_ResponseNat4& inf);
int operator >> (PTLStream& ss, PTL_P2S_ResponseNat4& inf);
int operator << (PTLStream& ss, const PTL_P2S_ResponsePulse& inf);
int operator >> (PTLStream& ss, PTL_P2S_ResponsePulse& inf);
int operator << (PTLStream& ss, const PTL_P2S_ResponseConfig& inf);
int operator >> (PTLStream& ss, PTL_P2S_ResponseConfig& inf);
//int operator << (PTLStream& ss, const & inf);
//int operator >> (PTLStream& ss, & inf);

