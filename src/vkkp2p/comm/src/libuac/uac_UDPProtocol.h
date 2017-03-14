#pragma once
#include "uac_basetypes.h"
#include "uac_SerialStream.h"
#include "uac_mempool.h"
#include "uac_UDPConfig.h"

//**********************************
//nat���Ϳ�����ͨ��ƥ��
//
//        nat1   nat2   nat3   nat4
// nat1    1      1      1      1
// nat2    1      1      1      1
// nat3    1      1      1      0
// nat4    1      1      0      0
//
//**********************************

//ע��: �����UDP���������Э��ǰ4λһ��������˰���־��ͬ

namespace UAC
{
//����������
enum UDPSAFE_PROTO_CONN_COMMAND{
	UDPS_CONN_HEAD_STX	= 0x20, //����־,ע�⣺0x80�Ѿ���stunʹ��
	UDPS_CONN_CMD_NAT    =(UDPS_CONN_HEAD_STX | 0x01),  //�򶴰�
	UDPS_CONN_CMD_CONN   =(UDPS_CONN_HEAD_STX | 0x02),  //���������Ӱ�
	UDPS_CONN_CMD_OK     =(UDPS_CONN_HEAD_STX | 0x03),  //���Ӿ�����
	UDPS_CONN_CMD_LIVE   =(UDPS_CONN_HEAD_STX | 0x04),  //ͨ�������
	UDPS_CONN_CMD_CLOSE  =(UDPS_CONN_HEAD_STX | 0x05),  //�Ͽ�ͨ����
	UDPS_CONN_CMD_DATA   =(UDPS_CONN_HEAD_STX | 0x06),		//ͨ�����ݰ�
	UDPS_CONN_CMD_TESTSPEED_DATA	=(UDPS_CONN_HEAD_STX | 0x07),	//�������ݰ�
	UDPS_CONN_CMD_TESTSPEED_ACK		=(UDPS_CONN_HEAD_STX | 0x08)	//���ٻ�Ӧ��
};

	
enum UDPSAFE_PROTO_CHANNEL_COMMAND1{

	UDPS_CHANNEL_CMD_DATA		=1,  //���ݰ�
	UDPS_CHANNEL_CMD_ACK		=2,  //ȷ�ϰ�
};

//UDPS �汾�ŷ���CONN����OK���е�sequence_num
#define UDPS_VERSION 1002

#define NEED_ACK 1
#define REPEAT_NAK 2
#define REPEAT_TIMEOUT 4
//ͷ�ṹ  9�ֽ�

#define UDPS_CONN_HEAD_LENGTH 9
typedef struct tagUDPSConnHeader
{
	uchar		cmd;                //��4λΪ����־����4λΪ������ֵ
	sint32		src_sessionid;      //Դ�ỰID
	sint32		des_sessionid;      //Ŀ�ĻỰID
	tagUDPSConnHeader(void):cmd(0),src_sessionid(0),des_sessionid(0){}
}UDPSConnHeader_t;


#define UDPHEAD_UPDATE_MASK(buf,mask) buf[14]=mask
#define UDPHEAD_UPDATE_MASK_SET_REPEAT_NAK(buf) buf[14]&=~REPEAT_TIMEOUT;buf[14]|=REPEAT_NAK
#define UDPHEAD_UPDATE_MASK_SET_REPEAT_TIMEOUT(buf) buf[14]&=~REPEAT_NAK;buf[14]|=REPEAT_TIMEOUT
#define UDPHEAD_UPDATE_SPEED_SEQ(buf,seq) buf[15]=seq

//����ͷ9�ֽ�,UDP���ݰ�����1���ֽ�
#define UDPHEAD_LENGTH 18
typedef struct tagUDPSData
{
	uint32		sequence_num;       //���к�
	uchar		mask;				//�Ӹ�����λ���ܣ� ��0|0|0|0|0|��ʱ�ط���|NAK�ظ���|������Ӧ��
	uchar		speed_seq;           //�ٶ�ͳ�Ʊ��,һ��ͳ������ʹ��ͬһ�����,0��Ч
	uint16		buflen;				//���ݳ���

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
	uint32		recv_win_num;	//ָʾ�Է���lowline_num�𻹿��Է��Ͷ��ٸ�block
	uint32		ack_sequence;		//�ۼƻظ����������Ͷ˿ɹ��ƶ�ack������
	uint32		rerecv_num;		//�յ��ظ��İ���,�ۼƺ�
	uchar		csp_speed_seq;		//�ٶ�����,һ���ٶ������ڿ��ܻᷢ�ض��ͬһ�����ڵ�recv_speedB��speed_i�����ڱ�ʾ���ڵı仯���ý��ն˱�֤���ظ�����ͬ�ٶ����ڵ��ٶ�
	uint16		csp_num;
	uint32		csp_speedB;
	uint16		const_send_speedKB;		//ָ���Է�������ٶȷ���
	uint16		const_send_lose_rate;  //ָ���Է�����������ʷ���
	uchar		size;
	uint64		recv_utick[ACK_ARR_LEN];		//��¼���յ�����ʱ��ʱ�䣬�����ݲ���Э�鴫�䣬ֻ�ǵ����ڲ�����ʹ��
	uint32		seq_nums[ACK_ARR_LEN];		//���һ�λظ�ACK_ARR_LEN��ȷ�ϰ�,Լ�����һ����Ϊ������Ӧ��
	uint32		wait_us[ACK_ARR_LEN];  //�յ�ȷ�ϰ�����˶���΢��Żظ�ACK
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
	//ֻ�������ṩ��
	int get_des_sessionid(){ return m_des_sessionid;}
	int get_src_sessionid(){ return m_src_sessionid;}
protected:
	int __idx;  //��UDPConnector�е�����
	SOCKET m_fd;
	sockaddr_in m_addr;
	int m_des_sessionid,m_src_sessionid;
	unsigned int m_mtu;//ȡ˫����Сֵ
};

class UDPConnector;
class UDPChannelFactory
{
public:
	virtual ~UDPChannelFactory(void){}
public:
	//����һ���µ�UDPChannel ������UDPChannelHandlerָ�룬��IP��PORT����UDPChannel,UDPChannel����Ҫ��ע��
	virtual UDPChannelHandler* attach_udp_channel(SOCKET fd,sockaddr_in& addr,UDPConnector* ctr)=0;
};

}





