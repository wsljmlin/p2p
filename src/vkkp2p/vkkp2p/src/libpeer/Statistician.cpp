#include "Statistician.h"
#include "Timer.h"
#include "Setting.h"
#include "Util.h"

Statistician::Statistician(void)
: m_iDownloadCount(0)
, m_iShareCount(0)
{
}

Statistician::~Statistician(void)
{
}

int Statistician::init()
{
	TimerSngl::instance()->register_timer(this,1,1000);
	return 0;
}
int Statistician::fini()
{
	TimerSngl::instance()->unregister_all(this);
	return 0;
}
void Statistician::on_timer(int e)
{
	switch(e)
	{
	case 1:
		{
			m_recvSpeedometer.on_second();
			m_sendSpeedometer.on_second();
			if(m_iDownloadCount>0)
				m_stat.downSeconds++;
			if(m_iShareCount>0)
				m_stat.shareSeconds++;
		}
		break;
	default:
		assert(0);
		break;
	}
}

int Statistician::GetMaxRecvAllow()
{
	int n = SettingSngl::instance()->get_download_speed();
	if(n<=0)
		return -1;
	n -= (int)m_recvSpeedometer.get_last();
	if(n<0) n=0;
	return n;
}

int Statistician::GetMaxSendAllow()
{
	int n = SettingSngl::instance()->get_share_speed();
	if(n<=0)
		return -1;
	n -= (int)m_sendSpeedometer.get_last();
	if(n<0) n=0;
	return n;
}
void Statistician::OnDownloadStart()
{
	m_iDownloadCount++;
}
void Statistician::OnDownloadStop()
{
	assert(m_iDownloadCount>0);
	m_iDownloadCount--;
}
void Statistician::OnDownloadBytes(int size,int iptype,int utype)
{
	m_stat.downBytesPerIPT_B[iptype-IPT_TCP] += size;
	m_stat.downBytesPerUserT_B[utype-UT_CLIENT] += size;
	m_recvSpeedometer.add(size);
}

void Statistician::OnShareStart()
{
	m_iShareCount++;
}
void Statistician::OnShareStop()
{
	assert(m_iShareCount>0);
	m_iShareCount--;
}
void Statistician::OnShareBytes(int size,int iptype)
{
	m_stat.shareBytesPerIPT_B[iptype-IPT_TCP] += size;
	m_sendSpeedometer.add(size);
}

void Statistician::OnConnectionSpeed(unsigned int ip,int speedB,int seconds,int iptype,int utype,bool isDownload)
{	
	if(!Util::is_write_debug_log())
		return;

	//if(seconds<5)
	//	return;
	char buf[128];
	char path[256];
	SYSTEMTIME tm;
	memset(&tm,0,sizeof(SYSTEMTIME));
	GetLocalTime(&tm);
	sprintf(path,"%sspeed.%02d%02d%02d.log",SettingSngl::instance()->get_log_path().c_str(),tm.wYear,tm.wMonth,tm.wDay);
	sprintf(buf,"[%02d:%02d:%02d]:%d-(%s, %dKB, %dS, %d,%d) \r\n",tm.wHour,tm.wMinute,tm.wSecond
		,isDownload?0:1,Util::ip_htoas(ip).c_str(),(speedB+512)>>10,seconds,iptype,utype);

	FILE *fp = NULL;
	fp = fopen(path,"ab+");
	if(fp)
	{
		fwrite(buf,strlen(buf),1,fp);
		fclose(fp);
	}
}
void Statistician::OnConnection(int ctype,bool succeed)
{
	if(LOCAL_TCP == ctype) ctype = TCP_CONN;
	if(LOCAL_UDP == ctype) ctype = UDP_CONN;
	if(succeed)
		m_stat.connSucceedPerNetT[ctype-TCP_CONN] += 1;
	else
		m_stat.connFailedPerNetT[ctype-TCP_CONN] += 1;
}

