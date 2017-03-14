#include "NattypeMgr.h"
#include "Util.h"
#include "Setting.h"
#include "Tracker.h"
#include "uac.h"

//************************ static fun ************************************
void NattypeMgr::callback_udp_onnatok(int nattype)
{
	//目前不处理，定时获取
}
void NattypeMgr::callback_udp_ipportchanged(unsigned int ip,unsigned short port)
{
	DEBUGMSG("#:-udp acceptor keeplive rsp \n");
	if(Util::is_private_ip(Util::ip_htoas(ip)))
		ip = 0; //支持tracker在内网时同内网可以互联
	if(g_netLiveInfo.udpRealIP!=ip || g_netLiveInfo.udpRealPort!=port)
	{
		DEBUGMSG("#:-udp acceptor keeplive rsp: realip:port=%s:%d \n",Util::ip_htoa(ip),port);
		g_netLiveInfo.udpRealIP = ip;
		g_netLiveInfo.udpRealPort = port;
		TrackerSngl::instance()->on_update_nattype();
	}
}

//************************************************************************

NattypeMgr::NattypeMgr(void)
:m_state(IDLE)
,m_peer(NULL)
,m_tryget_udp_nattype_times(0)
{
}

NattypeMgr::~NattypeMgr(void)
{
	
}
int NattypeMgr::init()
{
	if(m_peer)
		return 0;
	m_peer = new Peer(IPT_TCP,TCP_CONN,-1);
	m_peer->add_listener(this);
	if(0==g_netLiveInfo.tcpLocalIP)
	{	
		unsigned long ip=0;
		string strip = Util::get_local_private_ip_ex(10000);
		ip = Util::ip_atoh(strip.c_str());
		g_netLiveInfo.tcpLocalIP = ip;
		g_netLiveInfo.udpLocalIP = ip;
	}
	int ntype = GetPrivateProfileIntA("local","nat_type",6,SettingSngl::instance()->get_confini_path().c_str());
	if(ntype>=0&&ntype<=5)
	{
		g_netLiveInfo.natType = ntype;
		g_netLiveInfo.tcpRealPort = g_netLiveInfo.tcpLocalPort;
		g_netLiveInfo.udpRealPort = g_netLiveInfo.udpLocalPort;
		on_check_nat_ok();
		return 0;
	}
	//30分钟执行一次nat检查
	TimerSngl::instance()->register_timer(this,1,1800000);
	start_check_tcp_nattype();
	return 0;
}
void NattypeMgr::fini()
{
	if(!m_peer)
		return;
	m_peer->disconnect();
	TimerSngl::instance()->unregister_all(this);
	assert(DISCONNECTED==m_peer->get_channel()->get_state());
	m_peer->remove_listener(this);
	delete m_peer;
	m_peer = NULL;
}

void NattypeMgr::on_timer(int e)
{
	switch(e)
	{
	case 1:
		start_check_tcp_nattype();
		break;
	case 2:
		m_peer->disconnect();
		break;
	case 3:
		timer_tryget_udp_nattype();
		break;
	default:
		assert(false);
		break;
	}
}
void NattypeMgr::on(Connected,Peer* peer)
{
	assert(m_peer == peer);
	PTL_Head head;
	PTL_P2S_RequestTcpCheck req;
	req.tcpPort = g_netLiveInfo.tcpLocalPort;
	MemBlock* block = MemBlock::allot(1024);
	if(!block)
		return;
	PTLStream ss(block->buf,block->buflen);
	head.cmd = PTL_P2S_REQUEST_TCP_CHECK;
	ss << head;
	ss << req;
	ss.fitsize32(4);
	block->datalen = ss.length();
	m_peer->send(block);
}
void NattypeMgr::on(Disconnected,Peer* peer)
{
	assert(m_peer == peer);
	TimerSngl::instance()->unregister_timer(this,2);
	if(TCP_CHECKING==m_state)
	{
		m_state = IDLE;
		m_tryget_udp_nattype_times = 0;
		TimerSngl::instance()->register_timer(this,3,2000);
	}
}
void NattypeMgr::on(Data,Peer* peer,char* buf,int len)
{
	assert(m_peer == peer);
	PTL_Head head;
	PTL_P2S_ResponseTcpCheck rsp;
	PTLStream ss(buf,len,len);
	ss >> head;
	if(0==(ss >> rsp))
	{
		printf("#stun check: %d=rsp.result,%d=rsp.id,%d=g_netLiveInfo.sessionID \n",rsp.result,rsp.id,g_netLiveInfo.sessionID);
		if(0==rsp.result && (0==rsp.id || rsp.id == g_netLiveInfo.sessionID))
		{
			//等0时表示tracker还未登陆成功时就开始测试了，此时假设局域网主机一般不容易出现长时间未登陆成功tracker获取sessionID
			g_netLiveInfo.natType = 0;
			if(Util::is_private_ip(Util::ip_htoas(rsp.eyeIP)))
				rsp.eyeIP = 0; //支持tracker在内网时同内网可以互联
			//如果本地有填写外网IP，直接使用外网IP
			if(0!=SettingSngl::instance()->get_user_ip())
				rsp.eyeIP = SettingSngl::instance()->get_user_ip();
			g_netLiveInfo.tcpRealIP = rsp.eyeIP;
			g_netLiveInfo.udpRealIP = rsp.eyeIP;
			g_netLiveInfo.tcpRealPort = g_netLiveInfo.tcpLocalPort;
			on_check_nat_ok();
			//nat0以后都不再测试
			TimerSngl::instance()->unregister_timer(this,1);
		}
		else
		{
			if((uint32)-2==rsp.result)
			{
				//服务器忙无法测试
			}
		}
	}
	m_peer->disconnect();
}
void NattypeMgr::on_check_nat_ok()
{
	m_state = IDLE;
	g_netLiveInfo.isNatChecked = true;
	TrackerSngl::instance()->on_update_nattype();
}

int NattypeMgr::start_check_tcp_nattype()
{
	if(IDLE!=m_state || DISCONNECTED!=m_peer->get_channel()->get_state())
		return -1;
	TimerSngl::instance()->unregister_timer(this,3);
	string ip = Util::ip_explain_ex(SettingSngl::instance()->get_stun_ip().c_str());
	unsigned short port = SettingSngl::instance()->get_stun_port();
	m_state = TCP_CHECKING;
	TimerSngl::instance()->register_timer(this,2,6000); //TCP测试6秒内没断开即强行断开
	m_peer->connect(ip.c_str(),port,0);
	return 0;
}
void NattypeMgr::timer_tryget_udp_nattype()
{
	assert(IDLE==m_state);
	int nattype = uac_get_nattype();
	if(6!=nattype)
	{
		TimerSngl::instance()->unregister_timer(this,3);
		g_netLiveInfo.natType = nattype;
		on_check_nat_ok();
		return;
	}
	if(++m_tryget_udp_nattype_times>20)
	{
		//超过20*2=40秒都获取不到，就默认类似3
		DEBUGMSG("# NattypeMgr:: get_udp_nattype() timeout set nattype=3 \n");
		TimerSngl::instance()->unregister_timer(this,3);
		g_netLiveInfo.natType = 3;
		on_check_nat_ok();
		return;
	}
}

