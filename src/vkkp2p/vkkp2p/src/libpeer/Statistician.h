#pragma once
#include "commons.h"
#include "Handler.h"
#include "Speedometer.h"

typedef struct tagP2PStat
{
	uint32 connSucceedPerNetT[5]; //下标:0，直接TCP连接；1，callback TCP；2，UDP直接连接；3，UDP callback；4，UDP nat hole 连接
	uint32 connFailedPerNetT[5];  
	uint64 downBytesPerIPT_B[3];   //下标:0-TCP; 1- UDP ;2-HTTP
	uint64 shareBytesPerIPT_B[3];   //下标:0-TCP; 1- UDP
	uint64 downBytesPerUserT_B[6];   //0:UT_CLIENT;1:UT_SERVER;
	uint32 downSeconds;
	uint32 shareSeconds;          //共享时长
	tagP2PStat()
	{
		int i=0;
		for(i=0;i<5;++i)
			connSucceedPerNetT[i] = connFailedPerNetT[i] = 0;
		for(i=0;i<3;++i)
			downBytesPerIPT_B[i] = shareBytesPerIPT_B[i] = 0;
		for(i=0;i<6;++i)
			downBytesPerUserT_B[i] = 0;
		downSeconds = shareSeconds = 0;
	}
	tagP2PStat& operator=(const tagP2PStat& inf)
	{
		if(&inf == this)
			return *this;
		int i=0;
		for(i=0;i<5;++i)
		{
			connSucceedPerNetT[i] = inf.connSucceedPerNetT[i];
			connFailedPerNetT[i] = inf.connFailedPerNetT[i];
		}
		for(i=0;i<3;++i)
		{
			downBytesPerIPT_B[i] = inf.downBytesPerIPT_B[i];
			shareBytesPerIPT_B[i] = inf.shareBytesPerIPT_B[i];
		}
		for(i=0;i<6;++i)
		{
			downBytesPerUserT_B[i] = inf.downBytesPerUserT_B[i];
		}
		downSeconds = inf.downSeconds;
		shareSeconds = inf.shareSeconds;
		return *this;
	}
	tagP2PStat operator-(const tagP2PStat& s)
	{
		tagP2PStat st;
		int i=0;
		for(i=0;i<5;++i)
		{
			st.connSucceedPerNetT[i] = connSucceedPerNetT[i] - s.connSucceedPerNetT[i];
			st.connFailedPerNetT[i] = connFailedPerNetT[i] - s.connFailedPerNetT[i];
		}
		for(i=0;i<3;++i)
		{
			st.downBytesPerIPT_B[i] = downBytesPerIPT_B[i] - s.downBytesPerIPT_B[i];
			st.shareBytesPerIPT_B[i] = shareBytesPerIPT_B[i] - s.shareBytesPerIPT_B[i];
		}
		for(i=0;i<6;++i)
		{
			st.downBytesPerUserT_B[i] = downBytesPerUserT_B[i] - s.downBytesPerUserT_B[i];
		}
		return st;
		downSeconds = downSeconds - s.downSeconds;
		shareSeconds = shareSeconds - s.shareSeconds;
	}
}P2PStat;

class Statistician : public TimerHandler
{
	friend class Singleton<Statistician>;
private:
	Statistician(void);
	~Statistician(void);
public:
	int init();
	int fini();
	virtual void on_timer(int e);

	int GetMaxRecvAllow();
	int GetMaxSendAllow();

	void OnDownloadStart();
	void OnDownloadStop();
	void OnDownloadBytes(int size,int iptype,int utype);//iptype：tcp/udp

	void OnShareStart();
	void OnShareStop();
	void OnShareBytes(int size,int iptype); //iptype：tcp/udp

	void OnConnectionSpeed(unsigned int ip,int speedB,int seconds,int iptype,int utype,bool isDownload);//iptype：tcp/udp
	void OnConnection(int ctype,bool succeed); //ctype：直连/反连/穿透


	int GetLastStat(P2PStat& st){
		st = m_stat - m_lastStat;
		m_lastStat = m_stat;
		return 0;
	}

public:
	Speedometer<uint64> m_recvSpeedometer;
	Speedometer<uint64> m_sendSpeedometer;
private:
	P2PStat     m_stat;
	P2PStat     m_lastStat;
	int         m_iDownloadCount;
	int         m_iShareCount;
}; 
typedef Singleton<Statistician> StatisticianSngl;
