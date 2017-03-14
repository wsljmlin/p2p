#include "PeerManager.h"
#include "StatManager.h"
#include "Setting.h"
#include "Util.h"

PeerManager::PeerManager(void)
: m_binit(false)
, m_curr_sec(time(NULL))
, m_last_clear_dead_sec(m_curr_sec)
, m_peer_num(0)
, m_test_num(0)
, m_test_succeed_num(0)
{
}

PeerManager::~PeerManager(void)
{
}

int PeerManager::init()
{
	if(m_binit)
		return -1;
	m_binit = true;
	if(0!=TCPAcceptorSngl::instance()->open(SettingSngl::instance()->get_accept_port(),
		SettingSngl::instance()->get_accept_ip().c_str(),this,IOReactorSngl::instance()))
		return -1;
	m_peers.resize(PEER_MAX_SIZE);
	for(int i=0;i<m_peers.size;++i)
	{
		m_peers[i].peer = new Peer();
		m_peers[i].peer->add_listener(this);
		m_peers[i].peer->set_sid(i);
	}
	DEBUGMSG("#-PeerManager::init \n");
	return 0;
}
void PeerManager::fini()
{
	if(!m_binit)
		return;
	m_binit = false;
	TCPAcceptorSngl::instance()->close();
	put_all_peer();
	assert(0==m_peers.count);
	for(int i=0;i<m_peers.size;++i)
	{
		m_peers[i].peer->remove_listener(this);
		delete m_peers[i].peer;
	}
	m_peers.resize(0);

	TCPAcceptorSngl::destroy();
	DEBUGMSG("#-PeerManager::fini \n");
}


void PeerManager::handle_root()//处理收发线程
{
	static time_t curr_sec=0;
	//DEBUGMSG("+PeerManager::handle_root() \n");
	//io root
	IOReactorSngl::instance()->handle_root(50);

	//timer
	curr_sec = time(NULL);
	if(curr_sec != m_curr_sec)
	{
		m_curr_sec=curr_sec;
		handle_timer();
		StatManagerSngl::instance()->handle_timer();
	}
}
void PeerManager::on_line(Peer *peer,MemBlock *block)
{
	tmp_ss_read.attach(block->buf,block->buflen,block->datalen);
	if(0!=tmp_ss_read>>tmp_head)
		return;
	switch(tmp_head.cmd)
	{
	case PTL_P2S_REQUEST_TCP_CHECK:
		{
			if(0!=tmp_ss_read>>tmp_reqtcpcheck)
				return;
			on_ptl_request_tcpcheck(peer->get_sid(),tmp_reqtcpcheck);
		}
		break;
	case PTL_P2P_WELCOME:
		{
			if(0!=tmp_ss_read>>tmp_welcome)
				return;
			on_ptl_welcome(peer->get_sid(),tmp_welcome);
		}
		break;
	default:
		printf("#***unkown packet cmd: %d ***\n",tmp_head.cmd);
		break;
	}
}
void PeerManager::handle_timer()
{
	if(m_last_clear_dead_sec + 128 < m_curr_sec)
	{
		//每5分钟检查一次,超过两分半钟不活动的连接清掉
		m_last_clear_dead_sec = m_curr_sec;
		handle_dead_peer();
		int rate = m_test_num;
		if(rate) rate=m_test_succeed_num*100/rate;
		DEBUGMSG("#--------channel count = %d/%d ; succeedn/testn=%d/%d [%d] ---\n",
			m_peers.count,m_peer_num,
			m_test_succeed_num,m_test_num,rate);
	}
}
void PeerManager::handle_dead_peer()
{
	DEBUGMSG("#--- handle_dead_peer()...\n");
	for(int i=0;i<m_peers.size;++i)
	{
		if(DISCONNECTED!=m_peers[i].peer->get_channel()->get_state() && m_peers[i].last_active_time+180<m_curr_sec)
		{
			put_peer(m_peers[i].peer);
			DEBUGMSG("#--- close dead peer...\n");
		}
	}
}

