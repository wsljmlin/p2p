#pragma once
#include "SerialStream.h"


//*************************
//peer to tracker
enum PTLP2T
{
	PTL_P2T_ = 17107
	,PTL_P2T_REQUEST_LOGIN				 = PTL_P2T_ + 1
	,PTL_P2T_RESPONSE_LOGIN              = PTL_P2T_ + 2
	,PTL_P2T_REPORT_LOGOUT               = PTL_P2T_ + 3
	,PTL_P2T_REPORT_NAT                  = PTL_P2T_ + 4
	,PTL_P2T_REPORT_SHARE_FILE           = PTL_P2T_ + 5
	,PTL_P2T_REPORT_REMOVE_FILE          = PTL_P2T_ + 6
	,PTL_P2T_REPORT_START_DOWNLOAD_FILE  = PTL_P2T_ + 7
	,PTL_P2T_REPORT_STOP_DOWNLOAD_FILE   = PTL_P2T_ + 8
	,PTL_P2T_REQUEST_FILE_SOURCE         = PTL_P2T_ + 9
	,PTL_P2T_RESPONSE_FILE_SOURCE        = PTL_P2T_ + 10
	,PTL_P2T_REPORT_DOWNLOAD_WRONG       = PTL_P2T_ + 11
	,PTL_P2T_REPORT_DOWNLOAD_FILE_SPEED  = PTL_P2T_ + 12
	,PTL_P2T_REPORT_STAT                 = PTL_P2T_ + 13
	,PTL_P2T_REPORT_ERROR                = PTL_P2T_ + 14
	,PTL_P2T_REQUEST_CONN_TURN           = PTL_P2T_ + 15
	,PTL_P2T_RESPONSE_CONN_TURN          = PTL_P2T_ + 16

	,PTL_P2T_REQUEST_KEEPLIVE            = PTL_P2T_ + 19
	,PTL_P2T_RESPONSE_KEEPLIVE           = PTL_P2T_ + 20
	,PTL_P2T_REQUEST_SERVER_LIST         = PTL_P2T_ + 21
	,PTL_P2T_RESPONSE_SERVER_LIST        = PTL_P2T_ + 22 

	,PTL_P2T_REPORT_DOWNLOAD_FILE_INFO		= PTL_P2T_ + 32
	,PTL_P2T_REPORT_START_DOWNLOAD_LIST		= PTL_P2T_ + 33
	,PTL_P2T_REQUEST_START_DOWNLOAD_LIST	= PTL_P2T_ + 34
	,PTL_P2T_REQUEST_STOP_DOWNLOAD_LIST		= PTL_P2T_ + 35
	,PTL_P2T_REPORT_DOWNLOADLIST_MAXNUM		= PTL_P2T_ + 36
};

//*************************************************************PTL_SUB_STRUCT********************************************

//size=56B
typedef  struct  tagPTL_P2T_FileInfo
{
	fhash_t         fhash;
	uint64          fsize;   //无穷大为-1，用于直播
#ifdef SM_VOD
	int filetype;
#endif
}PTL_P2T_FileInfo;

//size=38B
typedef  struct  tagPTL_P2T_PeerInfo
{
	uint32           trackID;      //指定属于哪个tracker,用于支持多tracker级连保留
	uint32           sessionID;     //tracker使用的sessionID,直接指tracker上面的索引
	char             utype;         //USER 类型 UT_CLIENT...
	char             ntype;         //nat类型0~6,6为未知
	uint32           menu;         //菜单功能位:低0位共享支持,低1位TCP支持,低2位UDP支持
	uint32           tcpLocalIP; 
	uint32           tcpRealIP;  
	uint16           tcpLocalPort;
	uint16           tcpRealPort;
	uint32           udpLocalIP;   
	uint32           udpRealIP;  
	uint16           udpLocalPort;
	uint16           udpRealPort;  
}PTL_P2T_PeerInfo;

//size=38B+PUIDLEN = 70B
typedef struct tagPTL_P2T_ServerInfo
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
	char             reserve[4];
}PTL_P2T_ServerInfo; 

////***********************************************************************************************************************
////peer to tracker
#define MENU_VIP 0x8
typedef struct tagPTL_P2T_RequestLogin
{
	puid_t          uid;    //用户ID
	char           utype;  //用户类型
	uint32         menu;   //菜单功能位:低0位共享支持,低1位TCP支持,低2位UDP支持,低3位表示是否为VIP
	uint32         ver;
	uint32         sysver;
	uchar          mac[6];
}PTL_P2T_RequestLogin;

typedef struct tagPTL_P2T_ResponseLogin
{
	sint32         result; 
	puid_t          uid;    //用户ID
	uint32         trackID;
	uint32         trackVer;
	uint32         sessionID;
	uint32         eyeIP;
	char           reserve[16];
}PTL_P2T_ResponseLogin;

//空包
//typedef struct tagPTL_P2T_ReportLogout
//{
//}PTL_P2T_ReportLogout;

