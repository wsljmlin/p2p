#pragma once
#include "uac_Channel.h"
#include "uac_Timer.h"
#include "uac_UDPConnector.h"
#include "uac_Speedometer.h"
#include "uac_UDPSpeedCtrl2.h"
#include "uac_cyclist.h"
#include "uac_rbtmap.h"

namespace UAC
{

typedef struct tagUDPSSendPacket
{
	int send_num;
	int send_count;
	int	nak_count;			//预计丢失计数
	ULONGLONG last_send_tick; //用于记录开始发送包时间，当收到ACK时，可以统计该包一个来回用时多少
	memblock *block;//带head
	tagUDPSSendPacket(void) : send_num(0),send_count(0),nak_count(0),last_send_tick(0),block(0) {}
}UDPSSendPacket_t;

typedef struct tagUDPSRecvPacket
{
	int recv_num;
	ULONGLONG recv_utick; //接收时间
	int ack_count; //回复ACK次数,如果收到重复包,将计划置0
	memblock *block;//不带head
	tagUDPSRecvPacket(void) : recv_num(0),recv_utick(0),ack_count(0),block(0) {}
}UDPSRecvPacket_t;

typedef struct tagResendRate
{
	unsigned int valid_send_size;
	unsigned int resend_size;
	tagResendRate(void) : valid_send_size(0),resend_size(0){}
	void reset(){valid_send_size=0;resend_size=0;}
	unsigned int rate() 
	{
		if(valid_send_size)
			return resend_size*100/valid_send_size;
		if(resend_size)
			return 100;
		return 0;
	}
}ResendRate_t;


typedef struct tagUDPSSendWin
{
	int win_low_line;//窗口下界，对方未确认的下界，即对方想要的下一个数据
	unsigned int ack_sequence;
	unsigned int nak_num;
	int send_num;//下次发送新包使用的序列号
	int real_send_num;//真实已经发送到的位置，可能上层多发，暂存队列
	unsigned int ttl; 
	unsigned int maxttl; //最近时间内最大TTL
	UDPSpeedCtrl2 spctrl;
	
	ArrRuler<unsigned int,10> ttls; //回路TTL
	unsigned int resend_num;//重发了多少个包
	unsigned int resend_timeo_num;//重发了多少个包
	unsigned int other_rerecv_num;//对方收到多少个重复包
	uint64			send_sizeB;		//一共发了多少数据
	unsigned int timer_bwwin_send_sizeB;
	unsigned int timer_bwwin_resend_sizeB;

	unsigned char not_more_data_count; //无持续增加发送数据时，超时重发周期变短

	void reset()
	{
		win_low_line=0;
		ack_sequence=0;
		nak_num=0;
		send_num=0;
		real_send_num=-1;
		ttl = 100000; //
		maxttl = 0;
		resend_num=0;
		resend_timeo_num=0;
		other_rerecv_num=0;
		send_sizeB=0;
		timer_bwwin_send_sizeB=0;
		timer_bwwin_resend_sizeB=0;
		not_more_data_count = 0;
	}
	tagUDPSSendWin(void){ reset();}
}UDPSSendWin_t;

typedef struct tagUDPSRecvWin
{
	//recv win
	int					win_low_line;//窗口下界，未确认的下界
	int					recv_win_num; //考虑减掉被未应用层接收的数量
	bool				brecv_win_num_changed; //相对回复ACK,接收窗更新时为true,回复后变为FALSE
	unsigned int		max_recv_win_num; //最多可以接收多少个包
	int					max_sequence_num; //收到的最大包序号
	int					rerecv_num;//重收了多少个包

	//*************************************
	//ACK:
	UDPSAck_t			ack;	//
	unsigned int		ack_sequence; //
	unsigned int		last_ack_tick;
	unsigned int		unack_recv_num; //未回复ACK前收到的包数
	int					last_ack_win_low_line; //相对回复ACK,
	int					resend_ack_win_low_line_count; //重复回复同一个low_line计数,当变化或者有收到更新的包时置0
	////结构用于ACK回复计算ttl，当没有ACK可回复时，则回复一个，有则不能再回复这个，否则序号不顺序递增
	struct {
		ULONGLONG utick;
		unsigned int seq;
	}					last_pack;  
	//=====================================

	Speedometer<unsigned int>	speed;
	uint64						recv_sizeB;		//一共收了多少数据

	//*************************************
	//速度统计周期,相同编号的包统计速度,速度计算丢掉第一个包,数据包含第一个包
	struct tagCycleSpeed{
		unsigned char	speed_seq;
		ULONGLONG		begin_tick;
		ULONGLONG		end_tick;
		unsigned short	num;
		unsigned int	sizeB;
		unsigned char	last_speed_seq;
		unsigned short	last_num;
		unsigned int	last_speedB;

		tagCycleSpeed(void)
			:speed_seq(0)
			,begin_tick(0)
			,end_tick(0)
			,num(0)
			,sizeB(0)
			,last_speed_seq(0)
			,last_num(0)
			,last_speedB(0)
		{}
		void on_recv(ULONGLONG utick,unsigned char seq,unsigned int size)
		{
			//注意 seq=0 无效,空闲
			if(seq!=speed_seq)
			{
				//统计速度然后初始化
				unsigned int t = (unsigned int)((end_tick-begin_tick)/1000);
				//离上一次接收间隔不到2秒,这样上一次的数据才考虑要
				if(speed_seq>0 && sizeB>0 && t>1 && end_tick+2000000>utick)
				{
					last_speed_seq = speed_seq;
					last_num = num;
					last_speedB = sizeB*1000/t;
				}
				else
				{
					last_speed_seq = 0;
					last_num = 0;
					last_speedB = 0;
				}
				begin_tick = end_tick = utick;
				speed_seq = seq;
				num = 1; //统计接收数据时包含
				sizeB = 0; //第1个包不作速度统计
			}
			else if(0!=seq)
			{
				end_tick = utick;
				num++;
				sizeB += size;
			}
		}
	}	csp; //cycle speed
	//=====================================

	void reset() 
	{ 
		win_low_line=0;
		last_ack_win_low_line=-1;
		resend_ack_win_low_line_count=0;
		brecv_win_num_changed = true;
		max_recv_win_num = g_udps_conf.max_recv_win_num;
		recv_win_num = max_recv_win_num;
		last_ack_tick = 0;
		ack_sequence=0;
		max_sequence_num = -1;
		rerecv_num=0;
		ack.size = 0;
		unack_recv_num=0;
		recv_sizeB = 0;
		last_pack.utick = 0;
		last_pack.seq = 0;
	}
	tagUDPSRecvWin(void) {reset();}
}UDPSRecvWin_t;


//***********************************************************************************

class UDPChannel : public Channel,public UDPChannelHandler
	,public TimerHandler
{

public:
	UDPChannel(UDPConnector* ctr,int idx);
	virtual ~UDPChannel(void);
	typedef list<UDPSSendPacket_t> SendList;
	typedef SendList::iterator SendIter;
	typedef map<int,UDPSRecvPacket_t> RecvMap;
	typedef RecvMap::iterator RecvIter;
public:
	//Channel
	virtual int attach(SOCKET s,sockaddr_in& addr);
	virtual int connect(const char* ip,unsigned short port,int nattype=0);
	virtual int connect(unsigned int ip,unsigned short port,int nattype=0);
	virtual int disconnect();
	virtual int send(memblock *b,bool more=false);  //-1:false; 0:send ok; 1:put int sendlist
	virtual int recv(char *b,int size){assert(false);return 0;}
	void update_recv_win_num(uint32 cache_block_num);

	//UDPChannelHandler
	virtual int handle_connected();
	virtual int handle_disconnected();
	virtual int send_to(char *buf,int len,ULONGLONG utick);
	virtual int handle_send(bool roolcall=false);
	virtual int handle_recv(memblock* b);

	UDPChannelHandler* get_udpchannel_handler();

	//TimerHandler
	virtual void on_timer(int e);
private:
	virtual void reset();
	void on_connected();
	void on_disconnected();
	void on_data(memblock* b);
	void on_writable();

	int handle_recv_data(PTLStream& ps,memblock* b);
	int handle_recv_ack(PTLStream& ps);
	void send_ack(DWORD tick,ULONGLONG utick);
	void handle_ack_timer();
	void handle_bwwin_timer();

private:
	UDPConnector* m_ctr;
	//int m_smore; //recode last fire writable if call send();
	UDPSSendWin_t m_send_win;
	UDPSRecvWin_t m_recv_win;
	SendList m_send_list;
	RecvMap m_recv_map;
	bool m_bsend_timer;
	unsigned int m_send_list_max_size;
	unsigned int m_begin_tick;
	unsigned char m_des_nattype;

	char *_tmp_cmdbuf;
	PTLStream _tmp_ps,_tmp_recv_ps;
	UDPSConnHeader_t _tmp_head;
	uchar		_tmp_cmd,_tmp_recv_cmd;
	UDPSData_t _tmp_send_data,_tmp_recv_data;
	UDPSAck_t _tmp_ack;
};

}