bool PeerManager::attach_tcp_socket(SOCKET fd,sockaddr_in& addr)
{
	Peer *peer = get_peer();
	if(!peer)
		return false;
	if(0!=peer->attach(fd,addr))
	{
		put_peer(peer);
		return false;
	}
	return true;
}
Peer *PeerManager::get_peer(int psid/*=-1*/)
{
	int sid = m_peers.allot();
	if(-1==sid)
		return NULL;
	assert(DISCONNECTED==m_peers[sid].peer->get_channel()->get_state());
	m_peers[sid].last_active_time = m_curr_sec;
	m_peers[sid].psid = psid;
	m_peer_num++;
	return m_peers[sid].peer;
}
int PeerManager::put_peer(Peer *peer)
{
	int sid = peer->get_sid();
	if(0==m_peers.state[sid])
	{
		assert(DISCONNECTED==peer->get_channel()->get_state());
		return 0;
	}
	assert(peer == m_peers[sid].peer);
	m_peers[sid].peer->disconnect();
	return 0;
}
void PeerManager::put_all_peer()
{
	for(int i=0;i<m_peers.size;++i)
	{
		if(m_peers.state[i])
			put_peer(m_peers[i].peer);
	}
}
void PeerManager::on_ptl_request_tcpcheck(int sid,const PTL_P2S_RequestTcpCheck& req)
{
	//DEBUGMSG("# PeerManager::on_ptl_request_tcpcheck() ip:port=%s:%d \n",Util::ip_htoa(m_peers[sid].ip),req.tcpPort);
	m_test_num++;
	Peer *peer = get_peer(sid);
	if(NULL==peer)
	{
		ptl_response_tcpcheck(sid,-2);
	}
	else
	{
		m_peers[sid].test_port = req.tcpPort;
		m_peers[sid].csid = peer->get_sid();
		if(0!=peer->connect(m_peers[sid].ip,m_peers[sid].test_port))
		{
			ptl_response_tcpcheck(sid,-1);
			put_peer(peer);
		}
	}
}
void PeerManager::on_ptl_welcome(int sid,PTL_P2P_Welcome& req)
{
	//DEBUGMSG("# PeerManager::on_ptl_welcome() \n");
	int psid = m_peers[sid].psid;
	assert(-1==m_peers[sid].csid);
	assert(-1!=psid);
	if(-1!=psid)
	{
		ptl_response_tcpcheck(psid,0,req.sessionID);
		m_peers[sid].psid = -1; //防止断开时再次回复
	}
	put_peer(m_peers[sid].peer);

}
void PeerManager::ptl_response_tcpcheck(int sid,int result,uint32 dataid/*=0*/)
{
	if(0!=m_peers.state[sid])
	{
		assert(-1==m_peers[sid].psid);
		tmp_head.cmd = PTL_P2S_RESPONSE_TCP_CHECK;
		tmp_rsptcpcheck.result = result;
		tmp_rsptcpcheck.eyeIP = m_peers[sid].ip;
		tmp_rsptcpcheck.id = dataid;
		MemBlock *block = MemBlock::allot(1024,1);
		if(block)
		{
			tmp_ss_write.attach(block->buf,block->buflen,0);
			tmp_ss_write << tmp_head;
			tmp_ss_write << tmp_rsptcpcheck;
			tmp_ss_write.fitsize32(4);
			block->datalen = tmp_ss_write.length();
			m_peers[sid].peer->send(block);
		}
		//刚刚发完数据就关闭数据发不出，很可能出问题。导至收发缓冲出错,让对方自然断
		//put_peer(m_peer[sid].peer);
		if(0<=result) 
			m_test_succeed_num++;
	}
}
//***************************************

void PeerManager::on(PeerListener::Connected ,Peer* peer)
{
	m_peers[peer->get_sid()].ip = peer->get_channel()->get_hip();
	m_peers[peer->get_sid()].port = peer->get_channel()->get_hport();
}
void PeerManager::on(PeerListener::Disconnected ,Peer* peer)
{
	int sid = peer->get_sid();
	int psid = m_peers[sid].psid;
	int csid = m_peers[sid].csid;
	m_peers.free(sid);
	m_peers[sid].psid = -1;
	m_peers[sid].csid = -1;
	m_peers[sid].test_port = 0;
	m_peer_num--;
	if(-1!=psid)
	{
		assert(-1==csid);
		ptl_response_tcpcheck(psid,-1); 
	}
	if(-1!=csid)
	{
		assert(-1==psid);
		//可能子连接已经不存在
		put_peer(m_peers[csid].peer);
	}
}
void PeerManager::on(PeerListener::Data ,Peer* peer,MemBlock *block)
{
	//注意：block已经为一个游离块，所以投到某地方处理失败时，记得要put回块池
	m_peers[peer->get_sid()].last_active_time = m_curr_sec;
	on_line(peer,block);
	block->free(1);

}