typedef struct tagPTL_P2T_ReportNat
{
	char             ntype;                   //nat类型
	uint32           tcpLocalIP;  
	uint32           tcpRealIP;               //网络IP
	uint16           tcpLocalPort;
	uint16           tcpRealPort;
	uint32           udpLocalIP;    
	uint32           udpRealIP;               //网络IP          
	uint16           udpLocalPort;
	uint16           udpRealPort;  
}PTL_P2T_ReportNat;

typedef struct tagPTL_P2T_ReportShareFile
{
	uint32             num;
	PTL_P2T_FileInfo   files[20];
#ifdef SM_VOD
	int filetype; //share file type, 0 live, 1 vod
#endif
}PTL_P2T_ReportShareFile;

typedef struct tagPTL_P2T_ReportRemoveFile
{
	uint32             num;
	PTL_P2T_FileInfo   files[20];
#ifdef SM_VOD
	int filetype; //share file type, 0 live, 1 vod
#endif

}PTL_P2T_ReportRemoveFile;

typedef struct tagPTL_P2T_ReportStartDownloadFile
{
	fhash_t  fhash;
#ifdef SM_VOD
		int filetype; //share file type, 0 live, 1 vod
#endif

}PTL_P2T_ReportStartDownloadFile;

typedef struct tagPTL_P2T_ReportStopDownloadFile
{
	fhash_t  fhash;
#ifdef SM_VOD
		int filetype; //share file type, 0 live, 1 vod
#endif

}PTL_P2T_ReportStopDownloadFile;

typedef struct tagPTL_P2T_RequestFileSource
{
	fhash_t    fhash;
	uint32   maxnum;
#ifdef SM_VOD
	int filetype; //share file type, 0 live, 1 vod
#endif

}PTL_P2T_RequestFileSource;

typedef struct tagPTL_P2T_ResponseFileSource
{
	uint32            result;
	fhash_t			  fhash;
	uint64            fsize;
	uint32			  urlflag; //使用http url标志
	uint32            num;
	PTL_P2T_PeerInfo  peers[20];
}PTL_P2T_ResponseFileSource;

typedef struct tagPTL_P2T_ReportDownloadWrong
{
	fhash_t  fhash;
	fhash_t  fhashnew;
#ifdef SM_VOD
	int filetype; //share file type, 0 live, 1 vod
#endif

}PTL_P2T_ReportDownloadWrong;

typedef struct tagPTL_P2T_ReportDownloadFileSpeed
{
	fhash_t         fhash;
	uint64          size; 
	uint32          speed; // KB/s
	uint16          cacheTimes; //缓冲数 
	uint16          dragTimes; //拖动次数
	uint32          downSeconds; //下载时长(秒)
	uint32          cacheSenconds; //缓冲总时长
	char            reserve[16];
}PTL_P2T_ReportDownloadFileSpeed;

typedef struct tagPTL_P2T_ReportStat
{
	uint32        connSucceedPerNetT[5];    //nat0~nat4各数据
	uint32        connFailedPerNetT[5];
	uint32        downBytesPerIPT_KB[3];  //iptype_tcp/iptype_udp
	uint32        shareBytesPerIPT_KB[3];  //iptype_tcp/iptype_udp
	uint32        downBytesPerUserT_KB[6];  //预留可能增加多的类型
	uint16        shareSeconds;          //统计总共享时间
	uint16        downSeconds;
	char          reserve[16];
}PTL_P2T_ReportStat;

typedef struct tagPTL_P2T_ReportError
{
	uint16        dumpTimes;
	uint16        downWrongBlocks;   //下载错误block数
	char          reserve[32];
}PTL_P2T_ReportError;

typedef struct tagPTL_P2T_RequestConnTurn
{
	uint32        desTrackID;
	uint32        desSessionID;
}PTL_P2T_RequestConnTurn;

typedef struct tagPTL_P2T_ResponseConnTurn
{
	PTL_P2T_PeerInfo   desPeerInfo;
	char               reserve[4];
}PTL_P2T_ResponseConnTurn;

////keeplive 没有实体数据
//typedef struct tagPTL_P2T_RequestKeeplive
//{
//}PTL_P2T_RequestKeeplive;

////keeplive 没有实体数据
//typedef struct tagPTL_P2T_ResponseKeeplive
//{
//}PTL_P2T_ResponseKeeplive;

typedef struct tagPTL_P2T_RequestServerList
{
	int   maxnum;
}PTL_P2T_RequestServerList;

typedef struct tagPTL_P2T_ResponseServerList
{
	uint32              trackID;
	uint32              trackVer;
	uint32              userNum;
	uint32              beginTime;
	uint32              allNum;     //可能数量太多，分多包传送
	uint32              startNum;   //如果不为0，则为子包
	uint32              num;        //
	PTL_P2T_ServerInfo  servers[14]; //一次传14个左右,1K以内
}PTL_P2T_ResponseServerList;


