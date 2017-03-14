#pragma once

#include "commons.h"

#ifdef SM_VOD
#include "SynchroObj.h"
#endif

typedef struct tagUserInfo
{
	char            flag;                    //0 ��ʾû�б�ʹ�ã�  1��ʾ�Ѿ���ʹ��
	char            bshare;                  //�Ƿ���
	puid_t           uid;                     //�û�user id
	uint32          version;
	uint32          sysversion;              //����ϵͳ
	uint32          sessionID;
	uint32          ip;                      //���IPΪtracker��������peer����IP������Դmap�����ã��Ժ��ܱ䡣����tcpRealIP���棬tcpRealIP���ܻ��
	char            mac[6];
	uint32          totalDown_KB;               //�ܵ�������, KB(��λ)
	uint32          totalUpload_KB;             //�ܵ��ϴ�����KB(��λ)
	uint32          totalRequestNum;         //�����ļ��ĸ���
//	uint32          totalShareFileNum;            //�û��������ļ�����,sourceMap.size()
	char            dump;                    //�ϴ��Ƿ��쳣�˳�

	uint32			downloadlist_maxnum;	//��󲢷�ֱ������
	uint32			downloadlist_num;		//��ǰ�����playlist��
	time_t          beginTime;               //�û���¼ʱ��
	time_t          lastActiveTime;          //
	char            userType;             // 1��ʾ��ͨpeer,  2��ʾ������    3��ʾΪcenter�ַ�Դ
	char            natType;
	uint32          menu;
	uint32          tcpLocalIP;          //�ϱ�tcp����IP
	uint32          tcpRealIP;          //�ϱ�tcp����IP
	short           tcpLocalPort;        //�ϱ�tcp���ض˿�
	short           tcpRealPort;        //�ϱ�tcp���ض˿�
	uint32          udpLocalIP;          //�ϱ�udp����IP
	uint32          udpRealIP;          //�ϱ�udp����IP
	short           udpLocalPort;        //�ϱ�udp���ض˿�
	short           udpRealPort;        //�ϱ�udp���ض˿�
	map<hash_t,uint32> sourceMap;       //�����ļ��б�map<hash,urlflag>
#ifdef SM_VOD
	map<hash_t, map<hash_t, uint32> > sourceMapvod;
	Simple_Mutex sourceMapvod_mux;
#endif
	int				source_empty_count;  //sourceMap�ձ�������ǿ�ʱ����0������������һ��ʱ�䣬���ߡ�

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
		natType = 6;  //Ĭ��δ֪
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

