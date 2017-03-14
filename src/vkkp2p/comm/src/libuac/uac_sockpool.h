#pragma once
#include "uac_mempool.h"
#include "uac.h"
#include "uac_UDPAcceptor.h"
#include "uac_UDPChannel.h"


namespace UAC
{

typedef Recursive_Mutex SMutex;
typedef TLock<SMutex> SLock;
enum SOCKSTATE{ SOCK_IDLE=0,SOCK_WAIT_CONNECTING,SOCK_CONNECTING,SOCK_WAIT_ACCEPTING,SOCK_ACCEPTING,SOCK_CONNECTED,SOCK_DISCONNECTING};

//
#define UAC_SOCK_IDEL_TICK 60000
typedef struct tag_UACSocket
{
	SMutex mt;
	//注意: 只有上层改变
	int sock_state; //空闲，正在连接，正在接受，连接完成
	char close_state; //应用层和底层关闭
	UAC_sockaddr addr;
	UDPChannel* ch;
	unsigned int max_sendbufsize;
	unsigned int max_recvbufsize;
	list<memblock*> sendlist;
	list<memblock*> recvlist;
	DWORD last_active_tick;
	
	tag_UACSocket(void)
		:sock_state(SOCK_IDLE)
		,close_state(0)
		,max_sendbufsize(UAC_SOCK_SENDBUF)
		,max_recvbufsize(UAC_SOCK_RECVBUF)
		,last_active_tick((DWORD)-UAC_SOCK_IDEL_TICK)
	{
	}
}UACSocket_t;

//UAC_SOCKET fd 为sockpool中的索引
class sockpool  : public UDPChannelFactory,private ChannelListener 
	,public TimerHandler
{
	friend class Speaker<ChannelListener>;
public:
	sockpool(void);
	virtual ~sockpool(void);

public:
	int init(unsigned short port,const char* stunsvr,unsigned short stunport);
	void fini();
	virtual void on_timer(int e);

	//UAC socket 接口
	UAC_SOCKET accept(UAC_sockaddr* sa_client);
	UAC_SOCKET connect(const UAC_sockaddr* sa_client);
	int closesocket(UAC_SOCKET fd);
	int setsendbuf(UAC_SOCKET fd,int size) { if(size<10240)size=10240; m_socks[fd].max_sendbufsize = size;return 0;}
	int setrecvbuf(UAC_SOCKET fd,int size) { if(size<10240)size=10240; m_socks[fd].max_recvbufsize = size;return 0;}
	bool is_read(UAC_SOCKET fd);
	bool is_write(UAC_SOCKET fd);
	int select(UAC_fd_set* rset,UAC_fd_set* wset);
	int send(UAC_SOCKET fd,const char* buf,int len);
	int recv(UAC_SOCKET fd,char* buf,int len);

public:
	static void clear_memblock_list(list<memblock*>& ls,int ithreadtoken);
	int find_idel_socket();

	//被底层调用接口
	void handle_root(long delay_usec);
	virtual UDPChannelHandler* attach_udp_channel(SOCKET fd,sockaddr_in& addr,UDPConnector* ctr);
private:
	int _try_accepting();
	void _handle_send(UAC_SOCKET fd);
	void release_fd(UAC_SOCKET fd);

private:
	virtual void on(ChannelListener::Connecting,Channel* ch);
	virtual void on(ChannelListener::Connected,Channel* ch);
	virtual void on(ChannelListener::Disconnected,Channel* ch);
	virtual void on(ChannelListener::Data,Channel* ch,memblock *b);
	virtual void on(ChannelListener::Writable,Channel* ch);

private:
	SMutex m_mt;
	UACSocket_t *m_socks;
	UDPAcceptor	*m_acceptor;
	int m_socknum; //记录当前连接数
	int m_maxsock; //记录当前最大的fd

	//底层通知上层的消息队列
	list<UAC_SOCKET> m_ls_accepting; 

	//上层通知底层的消息队列
	list<UAC_SOCKET> m_ls_connecting;
	list<UAC_SOCKET> m_ls_disconnecting;
	list<UAC_SOCKET> m_ls_sending;
};
typedef Singleton<sockpool> sockpoolsngl;
}