typedef struct tagPTL_P2T_ReportDownloadFileInfo
{
	char			flag; //0表示下载过程中的，1表示下载停止时的记录
	fhash_t         fhash;
	uint64          size; 
	uint32          speed_KB; // KB/s
	uint32          downSeconds; //下载时长(秒)
	uint16          cacheTimes; //缓冲数 
	uint16          dragTimes; //拖动次数
	uint32          cacheSenconds; //缓冲总时长
	uint16			connSucceedPerNetT[5];    //nat0~nat4各数据
	uint16			connFailedPerNetT[5];
	uint32			shareBytesPerIPT_KB[2]; //分享了多少数据
	uint32			downBytesPerIPT_KB[3];  //iptype_tcp/iptype_udp/iptype_http
	uint32			downBytesPerUserT_KB[6];  //client,server,http,center,super
}PTL_P2T_ReportDownloadFileInfo;


typedef struct tagPTL_P2T_ReportStartDownloadList
{
	fhash_t  fhash;
	char url[1024]; //VIP启动时用于通知服务器加速
#ifdef SM_VOD
	int filetype; //share file type, 0 live, 1 vod
#endif
}PTL_P2T_ReportStartDownloadList;

typedef struct tagPTL_P2T_RequestStartDownloadList
{
	fhash_t  fhash;
	char url[1024];
}PTL_P2T_RequestStartDownloadList;


typedef struct tagPTL_P2T_RequestStopDownloadList
{
	fhash_t  fhash;
}PTL_P2T_RequestStopDownloadList;

//用于super分配VIP加速节目
typedef struct tagPTL_P2T_ReportDownloadListMaxnum
{
	uint32		downloadlist_maxnum;

}PTL_P2T_ReportDownloadListMaxnum;




//****************************
int operator << (SerialStream& ss, const PTL_P2T_FileInfo& inf);
int operator >> (SerialStream& ss, PTL_P2T_FileInfo& inf);
int operator << (SerialStream& ss, const PTL_P2T_PeerInfo& inf);
int operator >> (SerialStream& ss, PTL_P2T_PeerInfo& inf);
int operator << (SerialStream& ss, const PTL_P2T_ServerInfo& inf);
int operator >> (SerialStream& ss, PTL_P2T_ServerInfo& inf);

//****************************
int operator << (PTLStream& ss, const PTL_P2T_RequestLogin& inf);
int operator >> (PTLStream& ss, PTL_P2T_RequestLogin& inf);
int operator << (PTLStream& ss, const PTL_P2T_ResponseLogin& inf);
int operator >> (PTLStream& ss, PTL_P2T_ResponseLogin& inf);
//int operator << (PTLStream& ss, const PTL_P2T_ReportLogout& inf);
//int operator >> (PTLStream& ss, PTL_P2T_ReportLogout& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportNat& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportNat& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportShareFile& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportShareFile& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportRemoveFile& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportRemoveFile& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportStartDownloadFile& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportStartDownloadFile& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportStopDownloadFile& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportStopDownloadFile& inf);
int operator << (PTLStream& ss, const PTL_P2T_RequestFileSource& inf);
int operator >> (PTLStream& ss, PTL_P2T_RequestFileSource& inf);
int operator << (PTLStream& ss, const PTL_P2T_ResponseFileSource& inf);
int operator >> (PTLStream& ss, PTL_P2T_ResponseFileSource& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadWrong& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadWrong& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadFileSpeed& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadFileSpeed& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportStat& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportStat& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportError& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportError& inf);
int operator << (PTLStream& ss, const PTL_P2T_RequestConnTurn& inf);
int operator >> (PTLStream& ss, PTL_P2T_RequestConnTurn& inf);
int operator << (PTLStream& ss, const PTL_P2T_ResponseConnTurn& inf);
int operator >> (PTLStream& ss, PTL_P2T_ResponseConnTurn& inf);
int operator << (PTLStream& ss, const PTL_P2T_RequestServerList& inf);
int operator >> (PTLStream& ss, PTL_P2T_RequestServerList& inf);
int operator << (PTLStream& ss, const PTL_P2T_ResponseServerList& inf);
int operator >> (PTLStream& ss, PTL_P2T_ResponseServerList& inf);

int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadFileInfo& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadFileInfo& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportStartDownloadList& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportStartDownloadList& inf);
int operator << (PTLStream& ss, const PTL_P2T_RequestStartDownloadList& inf);
int operator >> (PTLStream& ss, PTL_P2T_RequestStartDownloadList& inf);
int operator << (PTLStream& ss, const PTL_P2T_RequestStopDownloadList& inf);
int operator >> (PTLStream& ss, PTL_P2T_RequestStopDownloadList& inf);
int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadListMaxnum& inf);
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadListMaxnum& inf);

//int operator << (PTLStream& ss, const & inf);
//int operator >> (PTLStream& ss, & inf);



