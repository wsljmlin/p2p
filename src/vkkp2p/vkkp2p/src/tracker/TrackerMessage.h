#pragma once
#include "commons.h"

typedef struct tagMsgTrackerInfo
{
	string begin_time;
	int tracker_id;
	int tracker_ver;
	int user_num;
	int peer_num;
	int file_num;

	int ut_client;
	int ut_server;
	int ut_center;
	int ut_super;

	int nt[7];

}MsgTrackerInfo;

typedef struct tagMsgServerNode
{
	puid_t            uid;
	uint32           ver;
	char             ntype;
	char             utype; // user type
	uint32           menu;
	uint32           sessionID;
	uint32           beginTime;
	uint32           sourceNum;
	uint32           tcpRealIP;
	uint32           udpRealIP;
	uint16           tcpRealPort;
	uint16           udpRealPort;
}MsgServerNode;
typedef struct tagMsgServerInfo
{
	list<MsgServerNode*> svrs;
}MsgServerInfo;

typedef struct tagMsgFileInfo
{
	hash_t hash;
	int result;
	int user_num;
	int server_num;
	int center_num;
	int super_num;
	list<int> svrs;
	tagMsgFileInfo(void)
		:result(0)
		,user_num(0)
		,server_num(0)
		,center_num(0)
		,super_num(0)
	{}
}MsgFileInfo;


