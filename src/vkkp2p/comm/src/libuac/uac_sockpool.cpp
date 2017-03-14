#include "uac_sockpool.h"
#include <assert.h>

#include "uac_UDPProtocol.h"

namespace UAC
{

#define MAXSOCK_CLR(s,ms) if(s==ms) {ms--;while(ms>-1 && SOCK_IDLE==m_socks[ms].sock_state) ms--;}

//****************************************************************
sockpool::sockpool(void)
:m_socks(NULL)
,m_acceptor(NULL)
,m_socknum(0)
,m_maxsock(-1)
{
}
sockpool::~sockpool(void)
{
}

int sockpool::init(unsigned short port,const char* stunsvr,unsigned short stunport)
{
	if(m_acceptor)
	{
		assert(false);
		return 1;
	}
	m_acceptor = new UDPAcceptor();
	if(0!=m_acceptor->open(port,stunsvr,stunport,static_cast<UDPChannelFactory*>(this)))
	{
		delete m_acceptor;
		m_acceptor = NULL;
		return -1;
	}

	m_socks = new UACSocket_t[UAC_FD_SIZE];
	for(int i=0;i<UAC_FD_SIZE;++i)
	{
		m_socks[i].ch = new UDPChannel(m_acceptor->get_connector(),i);
		m_socks[i].ch->add_listener(static_cast<ChannelListener*>(this));
	}
	//TimerSngl::instance()->register_timer(static_cast<TimerHandler*>(this),1,2000);
	return 0;
}

void sockpool::fini()
{
	//ִ�����һ�δ������������
	handle_root(0);
	assert(0==m_socknum);
	TimerSngl::instance()->unregister_all(static_cast<TimerHandler*>(this));
	if(m_socks)
	{
		for(int i=0;i<UAC_FD_SIZE;++i)
		{
			//Ҫ�ص�����SOCK_ACCEPTING������
			if(SOCK_WAIT_ACCEPTING==m_socks[i].sock_state || SOCK_ACCEPTING==m_socks[i].sock_state)
				m_socks[i].ch->disconnect();
			m_socks[i].ch->remove_listener(static_cast<ChannelListener*>(this));
			delete m_socks[i].ch;
		}
		delete[] m_socks;
		m_socks = NULL;
	}
	if(m_acceptor)
	{
		m_acceptor->close();
		delete m_acceptor;
		m_acceptor = NULL;
	}
}
void sockpool::on_timer(int e)
{
	switch(e)
	{
	case 1:
		{
			UACLOG("# socknum(%d,%d) \n",m_socknum,m_acceptor->get_connector()->get_socknum());
		}
		break;
	default:
		assert(0);
		break;
	}
}

//UAC socket �ӿ�
UAC_SOCKET sockpool::accept(UAC_sockaddr* sa_client)
{
	list<UAC_SOCKET> ls;
	UAC_SOCKET fd = -1;
	if(!m_ls_accepting.empty())
	{
		{
			SLock l(m_mt);
			if(!m_ls_accepting.empty())
			{
				fd = m_ls_accepting.front();
				m_ls_accepting.pop_front();
			}
			else
				return -1;
		}
		{
			SLock l(m_socks[fd].mt);
			if(SOCK_ACCEPTING==m_socks[fd].sock_state)
			{
				m_socks[fd].sock_state = SOCK_CONNECTED; //accepting ʱ�����Ѿ������ݣ��ײ�ֻ�н����ӽ���ɲŻ���accepting
				memcpy(sa_client,&m_socks[fd].addr,sizeof(UAC_sockaddr));
				return fd;
			}
			else
			{
				assert(0);
				return -1;
			}
		}
	}
	return -1;
}
UAC_SOCKET sockpool::connect(const UAC_sockaddr* sa_client)
{
	UAC_SOCKET fd = -1;
	//������ѭ����
	while(1)
	{
		fd = find_idel_socket();
		if(-1==fd)
		{
			return -1;
		}
		else
		{
			SLock l(m_socks[fd].mt);
			if(SOCK_IDLE!=m_socks[fd].sock_state || DISCONNECTED!=m_socks[fd].ch->get_state())
				continue;
			m_socks[fd].sock_state = SOCK_WAIT_CONNECTING;
			memcpy(&m_socks[fd].addr,sa_client,sizeof(UAC_sockaddr));
			m_socks[fd].last_active_tick = GetTickCount();
			break;
		}
	}
	{
		SLock l(m_mt);
		m_socknum++;
		m_ls_connecting.push_back(fd); //����һ��������Ϣ
	}
	return fd;
}
void sockpool::release_fd(UAC_SOCKET fd)
{
	m_socks[fd].sock_state = SOCK_IDLE;
	MAXSOCK_CLR(fd,m_maxsock);
	m_socks[fd].close_state = 0;
	m_socks[fd].last_active_tick = GetTickCount();
	clear_memblock_list(m_socks[fd].sendlist,UAC_THREAD_APP);
	clear_memblock_list(m_socks[fd].recvlist,UAC_THREAD_APP);
	m_socks[fd].max_sendbufsize = UAC_SOCK_SENDBUF;
	m_socks[fd].max_recvbufsize = UAC_SOCK_RECVBUF;
	{
		SLock ll(m_mt);
		m_socknum--;
	}
}
int sockpool::closesocket(UAC_SOCKET fd)
{
	//�ײ������Ͽ��Ļ���������Ϣ֪ͨ�ϲ㣬ֻ��״̬��Ϊdisconnecting
	if(fd<0||fd>=UAC_FD_SIZE)
	{
		assert(0);
		return -1;
	}
	else
	{
		//assert(SOCK_IDLE!=m_socks[fd].sock_state);
		SLock l(m_socks[fd].mt);
		if(SOCK_IDLE==m_socks[fd].sock_state)
			return 0;
		m_socks[fd].sock_state = SOCK_DISCONNECTING;
		m_socks[fd].close_state |= 0x01;
		if(m_socks[fd].close_state==0x03)
		{
			release_fd(fd);
		}
		else
		{
			SLock l(m_mt);
			m_ls_disconnecting.push_back(fd);
		}
	}
	return 0;
}
bool sockpool::is_read(UAC_SOCKET fd)
{
	if(fd<0||fd>=UAC_FD_SIZE)
	{
		assert(0);
		return false;
	}
	else
	{
		
		//SLock l(m_socks[fd].mt);
		//ֻҪ�����ݶ����ߵײ�Ͽ���Ϊ��
		if(!m_socks[fd].recvlist.empty() || SOCK_DISCONNECTING==m_socks[fd].sock_state)
			return true;
	}
	return false;
}
bool sockpool::is_write(UAC_SOCKET fd)
{
	if(fd<0||fd>=UAC_FD_SIZE)
	{
		assert(0);
		return false;
	}
	else
	{
		//SLock l(m_socks[fd].mt);
		//ֻҪ������д��Ϊ�棬����״̬
		//���ײ�Ͽ�ʱ��Ҳ��Ϊ��д�����ϲ�дʧ��ʱ�رգ���Ϊδ���ӳɹ�ʱֻ���д״̬������ɶ�״̬
		if((SOCK_CONNECTED==m_socks[fd].sock_state && (m_socks[fd].sendlist.size()*m_socks[fd].ch->mtu())<m_socks[fd].max_sendbufsize) || SOCK_DISCONNECTING==m_socks[fd].sock_state)
			return true;
	}
	return false;
}
int sockpool::select(UAC_fd_set* rset,UAC_fd_set* wset)
{
	//�������ǲ�ѯ����SOCK��״̬����
	int n = 0;
	if(rset) rset->fd_count = 0;
	if(wset) wset->fd_count = 0;
	for(int i=0;i<(m_maxsock+1);++i)
	{
		if(rset)
		{
			if(!m_socks[i].recvlist.empty() || SOCK_DISCONNECTING==m_socks[i].sock_state)
			{
				rset->fd_array[rset->fd_count++] = i;
				n++;
			}
		}
		if(wset)
		{
			if((SOCK_CONNECTED==m_socks[i].sock_state && (m_socks[i].sendlist.size()*m_socks[i].ch->mtu())<m_socks[i].max_sendbufsize) || SOCK_DISCONNECTING==m_socks[i].sock_state)
			{
				wset->fd_array[wset->fd_count++] = i;
				n++;
			}
		}
	}
	return n;
}
int sockpool::send(UAC_SOCKET fd,const char* buf,int len)
{
	if(fd<0||fd>=UAC_FD_SIZE)
	{
		assert(0);
		return -1;
	}
	else
	{
		SLock l(m_socks[fd].mt);
		if(SOCK_CONNECTED!=m_socks[fd].sock_state)
			return -1;

		if(m_socks[fd].sendlist.empty())
		{
			SLock ll(m_mt);
			m_ls_sending.push_back(fd); //֪ͨ�ײ㷢������
		}
		//�ְ�
		int packsize;
		memblock* block;
		int sendsize = 0;
		while(len>0)
		{
			packsize = m_socks[fd].ch->mtu();
			if(packsize>len)
				packsize = len;
			block = memblock::alloc(UAC_THREAD_APP);
			if(NULL==block) return sendsize;
			block->datapos = UDPHEAD_LENGTH;
			block->datasize = block->datapos + packsize;
			memcpy(block->buffer+block->datapos,buf+sendsize,packsize);
			sendsize += packsize;
			len -= packsize;
			m_socks[fd].sendlist.push_back(block);
		}
		return sendsize;
	}
}
int sockpool::recv(UAC_SOCKET fd,char* buf,int len)
{
	//-1 ��ʾ�ɹر�
	if(fd<0||fd>=UAC_FD_SIZE)
	{
		assert(0);
		return -1;
	}
	else
	{
		if(len<=0) return 0;
		SLock l(m_socks[fd].mt);
		int recvsize = 0;
		int cpsize;
		memblock* block;
		assert(0==(m_socks[fd].close_state&0x01));
		if(m_socks[fd].close_state&0x01) return -1;
		//������˵: Ӧ�ò�����ر����򲻻��ٵ���recv()
		//���ײ㼴ʹ�Ѿ��رգ���Ӧ�ò�δ�أ���ʱ��������Ȼ���ա��յ�û����ʱ
		//���ж��Ƿ��Ѿ��Ͽ����������һ�㡣
		while(len>0 && !m_socks[fd].recvlist.empty())
		{
			block = m_socks[fd].recvlist.front();
			cpsize = block->datasize - block->datapos;
			if(cpsize>len) cpsize = len;
			memcpy(buf+recvsize,block->buffer+block->datapos,cpsize);
			block->datapos += cpsize;
			len -= cpsize;
			recvsize += cpsize;
			if(block->datasize == block->datapos)
			{
				m_socks[fd].recvlist.pop_front();
				block->free(UAC_THREAD_APP);
			}
		}
		if(0==recvsize && SOCK_CONNECTED!=m_socks[fd].sock_state)
			return -1;

		m_socks[fd].ch->update_recv_win_num(m_socks[fd].recvlist.size());
		return recvsize;
	}
}

void sockpool::clear_memblock_list(list<memblock*>& ls,int ithreadtoken)
{
	for(list<memblock*>::iterator it=ls.begin();it!=ls.end();++it)
	{
		(*it)->free(ithreadtoken); //�ϲ��̵߳��ö���UAC_THREAD_APP���ײ���1
	}
	ls.clear();
}
int sockpool::find_idel_socket()
{
	//���̰߳�ȫ�����ҵ���idel�ڵ���ʹ��ʱ�ٴ�ȷ���Ƿ�Ϊidel�������ظ���
	//ÿ�����Ǵ�ͷ�ң�ʹconnectorѭ����������С
	DWORD tick = GetTickCount();
	for(int i=0;i<UAC_FD_SIZE;++i)
	{
		if(SOCK_IDLE==m_socks[i].sock_state && m_socks[i].last_active_tick + UAC_SOCK_IDEL_TICK<tick && DISCONNECTED==m_socks[i].ch->get_state())
		{
			if(m_maxsock<i) m_maxsock = i;
			return i;
		}
	}
	return -1;
}

void sockpool::handle_root(long delay_usec)
{
	m_acceptor->handle_select_read(delay_usec);
	//�����ϲ�����ӣ��Ͽ������ݷ��͵�
	list<UAC_SOCKET> ls;
	list<UAC_SOCKET>::iterator it;
	int fd;
	//1������
	if(!m_ls_sending.empty())
	{
		m_mt.lock();
		m_ls_sending.swap(ls);
		m_mt.unlock();

		for(it=ls.begin();it!=ls.end();++it)
		{
			_handle_send(*it);
		}
		ls.clear();
	}

	//2.����
	if(!m_ls_connecting.empty())
	{
		m_mt.lock();
		m_ls_connecting.swap(ls);
		m_mt.unlock();

		for(it=ls.begin();it!=ls.end();++it)
		{
			fd = *it;
			SLock l(m_socks[fd].mt);
			if(SOCK_WAIT_CONNECTING!=m_socks[fd].sock_state)
				continue;
			m_socks[fd].sock_state = SOCK_CONNECTING;
			if(0!=m_socks[fd].ch->connect(m_socks[fd].addr.ip,m_socks[fd].addr.port,m_socks[fd].addr.nattype))
			{
				assert(false);
				on(ChannelListener::Disconnected(),m_socks[fd].ch);
			}
		}
		ls.clear();
	}

	//2.�Ͽ�
	if(!m_ls_disconnecting.empty())
	{
		m_mt.lock();
		m_ls_disconnecting.swap(ls);
		m_mt.unlock();

		for(it=ls.begin();it!=ls.end();++it)
		{
			fd = *it;
			SLock l(m_socks[fd].mt);
			//�����ϲ�ײ㶼�Ѿ��ر�
			//(�ϲ�ر�ʱ�ײ�δ��ʱ�����������δִ�е���ʱ�ײ��й��˵�����ǿ��ܵ�)
			//���������Ȼִ��һ�ιر�(����fd�Ѿ���Ϊ�µ����Ӵ�أ��������Լ��͡�)
			m_socks[fd].ch->disconnect();
			
		}
		ls.clear();
	}
}
UDPChannelHandler* sockpool::attach_udp_channel(SOCKET fd,sockaddr_in& addr,UDPConnector* ctr)
{
	int idx = _try_accepting();
	if(-1==idx)
		return NULL;
	{
		SLock l(m_socks[idx].mt);
		m_socks[idx].ch->attach(fd,addr); //һ���ɹ�
		m_socks[idx].addr.ip = ntohl(addr.sin_addr.s_addr);
		m_socks[idx].addr.port = ntohs(addr.sin_port);
		m_socks[idx].addr.nattype = 6;
		{
			SLock ll(m_mt);
			m_socknum++;
		}
		return static_cast<UDPChannelHandler*>(m_socks[idx].ch);
	}
}
int sockpool::_try_accepting()
{
	UAC_SOCKET fd = -1;
	//������ѭ����
	while(1)
	{
		fd = find_idel_socket();
		if(-1==fd)
		{
			return -1;
		}
		else
		{
			SLock l(m_socks[fd].mt);
			if(SOCK_IDLE!=m_socks[fd].sock_state || DISCONNECTED!=m_socks[fd].ch->get_state())
				continue;
			m_socks[fd].sock_state = SOCK_WAIT_ACCEPTING;
			m_socks[fd].last_active_tick = GetTickCount();
			break;
		}
	}
	return fd;
}
void sockpool::_handle_send(UAC_SOCKET fd)
{
	memblock* b;
	SLock l(m_socks[fd].mt);
	//���ü��״̬�Ƿ�Ϊ����״̬��ch->send����
	while(!m_socks[fd].sendlist.empty())
	{
		b = m_socks[fd].sendlist.front();
		m_socks[fd].sendlist.pop_front();
		if(0!=m_socks[fd].ch->send(b,!m_socks[fd].sendlist.empty()))
			return;
	}
}

//------------------------------------------------------------
void sockpool::on(ChannelListener::Connecting,Channel* ch)
{
}
void sockpool::on(ChannelListener::Connected,Channel* ch)
{
	int fd = static_cast<UDPChannel*>(ch)->idx();
	SLock l(m_socks[fd].mt);
	if(SOCK_WAIT_ACCEPTING == m_socks[fd].sock_state)
	{
		m_socks[fd].sock_state = SOCK_ACCEPTING;
		SLock ll(m_mt);
		m_ls_accepting.push_back(fd);
	}
	else if(SOCK_CONNECTING == m_socks[fd].sock_state)
	{
		m_socks[fd].sock_state = SOCK_CONNECTED;
	}
	else
	{
		assert(SOCK_IDLE==m_socks[fd].sock_state);
		m_socks[fd].ch->disconnect();
	}
}
void sockpool::on(ChannelListener::Disconnected,Channel* ch)
{
	int fd = static_cast<UDPChannel*>(ch)->idx();
	SLock l(m_socks[fd].mt); //����ᵼ����������
	if(SOCK_IDLE!=m_socks[fd].sock_state)
	{
		m_socks[fd].close_state |= 0x02;
		//SOCK_WAIT_ACCEPTING,SOCK_ACCEPTINGʱ��ʾ�ϲ㻹û��������,û���ϲ����
		if(SOCK_WAIT_ACCEPTING==m_socks[fd].sock_state || SOCK_ACCEPTING==m_socks[fd].sock_state||m_socks[fd].close_state == 0x03)
			release_fd(fd);
		else
			m_socks[fd].sock_state = SOCK_DISCONNECTING;
	}
}
void sockpool::on(ChannelListener::Data,Channel* ch,memblock *b)
{
	int fd = static_cast<UDPChannel*>(ch)->idx();
	{
		SLock l(m_socks[fd].mt);
		//SOCK_ACCEPTING ��ʾӦ�ò�δִ��accept(),��δ�����ϲ����
		if((SOCK_ACCEPTING==m_socks[fd].sock_state||SOCK_CONNECTED==m_socks[fd].sock_state))
		{
			assert(m_socks[fd].recvlist.size() <= g_udps_conf.max_recv_win_num);
			m_socks[fd].recvlist.push_back(b);
		}
		else
		{
			assert(SOCK_IDLE==m_socks[fd].sock_state || SOCK_DISCONNECTING==m_socks[fd].sock_state);
			b->free(UAC_THREAD_CORE);
			//�ڴ˲�Ҫִ��disconnect,�����ϲ��ִ���꣬m_ls_disconnecting ���д�����δִ�е�
		}
	}
	
}
void sockpool::on(ChannelListener::Writable,Channel* ch)
{
	_handle_send(static_cast<UDPChannel*>(ch)->idx());
}

} //namespace UAC

