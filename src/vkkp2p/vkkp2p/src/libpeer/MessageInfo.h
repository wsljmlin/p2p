#pragma once
#include "basetypes.h"
#include "ntypes.h"
#include "nhash.h"

typedef struct tagMsgDownloadlistInfo
{
	hash_t hash;
	unsigned int begintime;
	unsigned int seconds;  //һ�������˶��
	int downing_num; //��ǰ��������Ƭ����
	int p2psrc_num;
	int p2pconn_num;
	int httpconn_num;
	uint64 downSizeB;
	uint64 shareSizeB;
	int downSpeed_KB;
	int shareSpeed_KB;
	int file_fini_num;  //������Ƭ����
	int file_all_num;	//��ĿǰΪֹһ��Ƭ����
	list<int> fini_i_ls; //������ص�100����
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
	unsigned int file_fini_num;  //һ������Ƭ����
	unsigned int max_i;		//��¼��ǰ���Ƭ�κ�
	unsigned int token_i;   //��¼��ǰ���ص���Ƭ�κ�
	unsigned int list_num;  //��¼m3u8��Ƭ�θ���
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

