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
	int	nak_count;			//Ԥ�ƶ�ʧ����
	ULONGLONG last_send_tick; //���ڼ�¼��ʼ���Ͱ�ʱ�䣬���յ�ACKʱ������ͳ�Ƹð�һ��������ʱ����
	memblock *block;//��head
	tagUDPSSendPacket(void) : send_num(0),send_count(0),nak_count(0),last_send_tick(0),block(0) {}
}UDPSSendPacket_t;

typedef struct tagUDPSRecvPacket
{
	int recv_num;
	ULONGLONG recv_utick; //����ʱ��
	int ack_count; //�ظ�ACK����,����յ��ظ���,���ƻ���0
	memblock *block;//����head
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
	int win_low_line;//�����½磬�Է�δȷ�ϵ��½磬���Է���Ҫ����һ������
	unsigned int ack_sequence;
	unsigned int nak_num;
	int send_num;//�´η����°�ʹ�õ����к�
	int real_send_num;//��ʵ�Ѿ����͵���λ�ã������ϲ�෢���ݴ����
	unsigned int ttl; 
	unsigned int maxttl; //���ʱ�������TTL
	UDPSpeedCtrl2 spctrl;
	
	ArrRuler<unsigned int,10> ttls; //��·TTL
	unsigned int resend_num;//�ط��˶��ٸ���
	unsigned int resend_timeo_num;//�ط��˶��ٸ���
	unsigned int other_rerecv_num;//�Է��յ����ٸ��ظ���
	uint64			send_sizeB;		//һ�����˶�������
	unsigned int timer_bwwin_send_sizeB;
	unsigned int timer_bwwin_resend_sizeB;

	unsigned char not_more_data_count; //�޳������ӷ�������ʱ����ʱ�ط����ڱ��

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
	int					win_low_line;//�����½磬δȷ�ϵ��½�
	int					recv_win_num; //���Ǽ�����δӦ�ò���յ�����
	bool				brecv_win_num_changed; //��Իظ�ACK,���մ�����ʱΪtrue,�ظ����ΪFALSE
	unsigned int		max_recv_win_num; //�����Խ��ն��ٸ���
	int					max_sequence_num; //�յ����������
	int					rerecv_num;//�����˶��ٸ���

	//*************************************
	//ACK:
	UDPSAck_t			ack;	//
	unsigned int		ack_sequence; //
	unsigned int		last_ack_tick;
	unsigned int		unack_recv_num; //δ�ظ�ACKǰ�յ��İ���
	int					last_ack_win_low_line; //��Իظ�ACK,
	int					resend_ack_win_low_line_count; //�ظ��ظ�ͬһ��low_line����,���仯�������յ����µİ�ʱ��0
	////�ṹ����ACK�ظ�����ttl����û��ACK�ɻظ�ʱ����ظ�һ�����������ٻظ������������Ų�˳�����
	struct {
		ULONGLONG utick;
		unsigned int seq;
	}					last_pack;  
	//=====================================

	Speedometer<unsigned int>	speed;
	uint64						recv_sizeB;		//һ�����˶�������

	//*************************************
	//�ٶ�ͳ������,��ͬ��ŵİ�ͳ���ٶ�,�ٶȼ��㶪����һ����,���ݰ�����һ����
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
			//ע�� seq=0 ��Ч,����
			if(seq!=speed_seq)
			{
				//ͳ���ٶ�Ȼ���ʼ��
				unsigned int t = (unsigned int)((end_tick-begin_tick)/1000);
				//����һ�ν��ռ������2��,������һ�ε����ݲſ���Ҫ
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
				num = 1; //ͳ�ƽ�������ʱ����
				sizeB = 0; //��1���������ٶ�ͳ��
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

