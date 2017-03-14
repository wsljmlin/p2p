#include "TCPAcceptor.h"


TCPAcceptor::TCPAcceptor(void)
:m_chf(NULL)
{
}

TCPAcceptor::~TCPAcceptor(void)
{
	
}

int TCPAcceptor::open(unsigned short port,const char* ip,TCPChannelFactory* chf,IOReactor* reactor)
{
	assert(chf);
	if(NULL==chf)
		return -1;
	m_chf = chf;
	if(0!=open_sock(port,ip,0,reactor))
		return -1;
	return 0;
}
void TCPAcceptor::close()
{
	close_sock();
	m_chf = NULL;
}
int TCPAcceptor::handle_input()
{
	//考虑epoll的边缘模型
	DEBUGMSG("TCPAcceptor::handle_input()...\n");
	SOCKET fd;
	sockaddr_in addr;
	socklen_t len ;
	while(1)
	{
		memset(&addr,0,sizeof(addr));
		len =  sizeof(addr);
		fd = accept(m_fd,(sockaddr*)&addr,&len);
		if(INVALID_SOCKET == fd)
			return 0;

		//如果频繁有相同IP连接进入，考虑洪水处理
		if(NULL==m_chf || !m_chf->attach_tcp_socket(fd,addr))
			::closesocket(fd);
	}
	return 0;
}


