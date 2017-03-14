#pragma once
#include "uac.h"

namespace UAC
{

typedef struct tagUDPSConf
{
	unsigned int	packet_size;
	unsigned int	max_recv_win_num; //���ɽ��հ���
	unsigned char	max_resend_num;
	unsigned short	max_lose_rate;  //����ʵʱ����ط��ʣ������ڴ�ֵ�ſ���������
	unsigned int	const_send_speedB; //�����0�����ٷ���
	unsigned int	const_recv_speedB; //�����0�����ٷ���
	unsigned short	const_recv_lose_rate; //�����0���������ʷ��ͣ���������

	unsigned int	udp_keeplive_timer_msec; //Ĭ��5000����
	unsigned int	udp_nak_timer_msec; //����Ӧ,15����
	unsigned int	udp_conn_timeout_msec; //15000����
	unsigned char	debug_msg_type;
	unsigned int	udp_testspeed_num; //���ٰ���

	//udp nattype
	int				nattype;
	//callback
	UAC_CALLBACK_ONNATOK callback_onnatok;
	UAC_CALLBACK_ONIPPORTCHANGED callback_onipportchanged;

	tagUDPSConf(void)
		:packet_size(0)  //�˴���ֵ����IPͷ,ipͷ20�ֽڣ�������ѡ�У�,UDPͷ8�ֽڣ�UDPSͷ26�ֽڣ��ܴ�С����1500�ֽ�,
		,max_recv_win_num(600) //1��packԼ1K����.
		,max_resend_num(40)
		,max_lose_rate(20)
		,const_send_speedB(0)
		,const_recv_speedB(0)
		,const_recv_lose_rate(0)

		,udp_keeplive_timer_msec(5000)
		,udp_nak_timer_msec(20)
		,udp_conn_timeout_msec(12000)
		,debug_msg_type(0)
		,udp_testspeed_num(10)

		,nattype(6)
		,callback_onnatok(0)
		,callback_onipportchanged(0)
	{}

}UDPSConf;
extern UDPSConf g_udps_conf;

}

