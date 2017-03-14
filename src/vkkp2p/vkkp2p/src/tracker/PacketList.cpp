#include "PacketList.h"
#include <assert.h>

PacketList::PacketList(void)
: m_arr(0)
, m_size(0)
, m_gpos(0)
, m_ppos(0)
{
}
PacketList::~PacketList(void)
{
	assert(!m_arr);
}

int PacketList::resize(int size)
{
	if(m_arr)
	{
		delete[] m_arr;
		m_arr=0;
		m_size = 0;
		m_gpos = 0;
		m_ppos = 0;
	}
	if(size)
	{
		m_arr = new Packet[size+1];
		if(!m_arr)
			return -1;
		m_size = size+1;
		m_gpos = 0;
		m_ppos = 0;
	}
	return 0;
}
bool PacketList::put_packet(const Packet& pack)
{
	if(!m_arr)
		return false;
	//检查如果空位不够2，则保存失败，最多保存m_size-1个
	if(m_ppos<m_size-1)
	{
		if(m_arr[m_ppos+1].flag)
			return false;
	}
	else
	{
		if(m_arr[0].flag)
			return false;
	}
	assert(!m_arr[m_ppos].flag);
	m_arr[m_ppos].block = pack.block;
	m_arr[m_ppos].cmd   = pack.cmd;
	m_arr[m_ppos].sid   = pack.sid;
	m_arr[m_ppos].ip    = pack.ip;
	m_arr[m_ppos].port  = pack.port;
	m_arr[m_ppos].data  = pack.data;
	m_arr[m_ppos].flag  = 1;  //最后赋值安全
	
	m_ppos++;
	if(m_ppos>=m_size)
		m_ppos = 0;
	
	return true;
}
bool PacketList::get_packet(Packet& pack)
{
	if(!m_arr)
		return false;
	if(m_gpos==m_ppos)
		return false;
	assert(m_arr[m_gpos].flag);
	pack.block  = m_arr[m_gpos].block;
	pack.cmd    = m_arr[m_gpos].cmd;
	pack.sid    = m_arr[m_gpos].sid;
	pack.ip     = m_arr[m_gpos].ip;
	pack.port   = m_arr[m_gpos].port;
	pack.data   = m_arr[m_gpos].data;
	m_arr[m_gpos].flag = 0;

	m_gpos++;
	if(m_gpos>=m_size)
		m_gpos=0;
	return true;
}

