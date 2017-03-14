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
	//执行最后一次处理完队列命令
	handle_root(0);
	assert(0==m_socknum);
	TimerSngl::instance()->unregister_all(static_cast<TimerHandler*>(this));
	if(m_socks)
	{
		for(int i=0;i<UAC_FD_SIZE;++i)
		{
			//要关掉所有SOCK_ACCEPTING的连接
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

//UAC socket 接口
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
				m_socks[fd].sock_state = SOCK_CONNECTED; //accepting 时可能已经有数据，底层只有将连接建完成才会变成accepting
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
	//不会死循环，
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
		m_ls_connecting.push_back(fd); //传递一个连接消息
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
	//底层主动断开的话，不发消息通知上层，只将状态变为disconnecting
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
		//只要有数据读或者底层断开都为真
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
		//只要有数据写都为真，不管状态
		//被底层断开时，也设为可写，让上层写失败时关闭，因为未连接成功时只查可写状态，不查可读状态
		if((SOCK_CONNECTED==m_socks[fd].sock_state && (m_socks[fd].sendlist.size()*m_socks[fd].ch->mtu())<m_socks[fd].max_sendbufsize) || SOCK_DISCONNECTING==m_socks[fd].sock_state)
			return true;
	}
	return false;
}
int sockpool::select(UAC_fd_set* rset,UAC_fd_set* wset)
{
	//这里总是查询所有SOCK的状态返回
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
			m_ls_sending.push_back(fd); //通知底层发送数据
		}
		//分包
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
	//-1 表示可关闭
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
		//正常来说: 应用层如果关闭了则不会再调用recv()
		//若底层即使已经关闭，但应用层未关，此时有数据仍然照收。收到没数据时
		//再判断是否已经断开，会更理想一点。
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
		(*it)->free(ithreadtoken); //上层线程调用队列UAC_THREAD_APP，底层用1
	}
	ls.clear();
}
int sockpool::find_idel_socket()
{
	//非线程安全，即找到的idel节点在使用时再次确定是否为idel，否则重复找
	//每次总是从头找，使connector循环次数尽量小
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
	//处理上层的连接，断开，数据发送等
	list<UAC_SOCKET> ls;
	list<UAC_SOCKET>::iterator it;
	int fd;
	//1。发送
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

	//2.连接
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

	//2.断开
	if(!m_ls_disconnecting.empty())
	{
		m_mt.lock();
		m_ls_disconnecting.swap(ls);
		m_mt.unlock();

		for(it=ls.begin();it!=ls.end();++it)
		{
			fd = *it;
			SLock l(m_socks[fd].mt);
			//可能上层底层都已经关闭
			//(上层关闭时底层未关时增加了引命令，未执行到此时底层有关了的情况是可能的)
			//这种情况仍然执行一次关闭(可能fd已经成为新的连接错关，但可能性极低。)
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
		m_socks[idx].ch->attach(fd,addr); //一定成功
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
	//不会死循环，
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
	//不用检查状态是否为连接状态，ch->send会检查
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
	SLock l(m_socks[fd].mt); //这里会导致锁的重入
	if(SOCK_IDLE!=m_socks[fd].sock_state)
	{
		m_socks[fd].close_state |= 0x02;
		//SOCK_WAIT_ACCEPTING,SOCK_ACCEPTING时表示上层还没接收连接,没有上层操作
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
		//SOCK_ACCEPTING 表示应用层未执行accept(),即未产生上层控制
		if((SOCK_ACCEPTING==m_socks[fd].sock_state||SOCK_CONNECTED==m_socks[fd].sock_state))
		{
			assert(m_socks[fd].recvlist.size() <= g_udps_conf.max_recv_win_num);
			m_socks[fd].recvlist.push_back(b);
		}
		else
		{
			assert(SOCK_IDLE==m_socks[fd].sock_state || SOCK_DISCONNECTING==m_socks[fd].sock_state);
			b->free(UAC_THREAD_CORE);
			//在此不要执行disconnect,可能上层刚执行完，m_ls_disconnecting 里有此命令未执行到
		}
	}
	
}
void sockpool::on(ChannelListener::Writable,Channel* ch)
{
	_handle_send(static_cast<UDPChannel*>(ch)->idx());
}

} //namespace UAC

