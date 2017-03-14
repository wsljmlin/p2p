#include "uac_UDPConnector.h"
#include "uac_Timer.h"
#include "uac_Util.h"


namespace UAC
{
UDPConnector::UDPConnector(SOCKET fd,UDPChannelFactory* chf,UDPStunClient* stun)
:m_fd(fd) 
,m_chf(chf)
,m_stun(stun)
,m_socknum(0)
,m_max_i(-1)
,m_tick(GetTickCount())
,m_mtu(Util::get_mtu())
{
	_tmp_ts_sbuf = new char[m_mtu];
	memset(_tmp_ts_sbuf,0xc5,m_mtu);
	m_udps = new UDPSession_t*[UAC_FD_SIZE];
	for(int i=0;i<UAC_FD_SIZE;++i)
		m_udps[i] = new UDPSession_t();
	TimerSngl::instance()->register_timer(this,1,200);
	//TimerSngl::instance()->register_timer(this,2,1);
}

UDPConnector::~UDPConnector(void)
{
	delete[] _tmp_ts_sbuf;
	TimerSngl::instance()->unregister_all(this);
	assert(0==m_socknum);
	for(int i=0;i<UAC_FD_SIZE;++i)
		delete m_udps[i];
	delete[] m_udps;
	m_natl.clear();
}

void UDPConnector::on_timer(int e)
{
	m_tick = GetTickCount();
	switch(e)
	{
	case 1:
		timer_resend();
		break;
	//case 2:
	//	timer_channel_send();
	//	break;
	default:
		assert(0);
		break;
	}
}

int UDPConnector::connect(UDPChannelHandler* ch,unsigned int ip,unsigned short port,int nattype)
{
	int i = register_channel(ch,ip,port,nattype);
	if(-1==i)
		return -1;
	m_udps[i]->state = UDP_CONNECTING;

	//send conn
	if(m_stun && m_udps[i]->des_nattype2 > 1)
		m_stun->ptl_request_hole(i,m_udps[i]->des_nip,m_udps[i]->des_nport);
	//if(m_udps[i]->des_nattype2 <= 1)
	{
		//等待通知对方成功后才发连接
		send_conn(i);
	}
	UACLOG("#[udp connect: %s:%d]\n",Util::ip_htoa(ip),port);
	return 0;
}
int UDPConnector::disconnect(UDPChannelHandler* ch)
{
	//连接发两个close
	if(ch->m_des_sessionid >= 0)
	{
		//有des_sessionid表示连接成功 disconn
		_tmp_shead.cmd = UDPS_CONN_CMD_CLOSE;
		_tmp_shead.src_sessionid = ch->m_src_sessionid;
		_tmp_shead.des_sessionid = ch->m_des_sessionid;
		send_head(_tmp_shead,ch->m_addr,2);
	}
	unregister_channel(ch);
	UACLOG("#[udp disconnect!!!] \n");
	return 0;
}
int UDPConnector::nat(unsigned int nip,unsigned short nport)
{
	UACLOG("#[udp nat] (%s:%d) \n",Util::ip_ntoa(nip),ntohs(nport));
	//查找到存在的不再处理
	for(NatIter it=m_natl.begin();it!=m_natl.end();++it)
	{
		if((*it).nip == nip && (*it).nport== nport)
			return 1;
	}


	NatInfo_t inf;
	inf.nip = nip;
	inf.nport = nport;
	inf.begin_tick = m_tick;
	inf.last_send_tick = m_tick;
	m_natl.push_back(inf);

	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inf.nip;
	addr.sin_port = inf.nport;
	_tmp_shead.cmd = UDPS_CONN_CMD_NAT;
	_tmp_shead.src_sessionid = 0;
	if(m_stun)
		_tmp_shead.src_sessionid = htonl(m_stun->get_binding_ip());
	send_head(_tmp_shead,addr);
	return 0;
}
int UDPConnector::on_rsp_nat_hole(unsigned int connid,unsigned int nip,unsigned short nport)
{
	//对端回应表示对方一定收到了NAT的请求，所以不再需要重发nat_req到对方去。
	if(connid<UAC_FD_SIZE)
	{
		if(m_udps[connid]->is_used && m_udps[connid]->des_nip==nip && m_udps[connid]->des_nport==nport)
			m_udps[connid]->des_nattype2 = 0;
	}
	return 0;
}
int UDPConnector::register_channel(UDPChannelHandler *ch,unsigned int ip,unsigned short port,int nattype)
{
	//todo: 注意，如果同连接同样的IP，当断开的时候并且立即再创建连接连接同样IP，很容易会错误
	int i=ch->__idx;
	assert(!m_udps[i]->is_used);
	if(m_udps[i]->is_used)
		return -1;
	m_socknum++;
	if(m_max_i<i) m_max_i = i;
	m_udps[i]->is_used = true;
	m_udps[i]->des_nip = htonl(ip);
	m_udps[i]->des_nport = htons(port);
	m_udps[i]->des_nattype = nattype;
	m_udps[i]->des_nattype2 = nattype;
	m_udps[i]->handle = ch;
	m_udps[i]->des_sessionid = -1;
	m_udps[i]->src_sessionid = i;
	m_udps[i]->last_send_tick = m_tick;
	m_udps[i]->last_recv_tick = m_tick;
	m_udps[i]->begin_tick = m_tick;

	ch->m_fd = m_fd;
	ch->m_src_sessionid = i;
	ch->m_des_sessionid = -1;
	memset(&ch->m_addr,0,sizeof(ch->m_addr));
	ch->m_addr.sin_family = AF_INET;
	ch->m_addr.sin_port = htons(port);
	ch->m_addr.sin_addr.s_addr = htonl(ip);
	return i;
}
void UDPConnector::check_update_port(int i,unsigned short nport)
{
	if(m_udps[i]->des_nport!=nport)
	{
		printf("#******** UDPConnector desport changed (%d,%d)\n",ntohs(m_udps[i]->des_nport),ntohs(nport));
		m_udps[i]->des_nport=nport;
		if(m_udps[i]->handle)
			m_udps[i]->handle->m_addr.sin_port = nport;
	}

}
void UDPConnector::unregister_channel(UDPChannelHandler *ch)
{
	int i = ch->__idx;
	assert(m_udps[i]->is_used);
	if(!m_udps[i]->is_used)
		return;

	//发起方连接失败的上报记录
	if(UDP_CONNECTING == m_udps[i]->state && m_stun)
	{
		PTL_STUN_ReportConnSF_t inf;
		inf.bsucceed = 0;
		inf.des_nattype = m_udps[i]->des_nattype;
		inf.des_nip = m_udps[i]->des_nip;
		inf.des_nport = m_udps[i]->des_nport;
		//注意src_nattype在stun里面赋值
		m_stun->ptl_report_connsf(inf);
	}

	m_udps[i]->reset();
	m_udps[i]->begin_tick = m_tick;
	ch->m_des_sessionid = -1;
	ch->m_src_sessionid = -1;
	ch->m_fd = INVALID_SOCKET;
	m_socknum--;
	if(i==m_max_i) m_max_i--;
	while(m_max_i>=0 && !m_udps[m_max_i]->is_used)
		m_max_i--;
}
int UDPConnector::send_head(const UDPSConnHeader_t& head,sockaddr_in& addr,int send_times/*=1*/)
{
	int ret = 0;
	if(INVALID_SOCKET == m_fd)
		return -1;
	_tmp_sps.attach(_tmp_sbuf,512,0);
	if(0==(_tmp_sps<< head))
	{
		for(int i=0;i<send_times;++i)
			ret = ::sendto(m_fd,_tmp_sbuf,_tmp_sps.length(),0,(sockaddr*)&addr,sizeof(addr));
	}
	return ret;
}
void UDPConnector::remove_nat(unsigned int nip,unsigned short nport)
{
	//取消发打洞包,对方可能是nat4,如果是nat4只要ip相同就册，否则ip,port相同才删。
	NatIter it1,it2,it;
	it1=it2=m_natl.end();
	for(it=m_natl.begin();it!=m_natl.end();++it)
	{
		if((*it).nip == nip)
		{
			it1 = it;
			if((*it).nport == nport)
			{
				it2 = it;
				break;
			}
		}
	}
	if(it2 != m_natl.end())
		m_natl.erase(it2);
	else if(it1 != m_natl.end())
		m_natl.erase(it1);
}
int UDPConnector::on_data(memblock* block,sockaddr_in& addr)
{
	//负责block的释放
	char* buf = block->buffer;
	int size = block->datasize;
	_tmp_rps.attach(buf,size,size);
	if(0!=_tmp_rps>>_tmp_rhead)
	{
		assert(false);
		UACLOG("#*** UDPConnector::on_data() wrong packet!!! \n");
		return -1;
	}
	buf = _tmp_rps.read_ptr();
	size = _tmp_rps.length();
	block->datapos = (int)(buf - block->buffer);

	switch(_tmp_rhead.cmd)
	{
	case UDPS_CONN_CMD_NAT:
		handle_cmd_nat(_tmp_rhead,buf,size,addr);
		break;
	case UDPS_CONN_CMD_CONN:
		handle_cmd_conn(_tmp_rhead,buf,size,addr,_tmp_rps);
		break;
	case UDPS_CONN_CMD_OK:
		handle_cmd_ok(_tmp_rhead,buf,size,addr,_tmp_rps);
		break;
	case UDPS_CONN_CMD_CLOSE:
		handle_cmd_close(_tmp_rhead,buf,size,addr);
		break;
	case UDPS_CONN_CMD_LIVE:
		handle_cmd_live(_tmp_rhead,buf,size,addr);
		break;
	case UDPS_CONN_CMD_DATA:
		handle_cmd_channel_packet(_tmp_rhead,block,addr);
		block = NULL;
		break;
	case UDPS_CONN_CMD_TESTSPEED_DATA:
		handle_cmd_testspeed_data(_tmp_rhead,buf,size,addr,_tmp_rps);
		break;
	case UDPS_CONN_CMD_TESTSPEED_ACK:
		handle_cmd_testspeed_ack(_tmp_rhead,buf,size,addr,_tmp_rps);
		break;
	default:
		assert(false);
		UACLOG("#:***-udp unknow packet--- cmd=%d [%s:%d] id=%d \n",
			_tmp_rhead.cmd,inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),_tmp_rhead.src_sessionid);
		break;
	}
	if(block) block->free(UAC_THREAD_CORE);
	return 0;
}
bool UDPConnector::check_ipport_ok(UDPSConnHeader_t& head,sockaddr_in& addr)
{
	unsigned int ip = addr.sin_addr.s_addr;
	unsigned short port = addr.sin_port;
	int i = head.des_sessionid;
	if(i<0 || i>=UAC_FD_SIZE)
	{
		UACLOG("#:***cmd:%d -> overflow [%s:%d] id=%d \n",(int)head.cmd,inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return false;
	}
	if(!m_udps[i]->is_used)
	{
		UACLOG("#:***cmd:%d -> old session [%s:%d] id=%d \n",(int)head.cmd,inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return false;
	}
	if(m_udps[i]->des_nip != ip/* || m_udps[i]->des_nport != port*/)
	{
		UACLOG("#:***cmd:%d  -> wrong ip-port [%s:%d] id=%d \n",(int)head.cmd,inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return false;
	}
	check_update_port(i,port);
	return true;
}
bool UDPConnector::check_ipport_session_ok(UDPSConnHeader_t& head,sockaddr_in& addr)
{
	unsigned int ip = addr.sin_addr.s_addr;
	unsigned short port = addr.sin_port;
	int i = head.des_sessionid;
	if(i<0 || i>=UAC_FD_SIZE)
	{
		UACLOG("#:***cmd:%d -> overflow [%s:%d] id=%d \n",(int)head.cmd,inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return false;
	}
	if(!m_udps[i]->is_used)
	{
		UACLOG("#:***cmd:%d -> old session [%s:%d] id=%d \n",(int)head.cmd,inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return false;
	}
	//有些端口是会变化的,在此忽略变化
	if(m_udps[i]->des_nip != ip /*|| m_udps[i]->des_nport != port*/ || m_udps[i]->des_sessionid != head.src_sessionid)
	{
		UACLOG("#:***cmd:%d  -> wrong ip-port-session [%s:%d] id=%d \n",(int)head.cmd,inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return false;
	}
	check_update_port(i,port);
	return true;
}
int UDPConnector::handle_cmd_nat(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr)
{
	//我主动连接对方，对方发的打洞包
	//我是nat4,对方是nat2时，对方端口并不会变
	//我是nat2,对方是nat4时，对方端口会改变，此时要更新端口
	unsigned int ip = addr.sin_addr.s_addr;
	unsigned short port = addr.sin_port;
	//UACLOG("#:on UDPS_CMD_NAT [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	int index1=-1,index2=-1;
	for(int i=0;i<=m_max_i;++i)
	{
		if(m_udps[i]->is_used && m_udps[i]->state==UDP_CONNECTING && (m_udps[i]->des_nip == ip || m_udps[i]->des_nip==(unsigned int)head.src_sessionid)) 
		{
			assert(m_udps[i]->des_sessionid == -1);
			index1=i;
			if(m_udps[i]->des_nport == port)
			{
				index2=i;
				break;
			}
		}
	}
	if(index2>=0)
	{
		//对方可能nat4也可能nat2/nat3,nat1没有nat包，让上层继续发conn
		////回复conn
		//对方双IP，stun看到的是src_sessionid，而我只能连接ip
		if(m_udps[index2]->des_nip != ip && m_udps[index2]->des_nip==(unsigned int)head.src_sessionid)
		{
			m_udps[index2]->des_nip = ip;
			m_udps[index2]->handle->m_addr.sin_addr.s_addr = ip;
			UACLOG("# ... UDP NAT ip changed \n");
		}
	}
	else if(index1>=0)
	{
		//我是nat2,对方nat4,更新对方的端口
		m_udps[index1]->des_nport = port;
		m_udps[index1]->handle->m_addr.sin_port = port;
		//对方双IP，stun看到的是src_sessionid，而我只能连接ip
		if(m_udps[index1]->des_nip != ip && m_udps[index1]->des_nip==(unsigned int)head.src_sessionid)
		{
			m_udps[index1]->des_nip = ip;
			m_udps[index1]->handle->m_addr.sin_addr.s_addr = ip;
			UACLOG("# ... UDP NAT ip port changed \n");
		}
		index2 = index1;
	}
	else
		return 0;
	//再发conn
	m_udps[index2]->des_nattype2 = 0;
	send_conn(index2);
	return 0;
}

int UDPConnector::handle_cmd_conn(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps)
{
	//我发打洞包，收对方连接请求,不用管对方是什么类型
	unsigned int ip = addr.sin_addr.s_addr;
	unsigned short port = addr.sin_port;
	uint32 mtu = 0;
	ps >> mtu; //
	if(0==mtu) mtu = 1500;

	//UACLOG("#:on UDPS_CMD_CONN [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	int i=0;
	//查找旧连接
	for(i=0;i<=m_max_i;++i)
	{
		if(m_udps[i]->is_used&&m_udps[i]->des_nip == ip/*&&m_udps[i]->des_nport == port*/ && m_udps[i]->des_sessionid == head.src_sessionid)
		{
			check_update_port(i,port);
			if(UDP_ACCEPTING == m_udps[i]->state)
			{
				//回复ok
				send_conn_ok(i);
			}
			else
			{
				//已经是一个过时包
				UACLOG("#:udp old UDPS_CMD_CONN!!! [%s:%d]  id=%d\n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
			}
			return 0;
		}
	}

	remove_nat(ip,port);

	//请求创建一个连接回应,注意attach_accepting_udp_channel里面channel要记为正在连接状态:CONNECTING
	UDPChannelHandler *ch = m_chf->attach_udp_channel(m_fd,addr,this);
	if(NULL==ch)
	{
		//连接满
		head.des_sessionid = head.src_sessionid;
		head.cmd = UDPS_CONN_CMD_CLOSE;
		send_head(head,addr);
		return 0;
	}

	//当测试127.0.0.1连接时，刚好对方超时断开后，此时又收到对方断开前的CONN包，得到的i_noused往往就是与head.src_sessionid相等，即变成了对方就是自己。这是极端
	i = register_channel(ch,ntohl(ip),ntohs(port),1);
	m_udps[i]->state = UDP_ACCEPTING;
	m_udps[i]->des_sessionid = head.src_sessionid;
	m_udps[i]->handle->m_des_sessionid = head.src_sessionid;
	m_udps[i]->handle->m_mtu = mtu<m_mtu?mtu:m_mtu; //取小的

	send_conn_ok(i);
	
	//UACLOG("#[udp Accepting connection...!] [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	return 0;
}
int UDPConnector::handle_cmd_ok(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps)
{
	//unsigned int ip = addr.sin_addr.s_addr;
	//unsigned short port = addr.sin_port;
	int i = head.des_sessionid;
	uint32 mtu = 0;

	remove_nat(addr.sin_addr.s_addr,addr.sin_port);
	if(!check_ipport_ok(head,addr))
		return -1;
	
	ps >> mtu;
	ps >> m_udps[i]->my_nip;
	ps >> m_udps[i]->my_nport;

	if(0==mtu) mtu = 1500;
	m_udps[i]->handle->m_mtu = mtu<m_mtu?mtu:m_mtu; //取小的

	if(UDP_CONNECTING == m_udps[i]->state || UDP_ACCEPTING == m_udps[i]->state || UDP_ACCEP_TESTSPEEDING==m_udps[i]->state)
	{
		if(UDP_CONNECTING == m_udps[i]->state)
		{
			assert(m_udps[i]->des_sessionid == -1);
			m_udps[i]->des_sessionid = head.src_sessionid;
			m_udps[i]->handle->m_des_sessionid = head.src_sessionid;

			on_connecting_ok(i,addr);
			//取消测速
			//if(m_udps[i]->my_nip == m_udps[i]->des_nip)
			//{
			//	m_udps[i]->handle->_test_speedB = 1024000;
			//	on_connecting_ok(i,addr);
			//}
			//else
			//{
			//	m_udps[i]->state = UDP_CONN_TESTSPEEDING;
			//	send_testspeed_data(i);
			//}
		}
		else if(UDP_ACCEPTING == m_udps[i]->state || UDP_ACCEP_TESTSPEEDING==m_udps[i]->state)
		{
			if(m_udps[i]->des_sessionid != head.src_sessionid)
			{
				//UACLOG("#:***UDPS_CMD_OK  -> local desid!=remote srcid [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),head.src_sessionid);
				return -1;
			}
			if(m_udps[i]->my_nip == m_udps[i]->des_nip)
			{
				m_udps[i]->handle->_test_speedB = 1024000;
			}
			m_udps[i]->state = UDP_ACCEPTED;
			UACLOG("#[udp accept succeed] [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),head.src_sessionid);
			m_udps[i]->handle->handle_connected();
		}
	}
	return 0;
}
void UDPConnector::on_connecting_ok(int i,sockaddr_in& addr)
{
	m_udps[i]->state = UDP_CONNECTED;

	//如果是发起方，会继续发ok
	send_conn_ok(i);

	UACLOG("#[udp connect succeed] [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),i);
	//发起方连接成功的上报记录
	if(m_stun)
	{
		PTL_STUN_ReportConnSF_t inf;
		inf.bsucceed = 1;
		inf.des_nattype = m_udps[i]->des_nattype;
		inf.des_nip = m_udps[i]->des_nip;
		inf.des_nport = m_udps[i]->des_nport;
		//注意src_nattype在stun里面赋值
		m_stun->ptl_report_connsf(inf);
	}
	m_udps[i]->handle->handle_connected();
}
int UDPConnector::handle_cmd_live(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr)
{
	unsigned int ip = addr.sin_addr.s_addr;
	unsigned short port = addr.sin_port;
	int i = head.des_sessionid;
	//UACLOG("#:on UDPS_CMD_LIVE [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	if(i<0 || i>=UAC_FD_SIZE)
	{
		UACLOG("#:***UDPS_CMD_LIVE -> overflow [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return -1;
	}
	if(m_udps[i]->is_used && m_udps[i]->des_nip == ip /*&& m_udps[i]->des_nport == port*/ && m_udps[i]->des_sessionid == head.src_sessionid)
	{
		check_update_port(i,port);
		m_udps[i]->last_recv_tick = m_tick;
		if(UDP_ACCEPTING == m_udps[i]->state)
		{
			//考虑没有收到ok包的情况
			m_udps[i]->state = UDP_ACCEPTED;
			UACLOG("#:udp accept connect succeed [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);

			m_udps[i]->handle->handle_connected();
		}
	}
	else
	{
		head.cmd = UDPS_CONN_CMD_CLOSE;
		head.des_sessionid = head.src_sessionid;
		head.src_sessionid = i;
		send_head(head,addr);
		UACLOG("#:***UDPS_CMD_LIVE -> is closed [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	}
	return 0;
}

int UDPConnector::handle_cmd_close(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr)
{
	unsigned int ip = addr.sin_addr.s_addr;
	unsigned short port = addr.sin_port;
	//UACLOG("#:on UDPS_CMD_CLOSE [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	int i = head.des_sessionid;
	if(i<0 || i>=UAC_FD_SIZE)
	{
		UACLOG("#:***UDPS_CMD_CLOSE -> overflow [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		return -1;
	}
	if(!m_udps[i]->is_used)
		return -1;
	if(m_udps[i]->des_nip != ip /*|| m_udps[i]->des_nport != port*/ || (m_udps[i]->des_sessionid>=0&& m_udps[i]->des_sessionid != head.src_sessionid))
		return -1;
	check_update_port(i,port);
	//考虑对方未连接成功的情况
	//UACLOG("#:udp close [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	m_udps[i]->handle->handle_disconnected();
	return 0;
}
int UDPConnector::handle_cmd_channel_packet(UDPSConnHeader_t& head,memblock* block,sockaddr_in& addr)
{
	unsigned int ip = addr.sin_addr.s_addr;
	unsigned short port = addr.sin_port;
	//UACLOG("#:on UDPS_CMD_DATA OR ACK [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
	int i = head.des_sessionid;
	if(i<0 || i>=UAC_FD_SIZE)
	{
		UACLOG("#:***UDPS_CMD_DATA ACK -> overflow [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		block->free(UAC_THREAD_CORE);
		return -1;
	}

	if(!m_udps[i]->is_used || m_udps[i]->des_nip != ip /*|| m_udps[i]->des_nport != port*/ || m_udps[i]->des_sessionid != head.src_sessionid)
	{
		head.cmd = UDPS_CONN_CMD_CLOSE;
		head.des_sessionid = head.src_sessionid;
		send_head(head,addr);	
		UACLOG("#:***UDPS_CMD_DATA ACK -> wrong packet [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);
		block->free(UAC_THREAD_CORE);
		return -1;
	}
	check_update_port(i,port);
	if(UDPS_CONN_CMD_DATA == head.cmd)
	{
		if(UDP_ACCEPTING == m_udps[i]->state)
		{
			//考虑没有收到ok包的情况
			m_udps[i]->state = UDP_ACCEPTED;
			UACLOG("#:udp accept connect succeed [%s:%d] id=%d \n",inet_ntoa(addr.sin_addr),ntohs(port),head.src_sessionid);

			m_udps[i]->handle->handle_connected();
		}
	}

	//有可UDP_CONNECTING时收到上一次建立连接的包
	if(UDP_CONNECTED == m_udps[i]->state || UDP_ACCEPTED==m_udps[i]->state)
	{
		m_udps[i]->last_recv_tick = m_tick;
	
		//buf为真实数据部分，对于回应包应该为0
		m_udps[i]->handle->handle_recv(block);
	}
	else
	{
		block->free(UAC_THREAD_CORE);
	}

	return 0;
}

int UDPConnector::handle_cmd_testspeed_data(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps)
{
	int i = head.des_sessionid;
	if(!check_ipport_session_ok(head,addr))
		return -1;

	UDPS_ptl_TestSpeed_data_t inf;
	if(0!=ps >> inf)
		return -1;
	//UACLOG(".ts.data[%d]\n",inf.seq);
	m_udps[i]->ts.onrecv(inf,size);
	if(m_udps[i]->ts.recv_num>1 && 0==m_udps[i]->ts.recv_num%2)
	{
		//回复ack
		UDPS_ptl_TestSpeed_ack_t ack;
		ack.sid = inf.sid;
		ack.seq = inf.seq;
		ack.size = m_udps[i]->ts.recv_num;
		ack.speedB = m_udps[i]->ts.get_speed();

		UDPSConnHeader_t& head = _tmp_shead;
		head.cmd = UDPS_CONN_CMD_TESTSPEED_ACK;
		head.src_sessionid = m_udps[i]->src_sessionid;
		head.des_sessionid = m_udps[i]->des_sessionid;
		_tmp_sps.attach(_tmp_ts_sbuf,m_mtu,0);
		_tmp_sps<< head;
		_tmp_sps<< ack;
		::sendto(m_fd,_tmp_ts_sbuf,_tmp_sps.length(),0,(sockaddr*)&addr,sizeof(addr));
	}
	
	//如果是接受方
	if(UDP_ACCEPTING == m_udps[i]->state && inf.seq > inf.size/2)
	{
		m_udps[i]->state = UDP_ACCEP_TESTSPEEDING;
		send_testspeed_data(i);
	}
	//如果是发起方
	if(UDP_CONN_TESTSPEEDING == m_udps[i]->state && inf.seq >= inf.size*2/3)
	{
		on_connecting_ok(i,addr);
	}
	return 0;
}
int UDPConnector::handle_cmd_testspeed_ack(UDPSConnHeader_t& head,char* buf,int size,sockaddr_in& addr,PTLStream& ps)
{
	int i = head.des_sessionid;
	if(!check_ipport_session_ok(head,addr))
		return -1;

	UDPS_ptl_TestSpeed_ack_t inf;
	if(0!=ps >> inf)
		return -1;
	
	//UACLOG(".ts.ack[%d]\n",inf.seq);
	if(UDP_DISCONNECTED==m_udps[i]->state)
		return -1;
	if(m_udps[i]->ts.my_send_recv_num < inf.size)
	{
		m_udps[i]->ts.my_send_recv_num = inf.size; //表示对方收到的包的个数
		m_udps[i]->ts.my_speedB = inf.speedB;
		m_udps[i]->handle->_test_speedB = inf.speedB;
	}
	return 0;
}
void UDPConnector::send_conn(int i)
{
	if(INVALID_SOCKET == m_fd)
		return;
	
	m_udps[i]->last_send_tick = m_tick;
	_tmp_shead.cmd = UDPS_CONN_CMD_CONN;
	_tmp_shead.des_sessionid = m_udps[i]->des_sessionid;
	_tmp_shead.src_sessionid = m_udps[i]->src_sessionid;

	_tmp_sps.attach(_tmp_sbuf,512,0);
	_tmp_sps << _tmp_shead;
	_tmp_sps << m_mtu;
	::sendto(m_fd,_tmp_sbuf,_tmp_sps.length(),0,(sockaddr*)&m_udps[i]->handle->m_addr,sizeof(m_udps[i]->handle->m_addr));
}
void UDPConnector::send_conn_ok(int i)
{
	if(INVALID_SOCKET == m_fd)
		return;
	
	m_udps[i]->last_send_tick = m_tick;

	UDPSConnHeader_t& head = _tmp_shead;
	head.cmd = UDPS_CONN_CMD_OK;
	head.des_sessionid = m_udps[i]->des_sessionid;
	head.src_sessionid = m_udps[i]->src_sessionid;

	_tmp_sps.attach(_tmp_sbuf,512,0);
	_tmp_sps << head;
	_tmp_sps << m_mtu;
	_tmp_sps << m_udps[i]->des_nip;
	_tmp_sps << m_udps[i]->des_nport;
	::sendto(m_fd,_tmp_sbuf,_tmp_sps.length(),0,(sockaddr*)&m_udps[i]->handle->m_addr,sizeof(m_udps[i]->handle->m_addr));
}
void UDPConnector::send_testspeed_data(int i)
{
	if(INVALID_SOCKET == m_fd)
		return;


	UDPS_ptl_TestSpeed_data_t inf;
	UDPSConnHeader_t& head = _tmp_shead;

	m_udps[i]->ts.my_begin_tick = m_tick;
	m_udps[i]->ts.my_send_num = g_udps_conf.udp_testspeed_num; //一次发20个包
	m_udps[i]->last_send_tick = m_tick;

	inf.sid = 0;
	inf.size = m_udps[i]->ts.my_send_num; 
	head.cmd = UDPS_CONN_CMD_TESTSPEED_DATA;
	head.src_sessionid = m_udps[i]->src_sessionid;
	head.des_sessionid = m_udps[i]->des_sessionid;

	for(int j=0;j<inf.size;++j)
	{
		inf.seq = j;
		_tmp_sps.attach(_tmp_ts_sbuf,m_mtu,0);
		_tmp_sps<< head;
		_tmp_sps<< inf;
		::sendto(m_fd,_tmp_ts_sbuf,m_mtu-60,0,(sockaddr*)&m_udps[i]->handle->m_addr,sizeof(m_udps[i]->handle->m_addr));
		if(0==j%10)
			Sleep(0);
	}
}
void UDPConnector::timer_resend()
{
	UDPSConnHeader_t& head = _tmp_shead;
	for(int i=0;i<=m_max_i;++i)
	{
		if(!m_udps[i]->is_used)
			continue;
		
		if(UDP_ACCEPTING == m_udps[i]->state || UDP_CONNECTING == m_udps[i]->state || UDP_CONN_TESTSPEEDING == m_udps[i]->state || UDP_ACCEP_TESTSPEEDING == m_udps[i]->state)
		{
			//1.创建的"主连接/被连接"15秒后不成功断开
			if((m_udps[i]->begin_tick+15000) < m_tick)
			{
				UACLOG("#[udp timeout! STATE=%d] \n",m_udps[i]->state);
				m_udps[i]->handle->handle_disconnected();
				continue;
			}
		}
		//此连接15秒内没有收过数据即超时断开
		if((m_udps[i]->last_recv_tick+g_udps_conf.udp_conn_timeout_msec) < m_tick)
		{
			UACLOG("#[udp un-active timeout!(%d sec)] \n",g_udps_conf.udp_conn_timeout_msec/1000);
			m_udps[i]->handle->handle_disconnected();
			continue;
		}

		if(m_udps[i]->state == UDP_CONNECTED || m_udps[i]->state == UDP_ACCEPTED)
		{
			//2.已成功的连接启动重发,;5秒内没收到过数据，保活一次
			if(m_udps[i]->last_recv_tick+g_udps_conf.udp_keeplive_timer_msec<m_tick && m_udps[i]->last_send_tick+g_udps_conf.udp_keeplive_timer_msec < m_tick)
			{
				//keeplive
				UACLOG("#:UDPS_CMD_LIVE [%s:%d] id=%d \n",inet_ntoa(m_udps[i]->handle->m_addr.sin_addr),ntohs(m_udps[i]->des_nport),m_udps[i]->src_sessionid);
				head.cmd = UDPS_CONN_CMD_LIVE;
				head.des_sessionid = m_udps[i]->des_sessionid;
				head.src_sessionid = m_udps[i]->src_sessionid;
				send_head(head,m_udps[i]->handle->m_addr);
				m_udps[i]->last_send_tick = m_tick;
			}
		}
		else if(m_udps[i]->state == UDP_ACCEPTING)
		{
			//"被连接"每0.8秒重发一次"ok"
			if( (m_udps[i]->last_send_tick + 800) <= m_tick)
			{
				//只有未收到过测速包，才重发
				if(m_udps[i]->ts.recv_num==0)
				{
					send_conn_ok(i);
					//UACLOG("# udp resend UDPS_CMD_OK! \n");
				}
			}
		}
		else if(m_udps[i]->state == UDP_CONNECTING)
		{
			//主连接每0.8秒发一次"connect"
			if((m_udps[i]->last_send_tick + 800) < m_tick)
			{
				//重发conn
				assert(m_udps[i]->des_sessionid == -1);	
				if(m_stun && m_udps[i]->des_nattype2 > 1)
					m_stun->ptl_request_hole(i,m_udps[i]->des_nip,m_udps[i]->des_nport);
				
				//if(m_udps[i]->des_nattype2 <= 1)
				{
					//等待通知对方成功后才发连接
					send_conn(i);
					//UACLOG("# udp resend UDPS_CMD_CONN! (%s:%d)\n",inet_ntoa(m_udps[i]->handle->m_addr.sin_addr),ntohs(m_udps[i]->handle->m_addr.sin_port));
				}
			}
		}
		else if(UDP_CONN_TESTSPEEDING == m_udps[i]->state)
		{
			if(m_udps[i]->ts.my_begin_tick + 3000 < m_tick)
			{
				//测速超过3秒未完成，则当完成
				//UDPTestSpeed& ts = m_udps[i]->ts;
				on_connecting_ok(i,m_udps[i]->handle->m_addr);
			}
		}
		else if(UDP_ACCEP_TESTSPEEDING == m_udps[i]->state)
		{
		}
		else
			assert(0);
	}

	//resend nat
	NatIter it;
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	head.cmd = UDPS_CONN_CMD_NAT;
	for(it=m_natl.begin();it!=m_natl.end();)
	{
		if( ((*it).begin_tick + 10000) < m_tick )
		{
			UACLOG("#[udp nat timeout!] (%s:%d) \n",Util::ip_ntoa((*it).nip),ntohs((*it).nport));
			m_natl.erase(it++);
			continue;
		}
		//打洞包每1秒发一个,10秒后取消.
		if(((*it).last_send_tick + 800) < m_tick)
		{
			addr.sin_addr.s_addr = (*it).nip;
			addr.sin_port = (*it).nport;
			send_head(head,addr);
			(*it).last_send_tick = m_tick;
			//UACLOG("#: udp resend UDPS_CMD_NAT (%s:%d) \n",Util::ip_ntoa((*it).nip),ntohs((*it).nport));
		}
		++it;
	}
}
void UDPConnector::timer_channel_send()
{
	for(int i=0;i<=m_max_i;++i)
	{
		if(!m_udps[i]->is_used)
			continue;
		if(m_udps[i]->state == UDP_CONNECTED || m_udps[i]->state == UDP_ACCEPTED)
		{
			m_udps[i]->handle->handle_send();//可能已经disconnected
		}
	}
}

}

