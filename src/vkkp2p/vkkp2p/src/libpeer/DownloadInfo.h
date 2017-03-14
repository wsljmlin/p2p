#pragma once
#include "basetypes.h"
#include "ntypes.h"
#include "nhash.h"

typedef struct tagDownloadInfo
{
	hash_t hash;
	string tth;
	int  ftype;  //��������
	string path;
	uint64 size;
	uint64 reqOffset; //�����
	unsigned int block_offset; //������ʼ��
	unsigned int blocks;
	unsigned int down_blocks;
	uint64 bufferingSize;  //���������������ɵĴ�С�����Ϊ���ڴ�С��size��������ͣʱ�Ŀ�ʼ
	uint64 streamSize; //�������������������ɵĴ�С�����Ϊ���ڴ�С��size��������ͣʱ�Ŀ�ʼ
	int srcNum;
	int connNum;
	int httpConnNum;
	int speedB;
	int state; //0:ֹͣ,1:����,2:����(�Ŷ�)
	
	unsigned int begintime;
	unsigned int seconds;  //һ�������˶��

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
