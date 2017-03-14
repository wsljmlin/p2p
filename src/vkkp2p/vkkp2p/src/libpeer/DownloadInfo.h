#pragma once
#include "basetypes.h"
#include "ntypes.h"
#include "nhash.h"

typedef struct tagDownloadInfo
{
	hash_t hash;
	string tth;
	int  ftype;  //下载类型
	string path;
	uint64 size;
	uint64 reqOffset; //请求点
	unsigned int block_offset; //下载起始块
	unsigned int blocks;
	unsigned int down_blocks;
	uint64 bufferingSize;  //从请求点起，下载完成的大小，最大为窗口大小的size，用于暂停时的开始
	uint64 streamSize; //从请求点起，连续下载完成的大小，最大为窗口大小的size，用于暂停时的开始
	int srcNum;
	int connNum;
	int httpConnNum;
	int speedB;
	int state; //0:停止,1:下载,2:就绪(排队)
	
	unsigned int begintime;
	unsigned int seconds;  //一共运行了多久

	tagDownloadInfo(void)
		:ftype(0)
		,size(0)
		,reqOffset(0)
		,block_offset(0)
		,blocks(0)
		,down_blocks(0)
		,bufferingSize(0)
		,streamSize(0)
		,srcNum(0)
		,connNum(0)
		,httpConnNum(0)
		,speedB(0)
		,state(0)
		,begintime(0)
		,seconds(0)
	{}
}DownloadInfo;
