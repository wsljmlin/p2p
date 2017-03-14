#pragma once
#include "uac.h"

namespace UAC
{

typedef struct tagUDPSConf
{
	unsigned int	packet_size;
	unsigned int	max_recv_win_num; //最多可接收包数
	unsigned char	max_resend_num;
	unsigned short	max_lose_rate;  //允许实时最大重发率（即低于此值才可能增窗）
	unsigned int	const_send_speedB; //如果非0，定速发送
	unsigned int	const_recv_speedB; //如果非0，定速发送
	unsigned short	const_recv_lose_rate; //如果非0，定丢包率发送，定速优先

	unsigned int	udp_keeplive_timer_msec; //默认5000毫秒
	unsigned int	udp_nak_timer_msec; //检测回应,15毫秒
	unsigned int	udp_conn_timeout_msec; //15000毫秒
	unsigned char	debug_msg_type;
	unsigned int	udp_testspeed_num; //测速包数

	//udp nattype
	int				nattype;
	//callback
	UAC_CALLBACK_ONNATOK callback_onnatok;
	UAC_CALLBACK_ONIPPORTCHANGED callback_onipportchanged;

	tagUDPSConf(void)
		:packet_size(0)  //此处赋值包含IP头,ip头20字节（无特殊选行）,UDP头8字节，UDPS头26字节，总大小不超1500字节,
		,max_recv_win_num(600) //1个pack约1K数据.
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

