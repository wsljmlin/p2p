#include "uac_UDPChannel.h"
#include "uac_Timer.h"

namespace UAC
{

enum {
	TIMER_SEND=1
	,TIMER_ACK
	,TIMER_BWWIN
};

UDPChannel::UDPChannel(UDPConnector* ctr,int idx)
:UDPChannelHandler(idx)
,m_ctr(ctr)
,m_bsend_timer(false)
,m_begin_tick(0)
,m_des_nattype(7)
{
	assert(ctr);
	_tmp_cmdbuf = new char[1024];
	m_last_active_tick = GetTickCount();
	_tmp_head.cmd = UDPS_CONN_CMD_DATA;
	m_send_list_max_size = m_send_win.spctrl.other_recv_win_num + m_send_win.spctrl.other_recv_win_num/2 + 20;
}

UDPChannel::~UDPChannel(void)
{
	delete[] _tmp_cmdbuf;
}

void UDPChannel::reset()
{
	m_recv_win.reset();
	m_send_win.reset();
	for(SendIter it=m_send_list.begin();it!=m_send_list.end(); ++it)
		(*it).block->free(UAC_THREAD_CORE);
	m_send_list.clear();
	for(RecvIter it=m_recv_map.begin();it!=m_recv_map.end();++it)
		it->second.block->free(UAC_THREAD_CORE);
	m_recv_map.clear();
	TimerSngl::instance()->unregister_all(this);
	m_bsend_timer = false;
	Channel::reset();
}
void UDPChannel::on_connected()
{
	//if(_test_speedB > 0)
	//{
	//	UACLOG("#------ test send speed ok : %d KB -----\n",_test_speedB/1024);
	//	m_send_win.spctrl.reset_speed(_test_speedB>1024000?1024000:_test_speedB);
	//	
	//}
	assert(m_mtu>0);
	if(g_udps_conf.packet_size>0)
		m_mtu = g_udps_conf.packet_size;
	m_mtu -= (34+UDPHEAD_LENGTH); //ȥ��IPͷ��UDPͷ����28,�Լ������ͷ����Լ30,����һЩ
	m_send_win.spctrl.mtu = m_mtu;

	m_state = CONNECTED;
	TimerSngl::instance()->register_timer(this,TIMER_ACK,g_udps_conf.udp_nak_timer_msec);
	TimerSngl::instance()->register_timer(this,TIMER_BWWIN,1000);
	//TimerSngl::instance()->register_timer(this,10,1000);
	m_begin_tick = GetTickCount();
	fire(ChannelListener::Connected(),this);
}
void UDPChannel::on_disconnected()
{
	UDPStunClient *stun = m_ctr->get_stun();
	//ֻ�ϱ���¼���ͷ�
	if(CONNECTED==m_state && stun && m_send_win.send_sizeB>m_recv_win.recv_sizeB)
	{
		//m_des_nattypeΪ7ʱ��ʾ�Լ��ǽ�������һ��
		PTL_STUN_ReportConnStat_t inf;
		inf.des_nattype = m_des_nattype;
		inf.des_nip = htonl(m_hip);
		inf.des_nport = htons(m_hport);
		
		inf.sizeKB = (unsigned int)(m_send_win.send_sizeB>>10);
		inf.sec = (GetTickCount()-m_begin_tick)/1000;
		inf.ttlMS = m_send_win.ttl/1000;
		inf.send_num = m_send_win.send_num;
		inf.resend_num = m_send_win.resend_num;
		inf.resend_timeo_num = m_send_win.resend_timeo_num;
		inf.other_rerecv_num = m_send_win.other_rerecv_num;
		stun->ptl_report_connstat(inf);
	}
	reset();
	fire(ChannelListener::Disconnected(),this);
}

void UDPChannel::on_data(memblock* b)
{
	//�����ݷŵ�pool�������,��ʱҪ���´���
	if(m_recv_win.recv_win_num>0)
		m_recv_win.recv_win_num--;
	fire(ChannelListener::Data(),this,b);
}
void UDPChannel::on_writable()
{
	if(CONNECTED==m_state)
		fire(ChannelListener::Writable(),this);
}

UDPChannelHandler* UDPChannel::get_udpchannel_handler()
{
	return static_cast<UDPChannelHandler*>(this);
}
//Channel
int UDPChannel::attach(SOCKET s,sockaddr_in& addr)
{
	//ע��:udp��attachֻ����������״̬��ûȷ��������ӵĽ��������Բ�Ҫon_connected()
	m_hip = ntohl(addr.sin_addr.s_addr);
	m_hport = ntohs(addr.sin_port);
	m_is_accept = true;
	m_state = CONNECTING;
	return 0;
}
int UDPChannel::connect(const char* ip,unsigned short port,int nattype/*=0*/)
{
	return connect(ntohl(inet_addr(ip)),port,nattype);
}

int UDPChannel::connect(unsigned int ip,unsigned short port,int nattype/*=0*/)
{
	//UDPĿǰ��֧���ڴ�ָ��bindip
	if(!m_ctr)
		return -1;
	if(0!=m_ctr->connect(static_cast<UDPChannelHandler*>(this),ip,port,nattype))
		return -1;
	m_des_nattype = nattype;
	m_hip = ip;
	m_hport = port;
	m_state = CONNECTING;
	fire(ChannelListener::Connecting(),this);
	return 0;
}

int UDPChannel::disconnect()
{
	if(CONNECTED == m_state)
	{
		//ȫ��δ�������ݷ�һ��,��������֤������ȫ����
		for(SendIter it=m_send_list.begin();it!=m_send_list.end();++it)
		{
			if((*it).send_count<1)
				send_to((*it).block->buffer,(*it).block->datasize,0);
		}
	}
	if(DISCONNECTED!=m_state)
	{
		m_ctr->disconnect(this);
		on_disconnected();
	}
	return 0;
}

int UDPChannel::send(memblock *b,bool more/*=false*/)  //-1:false; 0:send ok; 1:put int sendlist
{
	//���Ƿ��Ͷ���С��10��ʱ�����call ��д
	if(CONNECTED != m_state)
	{
		b->free(UAC_THREAD_CORE);
		return -1;
	}
	m_last_active_tick = GetTickCount();

	UDPSSendPacket_t sp;
	UDPSConnHeader_t& head=_tmp_head;
	UDPSData_t& data=_tmp_send_data;

	head.cmd = UDPS_CONN_CMD_DATA;
	head.des_sessionid = get_des_sessionid();
	head.src_sessionid = get_src_sessionid();
	_tmp_cmd = UDPS_CHANNEL_CMD_DATA;
	data.mask = more?0:1;
	data.sequence_num = m_send_win.send_num;
	data.buflen = (b->datasize - b->datapos);
	_tmp_ps.attach(b->buffer,UDPHEAD_LENGTH);
	_tmp_ps << head;
	_tmp_ps << _tmp_cmd;
	_tmp_ps << data;
	sp.block = b;
	sp.send_count = 0;
	sp.send_num = m_send_win.send_num++;
	m_send_list.push_back(sp);

	if(!m_bsend_timer && !m_send_list.empty())
	{
#ifdef _WIN32
		TimerSngl::instance()->register_timer(this,TIMER_SEND,2);
#else
		TimerSngl::instance()->register_timer(this,TIMER_SEND,1);
#endif
		m_bsend_timer = true;
	}

	//���Ͷ��г������ʹ�����Ϊ������������1��ʾ��Щ�������´��ֻ��ٷ�
	if(m_send_list.size()>m_send_list_max_size)
		return 1;
	return 0;
}


//UDPChannelHandler
int UDPChannel::handle_connected()
{
	on_connected();
	return 0;
}

int UDPChannel::handle_disconnected()
{
	return disconnect();
}
int UDPChannel::send_to(char *buf,int len,ULONGLONG utick)
{
	if(buf[UDPS_CONN_HEAD_LENGTH] == UDPS_CHANNEL_CMD_DATA)
	{
		//�����ٶȱ��
		UDPHEAD_UPDATE_SPEED_SEQ(buf,m_send_win.spctrl.csp.speed_seq);
		m_send_win.spctrl.csp.on_send(utick,len);
		m_send_win.timer_bwwin_send_sizeB += len;
	}
	return UDPChannelHandler::send_to(buf,len,utick);
}
void UDPChannel::on_timer(int e)
{
	switch(e)
	{
	case TIMER_SEND:
		{
			handle_send(); //�����Ѿ�disconnect(),����ִ����ע���ʱ��
			if(m_bsend_timer && m_send_list.empty())
			{
				TimerSngl::instance()->unregister_timer(this,TIMER_SEND);
				m_bsend_timer = false;
			}
		}
		break;
	case TIMER_ACK:
		handle_ack_timer();
		break;
	case TIMER_BWWIN:
		handle_bwwin_timer();
		break;
	case 10:
		break;
	default:
		assert(false);
		break;
	}
}

int UDPChannel::handle_send(bool roolcall/*=false*/)
{
	if(m_send_list.size()<m_send_list_max_size)
	{
		unsigned int num = m_send_list.size();
		on_writable();
		if(m_send_list.size()<=num)
		{
			m_send_win.not_more_data_count++;
			//�˴������ͻ������ݲ����Ӱ���ٶ����
			//UACLOG(" *no enoght (send list=%d / %d) \n",m_send_list.size(),m_send_list_max_size);
		}
		else
			m_send_win.not_more_data_count = 0;
	}
	if(m_send_list.empty())
		return 0;
	
	unsigned int n = 0,sn=0,rn=0;
	ULONGLONG utick = GetUTickCount();
	unsigned int max_count = m_send_win.spctrl.get_max_send_packnum(utick);
	int win = m_send_win.spctrl.get_win();

//#define ADDR_ROOT 0x0100007f
//	if(this->m_addr.sin_addr.s_addr == ADDR_ROOT)
//		max_count=1000;

	UDPSSendPacket_t *pack = NULL;
	bool bsend;
	for(SendIter it=m_send_list.begin();it!=m_send_list.end()&&n<max_count;++it)
	{
		pack = &(*it);
		if(pack->send_num>=m_send_win.win_low_line + win)
			break;
		if(pack->send_count<1)
		{
			//�������Ͳ��޴�
			//�����Ƿ���Է�����,ÿ16������һ�ΰ���,0�������
			send_to(pack->block->buffer,pack->block->datasize,utick);
			m_send_win.send_sizeB+=(pack->block->datasize - UDPHEAD_LENGTH);
			pack->last_send_tick = utick;
			pack->send_count++;
			n++;sn++;
			m_send_win.real_send_num = pack->send_num;
		}
		else if(pack->send_count<g_udps_conf.max_resend_num)
		{
			bsend = false;
			if(pack->nak_count>1 && 1==pack->send_count)
			{
				//δ�ط����İ�,ֻҪnak_count>1�����̻ظ�
				//UACLOG("a%d.",pack->send_count);
				UDPHEAD_UPDATE_MASK_SET_REPEAT_NAK(pack->block->buffer);
				bsend = true;
				//ת���ٿ��㷨
				if(LEVEL_WIN==m_send_win.spctrl.level)
					UACLOG("# into speed control! \n");
				m_send_win.spctrl.level = LEVEL_SPEED;
				
			}
			else
			{
				if(pack->nak_count>0)
				{
					//Ԥ�ƿ��ܶ���
					if((pack->last_send_tick + m_send_win.ttl+50000) < utick)
					{
						//UACLOG("n%d.",pack->send_count);
						UDPHEAD_UPDATE_MASK_SET_REPEAT_NAK(pack->block->buffer);
						bsend = true;
					}
				}
				else
				{
					//��ʱ�ط�
					if((pack->last_send_tick + 2*m_send_win.ttl+100000) < utick)
					{
						//UACLOG("t%d.",pack->send_count);
						UDPHEAD_UPDATE_MASK_SET_REPEAT_TIMEOUT(pack->block->buffer);
						bsend = true;
						m_send_win.resend_timeo_num++;
					}
				}
			}
			if(bsend)
			{
				send_to(pack->block->buffer,pack->block->datasize,utick);
				m_send_win.timer_bwwin_resend_sizeB+=(pack->block->datasize - UDPHEAD_LENGTH);
				pack->last_send_tick = utick;
				pack->send_count++;
				pack->nak_count=0;
				n++;rn++;
				m_send_win.resend_num++;
				m_send_win.timer_bwwin_resend_sizeB+=(pack->block->datasize - UDPHEAD_LENGTH);
			}
		}
		else
		{
			UACLOG("#**** udp resend timeout and disconnect (%d times)---\n",(int)g_udps_conf.max_resend_num);
			return disconnect();
		}
	}

	//if(max_count>0)
	//	UACLOG("<%d,%d,%d>",max_count,sn,rn);
	//else
	//	UACLOG(".");

	//��������ִ��on_tick�������������÷������ڣ��������utickʵʱ��û��utick����ᵼ�·����ٶȵ�΢С��ʧ	
	m_send_win.spctrl.csp.on_tick(utick);
	if(!roolcall)
	{
		
		if(n<max_count)
		{
			//����ĩ���㹻���ݷ��ͣ����ǼӴ����
			//UACLOG("# *** n<max_couunt; %d < %d  ls_size=%d \n",n,max_count,m_send_list.size());
			handle_send(true); //����һ��ִ�з��ͣ��Ծ�����֤
		}
	}
	else
	{
		//UACLOG("# ���뷢�� n - max_couunt; %d = %d  ls_size=%d \n",n,max_count,m_send_list.size());
	}
	
	return 0;
}

int UDPChannel::handle_recv(memblock* b)
{
	m_last_active_tick = GetTickCount();
	if(m_state != CONNECTED)
	{
		assert(false);
		b->free(UAC_THREAD_CORE);
		return 0;
	}

	_tmp_recv_ps.attach(b->buffer + b->datapos,b->datasize - b->datapos,b->datasize - b->datapos);
	while(_tmp_recv_ps.length()>0)
	{
		if(0!=_tmp_recv_ps>>_tmp_recv_cmd)
		{
			assert(false);
			UACLOG("#*** UDPChannel::handle_recv() wrong packet!!! \n");
			return -1;
		}
		if(UDPS_CHANNEL_CMD_DATA == _tmp_recv_cmd)
		{
			handle_recv_data(_tmp_recv_ps,b);
			b = NULL;
			return 0;
		}
		else if(UDPS_CHANNEL_CMD_ACK == _tmp_recv_cmd)
			handle_recv_ack(_tmp_recv_ps);
		else
		{
			UACLOG("# *** ERROR UDPChannel cmd(%d) *** !\n",_tmp_recv_cmd);
			assert(false);
			break;
		}
	}
	if(b) b->free(UAC_THREAD_CORE);
	return 0;
}


int UDPChannel::handle_recv_data(PTLStream& ps,memblock* b)
{
	//��4�����飺����ttl�����������պš����Ը��´�����ơ�������հ����С������½硢�ظ�ȷ�ϰ�
	//UACLOG("#..udp recv data num = %d \n",head.sequence_num);

	if(0!=ps>>_tmp_recv_data)
	{
		assert(false);
		UACLOG("#*** UDPChannel::handle_recv_data() wrong packet!!! \n");
		b->free(UAC_THREAD_CORE);
		return -1;
	}
	DWORD tick = GetTickCount();
	ULONGLONG curr_utick = GetUTickCount();
	
	m_recv_win.unack_recv_num++;

	assert(0==ps.length());
	b->datapos = UDPHEAD_LENGTH;
	m_recv_win.speed.add(b->datasize); 
	m_recv_win.csp.on_recv(curr_utick,_tmp_recv_data.speed_seq,b->datasize);


	m_recv_win.last_pack.utick = curr_utick;
	m_recv_win.last_pack.seq = _tmp_recv_data.sequence_num;
	

	if(m_recv_win.max_sequence_num < (int)_tmp_recv_data.sequence_num)
		m_recv_win.max_sequence_num = (int)_tmp_recv_data.sequence_num;
	

	
	//2�ж��Ƿ��Ǵ��ڱ߽����ݣ��ǣ��ϱ�����ִ�ж��еļ�飬�������һ���ϱ������Ƿ�ظ��� �񣺲����ظ���������,��¼�Ƿ���Ҫ�����ظ�
	if(m_recv_win.win_low_line == (int)_tmp_recv_data.sequence_num)
	{
		m_recv_win.recv_sizeB+=_tmp_recv_data.buflen;
		on_data(b);
		if(CONNECTED!=m_state) return -1;
		m_recv_win.win_low_line++;

		//ִ��һ�ζ��в�ѯ��
		for(RecvIter it=m_recv_map.begin();it!=m_recv_map.end();)
		{
			if(it->second.recv_num != m_recv_win.win_low_line)
			{
				assert(it->second.recv_num > m_recv_win.win_low_line);
				break;
			}
			on_data(it->second.block);
			if(CONNECTED!=m_state) return -1;
			m_recv_win.win_low_line++;
			m_recv_map.erase(it++);
		}
	}
	else if((int)_tmp_recv_data.sequence_num > m_recv_win.win_low_line)
	{
		RecvIter it = m_recv_map.find(_tmp_recv_data.sequence_num);
		if(it == m_recv_map.end())
		{
			m_recv_win.recv_sizeB+=_tmp_recv_data.buflen;
			UDPSRecvPacket_t rp;
			rp.block = b;
			rp.recv_num = _tmp_recv_data.sequence_num;
			rp.recv_utick = curr_utick;
			m_recv_map[rp.recv_num] = rp;
		}
		else
		{
			UACLOG("#......................udp rr<<seq:%d>>..\n",_tmp_recv_data.sequence_num);
			it->second.recv_utick = curr_utick; //���½���tick
			it->second.ack_count = 0; //����,���ܻظ�ACK�Ѿ��������¶Է��ط�.
			m_recv_win.rerecv_num++;
			b->free(UAC_THREAD_CORE);
		}
	}
	else
	{
		//�����ݰ�,��ʱ
		UACLOG("#......................udp rr old<<seq:%d>>..\n",_tmp_recv_data.sequence_num);
		m_recv_win.rerecv_num++;
		b->free(UAC_THREAD_CORE);
	}

	//3�ж��Ƿ������ظ����߽������Ѿ���������ظ�������ȴ����������ٻظ���������ɷ��ͷ��ѻ�������������
	if(m_recv_win.unack_recv_num >= 10/* || (_tmp_recv_data.mask&NEED_ACK)*/)
	{
		send_ack(tick,curr_utick);
	}
	return 0;
}

int UDPChannel::handle_recv_ack(PTLStream& ps)
{
	//��4�����飺���·����½硢�����Ͷ��С�����ttl�����·��ʹ�����
	UDPSAck_t& ack = _tmp_ack;
	if(0!=ps>>_tmp_ack)
	{
		assert(false);
		UACLOG("#*** UDPChannel::handle_recv_ack() wrong packet!!! \n");
		return -1;
	}
	//UACLOG("#ack{%d->%d}\n",ack.size,ack.lowline_num);
	
	ULONGLONG utick = GetUTickCount();
	unsigned int tick = GetTickCount();

	int low_line = (int)ack.lowline_num;
	m_send_win.spctrl.on_recv_ack(tick);

	if(ack.ack_sequence<m_send_win.ack_sequence)
	{
		//UACLOG("# *** old ack_sequence -- %d \n",ack.ack_sequence);
	}
	else
	{
		if(ack.ack_sequence>m_send_win.ack_sequence)
		{
			//UACLOG("#***ack lost n=%d (%d) \n",ack.ack_sequence-m_send_win.ack_sequence,ack.ack_sequence);
		}
		m_send_win.ack_sequence = ack.ack_sequence+1;
		m_send_win.spctrl.csp.on_ack_speed(ack.csp_speed_seq,ack.csp_num,ack.csp_speedB);
	}
	
	//�Է�Ҫ�������߶�������
	if(ack.const_send_speedKB)
		m_send_win.spctrl.csp.const_speedB = ((unsigned int)ack.const_send_speedKB)<<10;
	if(ack.const_send_lose_rate)
		m_send_win.spctrl.csp.max_lose_rate = ack.const_send_lose_rate;

	//������߽�,=��ʱ��Ҳ����,��Ϊother_recv_win_num���ܸ���
	if(m_send_win.win_low_line<=low_line)
	{
		m_send_win.win_low_line = low_line;
		m_send_win.spctrl.other_recv_win_num = ack.recv_win_num;
		m_send_list_max_size = m_send_win.spctrl.other_recv_win_num + m_send_win.spctrl.other_recv_win_num/2 + 20;
	}


	if(ack.size>0)
	{
		SendIter it=m_send_list.begin();
		for(int i=0;i<ack.size;++i)
		{
			for(; it!=m_send_list.end(); )
			{
				if((*it).send_num<(int)ack.seq_nums[i])
				{
					//��ʧ��ʱ
					(*it).nak_count++;
					++it;
				}
				else if((*it).send_num==(int)ack.seq_nums[i])
				{
					if(1==(*it).send_count)
					{
						unsigned int ttl = (unsigned int)(utick-(*it).last_send_tick - ack.wait_us[i]);
						//����10�����Ϊ��ЧTTL
						if(ttl<10000000)
						{
							m_send_win.ttls.step(ttl);
							if(m_send_win.maxttl<ttl) 
								m_send_win.maxttl = ttl;

						}
					}
					
					//ȥ����Ӧ�Ű�
					(*it).block->free(UAC_THREAD_CORE);
					m_send_list.erase(it++);
				}
				else
				{
					//
					break;
				}
			}
		}
	}
	
	//�ȼ�ttl,��ȥ����ʱ��
	for(SendIter it=m_send_list.begin();it!=m_send_list.end()&&(*it).send_num<low_line;)
	{
		(*it).block->free(UAC_THREAD_CORE);
		m_send_list.erase(it++);
	}

	//
	m_send_win.ttl = m_send_win.ttls.get_rate(3);
	if(m_send_win.ttl==0) 
		m_send_win.ttl=200000;
	m_send_win.spctrl.ttlus = m_send_win.ttl;

	return 0;
}

void UDPChannel::send_ack(DWORD tick,ULONGLONG utick)
{
	_tmp_ps.attach(_tmp_cmdbuf,1024);
	UDPSConnHeader_t& head = _tmp_head;
	head.cmd = UDPS_CONN_CMD_DATA;
	head.des_sessionid = this->get_des_sessionid();
	head.src_sessionid = this->get_src_sessionid();
	_tmp_ps << head;

	m_recv_win.last_ack_tick = tick;
	m_recv_win.unack_recv_num = 0;

	//Ӧ��ack���ܶ��������,ÿ�λظ�ACK���ظ�����,ͬʱ���ٻظ�NAK
	int i = 0;
	for(RecvIter it=m_recv_map.begin();it!=m_recv_map.end() && i<ACK_ARR_LEN;++it)
	{
		if(it->second.ack_count<3)
		{
			//ÿ��ACK�ظ�3��
			it->second.ack_count++;
			m_recv_win.ack.seq_nums[i] = it->first;
			m_recv_win.ack.wait_us[i] = (unsigned int)(utick - it->second.recv_utick);
			++i;
		}
	}
	if(0==i && m_recv_win.last_pack.seq)
	{
		//�ظ����һ��pack��Ϣ�ṩ���ͷ���ttl
		m_recv_win.ack.seq_nums[0] = m_recv_win.last_pack.seq;
		m_recv_win.ack.wait_us[0] = (unsigned int)(utick - m_recv_win.last_pack.utick);
		i = 1;
	}
	m_recv_win.ack.size = i;
	

	{
		//����ظ�����:
		if(m_recv_win.last_ack_win_low_line == m_recv_win.win_low_line && 0==m_recv_win.ack.size)
			m_recv_win.resend_ack_win_low_line_count++;
		else
		{
			m_recv_win.resend_ack_win_low_line_count = 0;
		}
		m_recv_win.last_ack_win_low_line=m_recv_win.win_low_line;
		m_recv_win.brecv_win_num_changed = false; //�ظ���֮��ͱ�Ϊfalse

		_tmp_cmd = UDPS_CHANNEL_CMD_ACK;
		m_recv_win.ack.lowline_num = m_recv_win.win_low_line;
		m_recv_win.ack.recv_win_num = m_recv_win.recv_win_num;
		m_recv_win.ack.ack_sequence = m_recv_win.ack_sequence++;
		m_recv_win.ack.rerecv_num = m_recv_win.rerecv_num;
		m_recv_win.ack.csp_speed_seq = m_recv_win.csp.last_speed_seq;
		m_recv_win.ack.csp_num = m_recv_win.csp.last_num;
		m_recv_win.ack.csp_speedB =  m_recv_win.csp.last_speedB;
		m_recv_win.ack.const_send_speedKB = g_udps_conf.const_recv_speedB>>10;
		m_recv_win.ack.const_send_lose_rate = g_udps_conf.const_recv_lose_rate;

		_tmp_ps<<_tmp_cmd;
		_tmp_ps<<m_recv_win.ack;

		m_recv_win.ack.size = 0;
	}

	send_to(_tmp_cmdbuf,_tmp_ps.length(),utick);
}
void UDPChannel::handle_ack_timer()
{
	ULONGLONG utick = GetUTickCount();
	unsigned int tick = GetTickCount();
	
	unsigned time_out = g_udps_conf.udp_nak_timer_msec*(m_recv_win.resend_ack_win_low_line_count+1)-5;
	if(_timer_after(tick,m_recv_win.last_ack_tick+time_out) && m_recv_win.resend_ack_win_low_line_count<20)
		send_ack(tick,utick);

}
void UDPChannel::handle_bwwin_timer()
{
	m_recv_win.speed.on_second();
	//m_send_win.spctrl.on_second();
	if(m_send_win.timer_bwwin_send_sizeB>0)
	{
		//UACLOG("# ttl=%d,low_line=%d,send_win/win=%d / %d \n",
		//	m_send_win.ttl/1000,
		//	m_send_win.win_low_line,
		//	m_send_win.spctrl.other_recv_win_num,
		//	m_send_win.spctrl.send_win
		//	);
	}

	m_send_win.timer_bwwin_send_sizeB = 0;
	m_send_win.timer_bwwin_resend_sizeB = 0;
	m_send_win.maxttl = 0;

}

void UDPChannel::update_recv_win_num(uint32 cache_block_num)
{
	m_recv_win.recv_win_num = m_recv_win.max_recv_win_num-cache_block_num;
	m_recv_win.brecv_win_num_changed = true;
	//���ڷ����ı�ʱҲ������0,ȷ�����Զ�λظ����ڱ��
	m_recv_win.resend_ack_win_low_line_count = 0; 
}

}

