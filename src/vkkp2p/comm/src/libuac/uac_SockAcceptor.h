#pragma once
#include "uac_basetypes.h"


namespace UAC
{

class SockAcceptor
{
public:
	SockAcceptor(void);
	virtual ~SockAcceptor(void);

	
public:
	int open_sock(unsigned short port,const char* ip);
	void close_sock();

	int handle_select_read(unsigned long delay_usec=0);

	int sock(){return (int)m_fd;}

private:
	virtual int handle_input()=0;

protected:
	int m_fd;
	unsigned int m_hip;
	unsigned short m_hport;

private:
	//¡Ÿ ±±‰¡ø
	timeval _timeout;
	fd_set m_rfd;
	fd_set _rfd;
	int _n;
};

}

