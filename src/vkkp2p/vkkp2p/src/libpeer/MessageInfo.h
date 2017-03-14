#pragma once
#include "basetypes.h"
#include "ntypes.h"
#include "nhash.h"

typedef struct tagMsgDownloadlistInfo
{
	hash_t hash;
	unsigned int begintime;
	unsigned int seconds;  //一共运行了多久
	int downing_num; //当前正在下载片段数
	int p2psrc_num;
	int p2pconn_num;
	int httpconn_num;
	uint64 downSizeB;
	uint64 shareSizeB;
	int downSpeed_KB;
	int shareSpeed_KB;
	int file_fini_num;  //已下载片段数
	int file_all_num;	//到目前为止一共片段数
	list<int> fini_i_ls; //最近下载的100个号
	uint64 downSizeB_Con[3];
	uint64 downSizeB_User[6];

	
	tagMsgDownloadlistInfo(void)
		:begintime(0)
		,seconds(0)
		,downing_num(0)
		,p2psrc_num(0)
		,p2pconn_num(0)
		,httpconn_num(0)
		,downSizeB(0)
		,shareSizeB(0)
		,downSpeed_KB(0)
		,shareSpeed_KB(0)
		,file_fini_num(0)
		,file_all_num(0)
	{
		int i;
		for(i=0; i<3;i++ ) downSizeB_Con[i] = 0;
		for(i=0; i<6;i++ ) downSizeB_User[i] = 0;
	}
}MsgDownloadlistInfo_t;


typedef struct tagMsgDownloadlistInfo2
{
	string url;
	string name;
	string strhash;
	int downSpeed_KB;
	int shareSpeed_KB;
	unsigned int file_fini_num;  //一共下载片段数
	unsigned int max_i;		//记录当前最大片段号
	unsigned int token_i;   //记录当前下载到的片段号
	unsigned int list_num;  //记录m3u8的片段个数
	time_t	last_update_pl_time;
	tagMsgDownloadlistInfo2(void)
		:downSpeed_KB(0)
		,shareSpeed_KB(0)
		,file_fini_num(0)
		,max_i(0)
		,token_i(0)
		,list_num(0)
		,last_update_pl_time(0)
	{
	}
}MsgDownloadlistInfo2_t;

