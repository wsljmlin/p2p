#include "ProReactor.h"

#ifdef _WIN32

#else
#ifndef NO_EPOLL
#define WAIT_SIZE 1024
//epoll_create(int size); Since Linux 2.6.8, the size argument is unused
ProReactor::ProReactor(int fdsize)
{
	m_max_fdsize = fdsize;
	m_epfd = epoll_create(m_max_fdsize);
	if(-1==m_epfd)
		DEBUGMSG("epoll_create error:%d\n",errno);
	events = new epoll_event[WAIT_SIZE];
	memset(events,0,WAIT_SIZE*sizeof(epoll_event));
	if(-1==m_sn.resize(m_max_fdsize))
	{
		DEBUGMSG("# *** ProReactor: m_sn.resize(%d) failed. allot memery failed !!! \n",m_max_fdsize);
		assert(false);
	}
}

ProReactor::~ProReactor(void)
{
	close(m_epfd);
	m_sn.resize(0);
	delete[] events;
}

void ProReactor::handle_root(ULONGLONG delay_usec/*=0*/)
{
	nfds = epoll_wait(m_epfd,events,WAIT_SIZE,0);
	//返回0为timeout
	if(-1 == nfds)
	{
		perror("epoll_wait error");
		DEBUGMSG("epoll_wait error:%d\n",errno);
	}else if(nfds>0)
	{
		int n = 0;
		//DEBUGMSG("ProReactor::handle_root :nfds=%d \n",nfds);
		SockHandler *h = NULL;
		for(int i=0;i<nfds;++i)
		{
			//是否一次响应多事件
			n = 0;
			if(events[i].events & EPOLLIN)
			{
				h = m_sn[events[i].data.fd].h;
				if(h)
					h->handle_input();
				n++;
			}
			if(events[i].events & EPOLLOUT)
			{
				h = m_sn[events[i].data.fd].h;
				if(h)
					h->handle_output();
				n++;
			}
			if(events[i].events & EPOLLERR)
			{
				h = m_sn[events[i].data.fd].h;
				if(h)
					h->handle_error();
				n++;
			}
			if(0 == n || n>1)
			{
				DEBUGMSG("***************once %d events : event=0x%x \n",n,events[i].events);
			}
		}
	}
}
int ProReactor::register_handler(SockHandler *h,int se,bool is_et/*=false*/)
{
	assert(h && (se & SE_BOTH));
	int i = h->__i;
	int s = h->sock();
	assert(-1 != s);
	bool bnew = false;
	if(-1==i)
	{
		//不存在于列表先从列表注册一个
		i = m_sn.allot();
		if(-1==i)
			return -1;
		bnew = true;
		h->__i = i;
		m_sn[i].h = h;
		m_sn[i].s = s;
		set_blocking(s,false);
		m_handler_num++;
	}
	if(0!=(m_sn[i].se&se))
		assert(false);//避免重复注册
	if(se==(m_sn[i].se&se))
		return 0;//已经有对应的

	//注意:修改监听事件时,要把旧的事件加上
	m_sn[i].is_et = is_et;
	se = m_sn[i].se | se;
	epoll_event ev;
	memset(&ev,0,sizeof(ev));
	ev.events = 0;
	ev.data.fd = i;
	if(m_sn[i].is_et)
		ev.events = EPOLLET/*|EPOLLERR*/;
	if(se & SE_READ) ev.events |= EPOLLIN;
	if(se & SE_WRITE) ev.events |= EPOLLOUT;

	int ret = 0;
	if(bnew)
		ret = epoll_ctl(m_epfd,EPOLL_CTL_ADD,s,&ev);
	else
		ret = epoll_ctl(m_epfd,EPOLL_CTL_MOD,s,&ev);
	if(ret != 0)
	{
		DEBUGMSG("***error fd=%d se=%d ***\n",s,se);
		perror("epoll_ctl(m_epfd,EPOLL_CTL_,fd,&ev);");
		//取消注册
		if(bnew)
		{
			set_blocking(s,true);
			m_sn[i].reset();
			m_sn.free(i);
			h->__i = -1;
		}
		return -1;
	}
	m_sn[i].se = se;
	return 0;
}
int ProReactor::unregister_handler(SockHandler *h,int se)
{
	assert(h && (se & SE_BOTH));
	if(!h)
		return -1;
	int i = h->__i;
	int s = h->sock();
	if(-1==i)
		return 0;

	se =m_sn[i].se & (~se);
	epoll_event ev;
	memset(&ev,0,sizeof(ev));
	ev.events = 0;
	ev.data.fd = i;
	if(0 == se)
	{
		epoll_ctl(m_epfd,EPOLL_CTL_DEL,s,&ev);
		set_blocking(m_sn[i].s,true);
		m_sn[i].reset();
		m_sn.free(i);
		h->__i = -1;
		m_handler_num--;
	}
	else
	{
		if(m_sn[i].is_et)
			ev.events = EPOLLET/*|EPOLLERR*/;
		if(se & SE_READ) ev.events |= EPOLLIN;
		if(se & SE_WRITE) ev.events |= EPOLLOUT;
		if(-1 == epoll_ctl(m_epfd,EPOLL_CTL_MOD,s,&ev))
		{
			perror("epoll_ctl(m_epfd,EPOLL_CTL_MOD,s,&ev);");
			return -1;
		}
		m_sn[i].se = se;
	}

	return 0;
}
#endif

#endif

