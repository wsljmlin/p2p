#pragma once
#include "uac_basetypes.h"
#include "uac_ntypes.h"
#include "uac_SerialStream.h"

namespace UAC
{
typedef struct tag_UDPS_ptl_TestSpeed_data
{
	sint32 sid; //һ�β��ٵ�id��ʶ
	sint32 size; //���Ͷ˱�ʾһ�������ٸ���
	sint32 seq; //������ţ���0��ʼ����

}UDPS_ptl_TestSpeed_data_t;

typedef struct tag_UDPS_ptl_TestSpeed_ack
{
	sint32 sid; //һ�β��ٵ�id��ʶ
	sint32 size; //�Ѿ��յ����ٸ���
	sint32 seq; //������ţ���0��ʼ����
	sint32 speedB; //ʵʱ�ٶ�

}UDPS_ptl_TestSpeed_ack_t;


int operator << (PTLStream& ps, const UDPS_ptl_TestSpeed_data_t& inf);
int operator >> (PTLStream& ps, UDPS_ptl_TestSpeed_data_t& inf);

int operator << (PTLStream& ps, const UDPS_ptl_TestSpeed_ack_t& inf);
int operator >> (PTLStream& ps, UDPS_ptl_TestSpeed_ack_t& inf);

class UDPTestSpeed
{
public:
	UDPTestSpeed(void){reset();}
	~UDPTestSpeed(void){}

	void reset()
	{
		recv_begin_utick = 0;
		recv_size = 0;
		recv_num = 0;
		
		my_begin_tick = 0;
		my_send_num = 0;
		my_send_recv_num = 0;
		my_speedB = 0;
	}

	void onrecv(const UDPS_ptl_TestSpeed_data_t& inf,int pksize)
	{
		if(0==recv_num)
		{
			recv_begin_utick = GetUTickCount();
			recv_size = 0;
		}
		else
		{
			recv_size += pksize;
		}
		recv_num++;
	}
	int get_speed()
	{
		return (int)(recv_size*1000000/(GetUTickCount()-recv_begin_utick));
	}

public:
	//ͳ�ƶԷ������ٶȵ���Ϣ
	uint64	recv_begin_utick;
	uint64	recv_size;
	int		recv_num;

	//���ٷ���Ϣ
	DWORD my_begin_tick;
	int my_send_num;
	int my_send_recv_num;
	int my_speedB;		
};

}
