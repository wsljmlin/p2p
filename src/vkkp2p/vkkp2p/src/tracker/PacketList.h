#pragma once
#include "MemBlock.h"

enum PACKET_CMD{ PACK_CMD_DATA,PACK_CMD_TCP_DISCONNECT };

typedef struct tagPacket
{
	char			flag;//0未占用，1占用
	short			cmd;
	int				sid; //session_id,用于channelmanager 与 peermanager 中的数组索引传递
	unsigned int    ip;
	short			port;
	MemBlock		*block;
	void			*data;//保存其它数据用
	tagPacket(void) : flag(0),cmd(0),sid(0),ip(0),port(0),block(0),data(0){}
}Packet;

//本链表要求读总是同一线程，写总是同一线程，但读写可以是不同线程。
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
