#include "uac_SocketSelector.h"


UAC_SocketSelector::UAC_SocketSelector(UAC_SocketFactory* fac)
:m_chs(new Node_t[UAC_FD_SIZE])
,m_regnum(0)
,m_fac(fac)
{
	assert(fac);
}

UAC_SocketSelector::~UAC_SocketSelector(void)
{
	assert(0==m_regnum);
	delete[] m_chs;
}


int UAC_SocketSelector::register_socket(UAC_Socket* s,char rw_mask)
{
	int fd = s->uac_get_fd();
	if(0==m_chs[fd].s)
	{
		m_chs[fd].s = s;
		m_chs[fd].rw_mask = rw_mask;
		m_regnum++;
	}
	else
	{
		m_chs[fd].rw_mask |= rw_mask;
	}
	return 0;
}
int UAC_SocketSelector::unregister_socket(UAC_Socket* s,char rw_mask)
{
	int fd = s->uac_get_fd();
	if(0!=m_chs[fd].s)
	{
		m_chs[fd].rw_mask &= (~rw_mask);
		if(0==m_chs[fd].rw_mask)
		{
			m_chs[fd].s = 0;
			m_regnum--;
		}
	}
	return 0;
}

int UAC_SocketSelector::handle_readwrite()
{
	int i=0,n=0,fd;
	//accept:

	//read/write
	uac_select(&m_rset,&m_wset);
	for(i=0;i<m_rset.fd_count;++i)
	{
		fd = m_rset.fd_array[i];
		if(m_chs[fd].s && (UAC_READ&m_chs[fd].rw_mask))
		{
			if(1==m_chs[fd].s->uac_on_read())
				n++; //限速情况
		}
	}
	for(i=0;i<m_wset.fd_count;++i)
	{
		fd = m_wset.fd_array[i];
		if(m_chs[fd].s && (UAC_WRITE&m_chs[fd].rw_mask))
		{
			m_chs[fd].s->uac_on_write();
		}
	}
	if(n>0 && n==m_rset.fd_count) return 1; //
	if(0==m_rset.fd_count) return -1;//本轮处理空闲
	return 0; //本轮有处理
}

void UAC_SocketSelector::handle_accept()
{
	UAC_SOCKET fd;
	while(-1!=(fd = uac_accept(&_client_addr)))
	{
		if(!m_fac->uac_attach_socket(fd,_client_addr))
			uac_closesocket(fd);
	}
}



