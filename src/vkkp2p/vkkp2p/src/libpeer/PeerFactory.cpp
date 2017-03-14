#include "PeerFactory.h"
#include "Setting.h"

PeerFactory::PeerFactory(void)
:m_curr_tick(0)
,m_new_peer_times(0)
{
}

PeerFactory::~PeerFactory(void)
{
}
int PeerFactory::init()
{
	if(0!=m_peers.size)
		return -1;
	m_peers.resize(2*FD_SETSIZE);
	TimerSngl::instance()->register_timer(this,1,1000);
	TimerSngl::instance()->register_timer(this,2,30000);


	return 0;
}
int PeerFactory::fini()
{
	if(0==m_peers.size)
		return 0;
	put_all_peer();
	deal_pending();
	TimerSngl::instance()->unregister_all(this);
	assert(0==m_peers.count);
	m_peers.resize(0);

	return 0;
}

Peer* PeerFactory::get_peer(char ip_type,char conn_type,void* dref,bool turn)
{
	int index = m_peers.allot2();
	if(-1==index)
		return NULL;
	Peer *peer = new Peer(ip_type,conn_type,index);
	if(NULL==peer)
	{
		m_peers.free2(index);
		return NULL;
	}
	m_new_peer_times++;
	peer->add_listener(this);
	m_peers[index].peer = peer;
	m_peers[index].bturn = turn;
	m_peers[index].dref = dref;
	m_peers[index].peer->m_last_active_tick = m_curr_tick;
	return peer;
}
int PeerFactory::put_peer(Peer* peer)
{
	//考虑重入,如对方主动连开时先到on(disconnected),然后通知shareservice和downloadmanager释放时,可能会再调用到此.所以
	//on(disconnected)必须先设置回调句柄空
	assert(peer && m_peers[peer->get_index()].peer==peer);
	peer->disconnect();
	return 0;
}


void PeerFactory::put_all_peer()
{
	for(int i=0;i<=m_peers.cursor;++i)
	{
		if(m_peers[i].peer)
			m_peers[i].peer->disconnect();
	}
}
void PeerFactory::deal_pending()
{
	list<Peer*> ls;
	m_pending_peerls.swap(ls);
	for(list<Peer*>::iterator it=ls.begin();it!=ls.end();++it)
		delete *it;
}
void PeerFactory::deal_deadpeer()
{
	for(int i=0; i<=m_peers.cursor; i++)
	{
		if(m_peers[i].peer)
		{
			if(m_peers[i].peer->m_last_active_tick + 120 < m_curr_tick)
			{
				DEBUGMSG("#*** clear dead peer\n");
				LOG_disconnect(" PeerFactory clear dead peer ");
				m_peers[i].peer->disconnect();
			}
		}
	}
}
void PeerFactory::on_timer(int e)
{
	switch(e)
	{
	case 1:
		{
			m_curr_tick++;
			deal_pending();
		}
		break;
	case 2:
		{
			deal_deadpeer();
		}
		break;
	default:
		assert(0);
		break;
	}
}
bool PeerFactory::attach_tcp_socket(SOCKET fd,sockaddr_in& addr)
{
	Peer* peer = get_peer(IPT_TCP,TCP_TURN,NULL,false);
	if(!peer)
		return false;
	peer->attach(fd,addr);
	return true;
}
bool PeerFactory::uac_attach_socket(UAC_SOCKET fd,const UAC_sockaddr& addr)
{
	Peer* peer = get_peer(IPT_UDP,TCP_TURN,NULL,false);
	if(!peer)
		return false;
	sockaddr_in sock_addr;
	sock_addr.sin_port = htons(addr.port);
	sock_addr.sin_addr.s_addr = htonl(addr.ip);
	peer->get_channel()->attach(fd,sock_addr);
	return true;
}

void PeerFactory::on(PeerListener::Disconnected,Peer* peer)
{
	peer->remove_listener(this);
	on_disconnected(peer);
	m_peers.free2(peer->get_index());
	m_peers[peer->get_index()].reset();
	m_pending_peerls.push_back(peer);
}

