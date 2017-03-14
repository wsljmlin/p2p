#pragma once
#include "uac_basetypes.h"
#include "uac_SerialStream.h"
#include "uac_mempool.h"
#include "uac_UDPConfig.h"

//**********************************
//nat类型可连接通性匹配
//
//        nat1   nat2   nat3   nat4
// nat1    1      1      1      1
// nat2    1      1      1      1
// nat3    1      1      1      0
// nat4    1      1      0      0
//
//**********************************

//注意: 如果是UDP其它命令辅助协议前4位一定不能与此包标志相同

namespace UAC
{
//包命令类型
enum UDPSAFE_PROTO_CONN_COMMAND{
	UDPS_CONN_HEAD_STX	= 0x20, //包标志,注意：0x80已经被stun使用
	UDPS_CONN_CMD_NAT    =(UDPS_CONN_HEAD_STX | 0x01),  //打洞包
	UDPS_CONN_CMD_CONN   =(UDPS_CONN_HEAD_STX | 0x02),  //请求新连接包
	UDPS_CONN_CMD_OK     =(UDPS_CONN_HEAD_STX | 0x03),  //连接就绪包
	UDPS_CONN_CMD_LIVE   =(UDPS_CONN_HEAD_STX | 0x04),  //通道保活包
	UDPS_CONN_CMD_CLOSE  =(UDPS_CONN_HEAD_STX | 0x05),  //断开通道包
	UDPS_CONN_CMD_DATA   =(UDPS_CONN_HEAD_STX | 0x06),		//通道数据包
	UDPS_CONN_CMD_TESTSPEED_DATA	=(UDPS_CONN_HEAD_STX | 0x07),	//测速数据包
	UDPS_CONN_CMD_TESTSPEED_ACK		=(UDPS_CONN_HEAD_STX | 0x08)	//测速回应包
};

	
enum UDPSAFE_PROTO_CHANNEL_COMMAND1{

	UDPS_CHANNEL_CMD_DATA		=1,  //数据包
	UDPS_CHANNEL_CMD_ACK		=2,  //确认包
};

//UDPS 版本号放在CONN包和OK包中的sequence_num
#define UDPS_VERSION 1002

#define NEED_ACK 1
#define REPEAT_NAK 2
#define REPEAT_TIMEOUT 4
//头结构  9字节

#define UDPS_CONN_HEAD_LENGTH 9
typedef struct tagUDPSConnHeader
{
	uchar		cmd;                //高4位为包标志，低4位为包命令值
	sint32		src_sessionid;      //源会话ID
	sint32		des_sessionid;      //目的会话ID
	tagUDPSConnHeader(void):cmd(0),src_sessionid(0),des_sessionid(0){}
}UDPSConnHeader_t;


#define UDPHEAD_UPDATE_MASK(buf,mask) buf[14]=mask
#define UDPHEAD_UPDATE_MASK_SET_REPEAT_NAK(buf) buf[14]&=~REPEAT_TIMEOUT;buf[14]|=REPEAT_NAK
#define UDPHEAD_UPDATE_MASK_SET_REPEAT_TIMEOUT(buf) buf[14]&=~REPEAT_NAK;buf[14]|=REPEAT_TIMEOUT
#define UDPHEAD_UPDATE_SPEED_SEQ(buf,seq) buf[15]=seq

//连接头9字节,UDP数据包命令1个字节
#define UDPHEAD_LENGTH 18
typedef struct tagUDPSData
{
	uint32		sequence_num;       //序列号
	uchar		mask;				//从高至低位功能： “0|0|0|0|0|超时重发包|NAK重复包|立即回应”
	uchar		speed_seq;           //速度统计编号,一个统计周期使用同一个编号,0无效
	uint16		buflen;				//数据长度

	tagUDPSData(void)
		:sequence_num(0)
		,mask(0)
		,speed_seq(0)
		,buflen(0)
	{}
}UDPSData_t;

#define ACK_ARR_LEN 100

typedef struct tagUDPSAck
{
	uint32		lowline_num;	//lowline_num
	uint32		recv_win_num;	//指示对方从lowline_num起还可以发送多少个block
	uint32		ack_sequence;		//累计回复次数，发送端可估计丢ack包总数
	uint32		rerecv_num;		//收到重复的包数,累计和
	uchar		csp_speed_seq;		//速度周期,一个速度周期内可能会发关多次同一个周期的recv_speedB，speed_i仅用于表示周期的变化，让接收端保证不重复利用同速度周期的速度
	uint16		csp_num;
	uint32		csp_speedB;
	uint16		const_send_speedKB;		//指定对方以这个速度发送
	uint16		const_send_lose_rate;  //指定对方以这个丢包率发送
	uchar		size;
	uint64		recv_utick[ACK_ARR_LEN];		//记录接收到数据时的时间，此数据不用协议传输，只是单方内部程序使用
	uint32		seq_nums[ACK_ARR_LEN];		//最多一次回复ACK_ARR_LEN个确认包,约定最后一个包为立即回应包
	uint32		wait_us[ACK_ARR_LEN];  //收到确认包后等了多少微秒才回复ACK
	tagUDPSAck(void)
		:lowline_num(0)
		,recv_win_num(0)
		,ack_sequence(0)
		,rerecv_num(0)
		,csp_speed_seq(0)
		,csp_num(0)
		,csp_speedB(0)
		,const_send_speedKB(0)
		,const_send_lose_rate(0)
		,size(0){}
}UDPSAck_t;


int operator << (PTLStream& ps, const UDPSConnHeader_t& inf);
int operator >> (PTLStream& ps, UDPSConnHeader_t& inf);

int operator << (PTLStream& ps, const UDPSData_t& inf);
int operator >> (PTLStream& ps, UDPSData_t& inf);

int operator << (PTLStream& ps, const UDPSAck_t& inf);
int operator >> (PTLStream& ps, UDPSAck_t& inf);


class UDPChannelHandler
{
	friend class UDPConnector;
public:
	int _test_speedB;
	UDPChannelHandler(int idx) :_test_speedB(0), __idx(idx),m_fd(INVALID_SOCKET),m_mtu(0){}
	virtual ~UDPChannelHandler(void){}
	
public:
	virtual int handle_connected(){return 0;};
	virtual int handle_disconnected(){return 0;};
	virtual int handle_send(bool roolcall=false){return 0;};
	virtual int handle_recv(memblock* b)=0;
	virtual int send_to(char *buf,int len,ULONGLONG utick){ return ::sendto(m_fd,buf,len,0,(sockaddr*)&m_addr,sizeof(m_addr));}
	int idx() const {return __idx;}
	unsigned int mtu() const {return m_mtu;}
protected:
	//只对子类提供读
	int get_des_sessionid(){ return m_des_sessionid;}
	int get_src_sessionid(){ return m_src_sessionid;}
protected:
	int __idx;  //在UDPConnector中的索引
	SOCKET m_fd;
	sockaddr_in m_addr;
	int m_des_sessionid,m_src_sessionid;
	unsigned int m_mtu;//取双方最小值
};

class UDPConnector;
class UDPChannelFactory
{
public:
	virtual ~UDPChannelFactory(void){}
public:
	//创建一个新的UDPChannel 并返回UDPChannelHandler指针，把IP，PORT赋给UDPChannel,UDPChannel不需要再注册
	virtual UDPChannelHandler* attach_udp_channel(SOCKET fd,sockaddr_in& addr,UDPConnector* ctr)=0;
};

}





