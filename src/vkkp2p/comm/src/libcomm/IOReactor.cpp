#include "IOReactor.h"

IOReactor::IOReactor(void)
:m_handler_num(0)
{
}

IOReactor::~IOReactor(void)
{
	assert(0==m_handler_num);
}


int IOReactor::set_blocking(SOCKET s,bool is_blocking)
{
#ifdef _WIN32
	//NONBLOCKING=1
	u_long val = is_blocking?0:1;
	if(INVALID_SOCKET!=s)
		return ioctlsocket(s,FIONBIO,&val);
	return -1;
#elif defined(_ECOS_8203)
	int val = is_blocking?0:1;
	return ioctl(s,FIONBIO,&val);
#else
	int opts;
	opts = fcntl(s,F_GETFL);
	if(-1 == opts)
	{
		perror("fcntl(s,GETFL)");
		return -1;
	}
	if(!is_blocking)
		opts |= O_NONBLOCK;
	else
		opts &= ~O_NONBLOCK;
	if(-1 == fcntl(s,F_SETFL,opts))
	{
		DEBUGMSG("***error s=%d ***\n",s);
		perror("fcntl(s,SETFL,opts); ");
		return -1;
	}
	return 0;
#endif
}

