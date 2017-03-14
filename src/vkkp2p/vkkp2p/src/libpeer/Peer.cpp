#include "Peer.h"
#include "TCPFWChannel.h"
#include "UACChannel.h"

Peer::Peer(char ip_type,char conn_type,int index)
:Speaker<PeerListener>(1)
,m_last_active_tick(0)
{
	if(IPT_TCP==ip_type)
	{
		m_ch = new TCPFWChannel();
	}
	else if(IPT_UDP==ip_type)
	{
		m_ch = new UACChannel();
	}
	else if(IPT_HTTP==ip_type)
	{
		m_ch = new TCPChannel();
	}
	else
	{
		assert(0);
	}
	m_ip_type = ip_type;
	m_conn_type = conn_type;
	m_ch->add_listener(this);
	__i = index;
	m_block = MemBlock::allot(63<<10);
	m_block->limit(8);
	DEBUGMSG("#:new peer() \n");
	m_pPeerReadLimitSinker = NULL;
	m_conn_success = false;
}

Peer::~Peer(void)
{
	m_ch->remove_listener(this);
	delete m_ch;
	if(m_block)
	{
		m_block->free();
		m_block = NULL;
	}
	DEBUGMSG("#:delete peer() \n");
}

int Peer::attach(SOCKET s,sockaddr_in& addr)
{
	return m_ch->attach(s,addr);
}

int Peer::connect(const char* ip,unsigned short port,int nattype)
{
	return m_ch->connect(ip,port,NULL,nattype);
}

int Peer::connect(unsigned int ip,unsigned short port,int nattype)
{
	return m_ch->connect(ip,port,NULL,nattype);
}

int Peer::disconnect()
{
	if(DISCONNECTED==m_ch->get_state())
	{
		//未连接时也要回调，上层在Disconnected()时才删除
		fire(PeerListener::Disconnected(),this);
		return 0;
	}
	return m_ch->disconnect();
}

int Peer::send(MemBlock *b)
{
	return m_ch->send(b);
}

void Peer::on(Connecting,Channel* ch)
{
	fire(PeerListener::Connecting(),this);
}

void Peer::on(Connected,Channel* ch)
{
	m_conn_success = true;
	fire(PeerListener::Connected(),this);
}

void Peer::on(Disconnected,Channel* ch)
{
	m_block->position(0);
	m_block->limit(8);
	fire(PeerListener::Disconnected(),this);
	m_conn_success = false;
}

void Peer::on(Data,Channel* ch,char* buf,int size)
{
	assert(0);
	//udp data
	int cpsize = 0;
	while(size>0 && CONNECTED==m_ch->get_state())
	{
		cpsize = m_block->remaining();
		if(cpsize>size) cpsize = size;
		memcpy(m_block->pointer(),buf,cpsize);
		size -= cpsize;
		buf += cpsize;
		m_block->increase(cpsize);
		if(0==m_block->remaining())
			on_data();
	}
}

void Peer::on(Readable,Channel* ch,const int& wait)
{
	//tcp recv
	int ret = 0;
	int& wait1 = (int&)wait;
	wait1 = 0;
	while(CONNECTED==m_ch->get_state())
	{
		ret = m_block->remaining();
		if(m_pPeerReadLimitSinker)
		{
			int n = m_pPeerReadLimitSinker->on_peer_readlimit(this);
			if(n>=0 && ret>n)
				ret = n;
			if(ret==0)
			{
				wait1 = 1;
				return;
			}
		}
		ret = m_ch->recv(m_block->pointer(),ret);
		if(ret<=0)
			return;
		m_block->increase(ret);
		if(0==m_block->remaining())
			on_data();
	}
}

void Peer::on(Writable,Channel* ch)
{
	fire(PeerListener::Writable(),this);
}
void Peer::on_data()
{
	if(8==m_block->limit())
	{
		int size = SerialStream::ltoh32(*((uint32*)(m_block->buf+4)));
		if(PTL_HEAD_STX_32!= *(uint32*)m_block->buf || size<=8 || size>(60<<10))
		{
			DEBUGMSG("#****wrong packet!****\n");
			disconnect(); //里面会删除m_block
			return;
		}
		m_block->limit(size);
	}
	else
	{
		m_block->position(0);
		fire(PeerListener::Data(),this,m_block->pointer(),m_block->limit());
		if(CONNECTED!=m_ch->get_state())
			return;
		m_block->limit(8);
	}
}

