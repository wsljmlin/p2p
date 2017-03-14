#pragma once
#include "uac_mempool.h"
#include "uac_SockAcceptor.h"
#include "uac_UDPConnector.h"
#include "uac_UDPStunClient.h"

namespace UAC
{

class UDPAcceptor : public  SockAcceptor,private UDPStunClientListener
{
	friend class Speaker<UDPStunClientListener>;
public:
	UDPAcceptor(void);
	virtual ~UDPAcceptor(void);

public:
	int open(unsigned short port,const char* stunsvr,unsigned short stun_port,UDPChannelFactory* chf);
	int close();
	UDPConnector* get_connector(){return m_ctr;}

private:
	virtual int handle_input();
private:
	virtual void on(UDPStunClientListener::NatOk,int nat_type);
	virtual void on(UDPStunClientListener::BindingAddrChanged,unsigned int ip,unsigned short port);
	virtual void on(UDPStunClientListener::ReqHole,unsigned int connid,unsigned int nip,unsigned short nport);
	virtual void on(UDPStunClientListener::RspHole,unsigned int connid,unsigned int nip,unsigned short nport);
private:
	UDPConnector *m_ctr;
	UDPStunClient *m_stun;


	int _tmp_n;
	memblock *_tmp_block;
	sockaddr_in _tmp_addr;
	socklen_t _tmp_addr_len;
};

}

