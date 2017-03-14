#pragma once
#include "MemBlock.h"

enum PACKET_CMD{ PACK_CMD_DATA,PACK_CMD_TCP_DISCONNECT };

typedef struct tagPacket
{
	char			flag;//0δռ�ã�1ռ��
	short			cmd;
	int				sid; //session_id,����channelmanager �� peermanager �е�������������
	unsigned int    ip;
	short			port;
	MemBlock		*block;
	void			*data;//��������������
	tagPacket(void) : flag(0),cmd(0),sid(0),ip(0),port(0),block(0),data(0){}
}Packet;

//������Ҫ�������ͬһ�̣߳�д����ͬһ�̣߳�����д�����ǲ�ͬ�̡߳�
class PacketList
{
public:
	PacketList(void);
	~PacketList(void);

	int resize(int size);
	bool put_packet(const Packet& inf);
	bool get_packet(Packet& inf);

	int get_blocking_count() const
	{
		return m_ppos>=m_gpos?m_ppos-m_gpos:m_size-m_gpos+m_ppos;
	}
private:
	Packet *m_arr;
	int m_size;
	int m_gpos,m_ppos;
};
