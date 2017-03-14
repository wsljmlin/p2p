#pragma once

#include "SerialStream.h"
#include "nhash.h"
#include "ProtocolP2T.h"
#include "ProtocolP2S.h"
#include "ProtocolP2P.h"
#include "ProtocolMessage.h"

enum UserType
{ 
	UT_CLIENT=1		//�ͻ���
	,UT_SERVER=2	//һ���super node
	,UT_HTTP=3
	,UT_CENTER=4    //ĿǰԤ����ָ�ַ�Դ������ֱ��Դ
	,UT_SUPER=5
};

#ifdef SM_VOD
enum _ePlayType{
	PLAYTYPE_LIVE=0,
	PLAYTYPE_VOD,
};
#endif


#define  PTL_HEAD_STX		  ".vkk"
#define  PTL_HEAD_STX_32      *(uint32*)PTL_HEAD_STX

typedef struct tagPLTNetInfo
{
	bool isNatChecked;
	char natType;
	uint32 trackerID;
	uint32 trackerVer;
	uint32 sessionID;
	uint32 tcpRealIP;
	uint32 tcpLocalIP;
	uint16 tcpRealPort;
	uint16 tcpLocalPort;
	uint32 udpRealIP;
	uint32 udpLocalIP;
	uint16 udpRealPort;
	uint16 udpLocalPort;

	tagPLTNetInfo(void)
	{
		isNatChecked = false;
		natType = 6;
		trackerID = 0;
		trackerVer = 0;
		sessionID = 0;
		tcpRealIP = 0;
		tcpLocalIP = 0;
		tcpRealPort = 0;
		tcpLocalPort = 0;
		udpRealIP = 0;
		udpLocalIP = 0;
		udpRealPort = 0;
		udpLocalPort = 0;
	}
}PLTNetInfo_t;

extern PLTNetInfo_t g_netLiveInfo;
extern const char *g_str_usertype[6];


//**********************************************

//ͨ��Э���ͷ;19byte;
typedef struct tagPTL_Head
{
	char			 stx[4];           //��־
	uint32           size;             //��С
	char             mask;			   //0λ��ʾ�����������ֵ
	uint32           checksum;         //�����
	uint16           cmd;	           //�����־
	sint32           id;               //����
	tagPTL_Head(void) : size(0),mask(0),checksum(0),cmd(0),id(0){ }
}PTL_Head;


int operator << (PTLStream& ss, const PTL_Head& inf);
int operator >> (PTLStream& ss, PTL_Head& inf);
