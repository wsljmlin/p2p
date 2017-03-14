#include "SelectReactor.h"

SelectReactor::SelectReactor(void)
{
	assert(1024==FD_SETSIZE);
	m_cursor = -1;
	m_maxsock = INVALID_SOCKET;
	m_sn = new SNode_t[FD_SETSIZE];
	FD_ZERO(&m_rfd);
	FD_ZERO(&m_wfd);
	FD_ZERO(&m_efd);
}

SelectReactor::~SelectReactor(void)
{
	assert(-1==m_cursor);
	delete[] m_sn;
}

void SelectReactor::handle_root(ULONGLONG delay_usec/*=0*/)
{
	timeval timeout;
	timeout.tv_sec=(long)(delay_usec/1000000);
	timeout.tv_usec=(long)(delay_usec%1000000);
	int n;
	
	rfd=m_rfd; //自动拷贝数组数据
	wfd=m_wfd;
	efd=m_efd;
	//select_init(&rfd,&wfd,&efd,m_maxsock);
	if(INVALID_SOCKET == m_maxsock)
	{
		Sleep((long)(delay_usec/1000));
	}
	else
	{
		n = select(m_maxsock+1,&rfd,&wfd,&efd,&timeout);
		if(n>0)
			select_finish(&rfd,&wfd,&efd,n);
	}


}
int SelectReactor::register_handler(SockHandler *h,int se,bool is_et/*=false*/)
{
	assert(h && (se & SE_BOTH));
	if(NULL==h)
		return -1;

	int i = h->__i;
	if(-1==i)
	{
		//新分配
		i = allot_nodei();
		if(-1==i)
			return -1;
		if(m_cursor<i)
			m_cursor = i;
		h->__i = i;
		m_sn[i].h = h;
		m_sn[i].s = h->sock();
		assert(m_sn[i].s!=INVALID_SOCKET);
		set_blocking(m_sn[i].s,false);
		FD_SET(m_sn[i].s,&m_efd);
		if(m_maxsock<(int)m_sn[i].s)
			m_maxsock = (int)m_sn[i].s;
		m_handler_num++;
	}
	else
	{
		assert(m_sn[i].h == h);
	}
	if(0!=(m_sn[i].se&se))
		assert(false);//避免重复注册
	if(se==(m_sn[i].se&se))
		return 0;//已经有对应的
	m_sn[i].se |= se;
	if(se & SE_READ)
		FD_SET(m_sn[i].s,&m_rfd);
	if(se & SE_WRITE)
		FD_SET(m_sn[i].s,&m_wfd);
	
	return 0;
}
int SelectReactor::unregister_handler(SockHandler *h,int se)
{
	assert(h && (se & SE_BOTH));
	if(NULL==h)
		return -1;

	int i = h->__i;
	if(-1==i)
		return -1;
	assert(m_sn[i].h == h);
	m_sn[i].se &= ~se;
	if(se & SE_READ)
		FD_CLR(m_sn[i].s,&m_rfd);
	if(se & SE_WRITE)
		FD_CLR(m_sn[i].s,&m_wfd);
	if(0==m_sn[i].se)
	{
		FD_CLR(m_sn[i].s,&m_efd);
		int s = (int)m_sn[i].s;
		//释放
		h->__i = -1;
		set_blocking(m_sn[i].s,true);
		m_sn[i].reset();
		while(m_cursor>=0 && !m_sn[m_cursor].h)
			m_cursor--;
		if(s==m_maxsock)
		{
			m_maxsock = INVALID_SOCKET;
			for(int i=0;i<=m_cursor;++i)
				if(m_maxsock<(int)m_sn[i].s)
					m_maxsock = (int)m_sn[i].s;
		}
		m_handler_num--;
	}
	return 0;
}

void SelectReactor::select_init(fd_set* prfd,fd_set* pwfd,fd_set* pefd,SOCKET& maxs)
{
	SNode_t *p;
	FD_ZERO(prfd);
	FD_ZERO(pwfd);
	FD_ZERO(pefd);
	maxs=INVALID_SOCKET;
	for(int i=0;i<=m_cursor;++i)
	{
		if(!m_sn[i].h)
			continue;
		p = &m_sn[i];
		assert(p->se&&INVALID_SOCKET!=p->s);
		if(p->se & SE_READ)
			FD_SET(p->s,prfd);
		if(p->se & SE_WRITE)
			FD_SET(p->s,pwfd);
		FD_SET(p->s,pefd);
		if(INVALID_SOCKET==maxs || maxs< p->s)
			maxs = p->s;
	}
}
void SelectReactor::select_finish(fd_set* prfd,fd_set* pwfd,fd_set* pefd,int n)
{
	SNode_t *p;
	int j=0;
	int wait=0;
	//注意考虑回调后有些被注销了的情况
	for(int i=0;i<=m_cursor;++i)
	{
		if(!m_sn[i].h)
			continue;
		p = &m_sn[i];
		if(FD_ISSET(p->s,pefd))
		{
			++j;
			p->h->handle_error();
		}
		if((p->se & SE_READ) && FD_ISSET(p->s,prfd))
		{
			++j;
			if(1==p->h->handle_input())
			{
				wait++;
			}
		}
		
		if((p->se & SE_WRITE) && FD_ISSET(p->s,pwfd))
		{
			++j;
			if(1==p->h->handle_output())
			{
				wait++;
			}
		}
		if(j>=n)
			break;
	}
	if(wait && wait == j)
		Sleep(1); //限速使用
}

int SelectReactor::allot_nodei()
{
	for(int i=0;i<FD_SETSIZE;++i)
	{
		if(!m_sn[i].h)
			return i;
	}
	return -1;
}
