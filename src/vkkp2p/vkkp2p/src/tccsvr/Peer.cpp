#include "Peer.h"
#include "Protocol.h"

#define PEER_PACKET_SIZE 1024
Peer::Peer(void)
:Speaker<PeerListener>(1)
,m_block(NULL)
{
	m_ch.add_listener(this);
}

Peer::~Peer(void)
{
	assert(DISCONNECTED==m_ch.get_state());
	m_ch.remove_listener(this);
}

int Peer::attach(SOCKET s,sockaddr_in& addr)
{
	return m_ch.attach(s,addr);
}
int Peer::connect(unsigned int ip,unsigned short port)
{
	return m_ch.connect(ip,port);
}
int Peer::disconnect()
{
	if(DISCONNECTED==m_ch.get_state())
	{
		//未连接时也要回调，上层在Disconnected()时才删除
		fire(PeerListener::Disconnected(),this);
		return 0;
	}
	return m_ch.disconnect();
}
int Peer::send(MemBlock *b)
{
	return m_ch.send(b);
}
void Peer::on(Connected,Channel* ch)
{
	//DEBUGMSG("# peer::on(Connected) \n");
	fire(PeerListener::Connected(),this);
}
void Peer::on(Disconnected,Channel* ch)
{
	//DEBUGMSG("# peer::on(Disconnected) \n");
	if(m_block)
	{
		m_block->free(1);
		m_block = NULL;
	}
	fire(PeerListener::Disconnected(),this);
}
void Peer::on(Readable,Channel* ch,const int& wait)
{
	while(CONNECTED==m_ch.get_state())
	{
		if(!m_block)
		{
			m_block = MemBlock::allot(PEER_PACKET_SIZE,1);
			if(!m_block)
			{
				disconnect();
				return;
			}
			assert(0==m_block->datapos);
			m_block->position(0);
			m_block->limit(8);
		}
		m_tmpsize = m_ch.recv(m_block->pointer(),m_block->remaining());
		if(m_tmpsize>0)
		{
			m_block->increase(m_tmpsize);
			if(0==m_block->remaining())
				on_data();

		}
		else
			return;
	}
}
void Peer::on_data()
{
	if(8==m_block->limit())
	{
		uint32 size = SerialStream::ltoh32(*((uint32*)(m_block->buf+4)));
		if(PTL_HEAD_STX_32!= *(uint32*)m_block->buf || size<=8 || size>PEER_PACKET_SIZE)
		{
			DEBUGMSG("#****wrong packet!****\n");
			disconnect(); //里面会删除m_block
			return;
		}
		m_block->limit(size);

	}else
	{
		m_block->position(0);
		MemBlock* b = m_block;
		m_block = NULL;
		fire(PeerListener::Data(),this,b); //可能fire过程中disconnect()导致m_block->free()重复
	}
}


