#pragma once

#include "commons.h"

#ifdef SM_VOD
#include "SynchroObj.h"
#endif

typedef struct tagUserInfo
{
	char            flag;                    //0 表示没有被使用，  1表示已经被使用
	char            bshare;                  //是否共享
	puid_t           uid;                     //用户user id
	uint32          version;
	uint32          sysversion;              //操作系统
	uint32          sessionID;
	uint32          ip;                      //这个IP为tracker所见到的peer外网IP，用作源map排序用，以后不能变。不用tcpRealIP代替，tcpRealIP可能会变
	char            mac[6];
	uint32          totalDown_KB;               //总的下载量, KB(单位)
	uint32          totalUpload_KB;             //总的上传量，KB(单位)
	uint32          totalRequestNum;         //请求文件的个数
//	uint32          totalShareFileNum;            //用户本共享文件个数,sourceMap.size()
	char            dump;                    //上次是否异常退出

	uint32			downloadlist_maxnum;	//最大并发直播加速
	uint32			downloadlist_num;		//当前共享的playlist数
	time_t          beginTime;               //用户登录时间
	time_t          lastActiveTime;          //
	char            userType;             // 1表示普通peer,  2表示服务器    3表示为center分发源
	char            natType;
	uint32          menu;
	uint32          tcpLocalIP;          //上报tcp本地IP
	uint32          tcpRealIP;          //上报tcp本地IP
	short           tcpLocalPort;        //上报tcp本地端口
	short           tcpRealPort;        //上报tcp本地端口
	uint32          udpLocalIP;          //上报udp本地IP
	uint32          udpRealIP;          //上报udp本地IP
	short           udpLocalPort;        //上报udp本地端口
	short           udpRealPort;        //上报udp本地端口
	map<hash_t,uint32> sourceMap;       //共享文件列表map<hash,urlflag>
#ifdef SM_VOD
	map<hash_t, map<hash_t, uint32> > sourceMapvod;
	Simple_Mutex sourceMapvod_mux;
#endif
	int				source_empty_count;  //sourceMap空表计数，非空时重置0。当计数超过一定时间，断线。

	tagUserInfo(void){ reset();}
	void reset()
	{
		flag = 0;
		bshare = 1;
		memset(uid,0,PUIDLEN);
		version = 0;
		sysversion = 0;
		sessionID = 0;
		ip = 0;
		memset(mac,0,sizeof(mac));
		totalDown_KB = 0;
		totalUpload_KB = 0;
		totalRequestNum = 0;
		dump = 0;
		
		downloadlist_maxnum = 5;
		downloadlist_num = 0;
		beginTime = 0;
		lastActiveTime = 0;
		userType = 0;
		natType = 6;  //默认未知
		menu = 0;
		tcpLocalIP = 0;
		tcpRealIP = 0;
		tcpLocalPort = 0;
		tcpRealPort = 0;
		udpLocalIP = 0;
		udpRealIP = 0;
		udpLocalPort = 0;
		udpRealPort = 0;
		sourceMap.clear();
		source_empty_count = 0;
	}
}UserInfo;

