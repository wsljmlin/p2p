#include "uac_UDPAcceptor.h"

#include "uac_UDPProtocol.h"
#include "uac_Util.h"


namespace UAC
{
UDPAcceptor::UDPAcceptor(void)
:m_ctr(NULL)
,m_stun(NULL)
,_tmp_n(0)
,_tmp_block(NULL)
{
	_tmp_addr_len = sizeof(_tmp_addr);
	memset(&_tmp_addr,0,_tmp_addr_len);	
}

UDPAcceptor::~UDPAcceptor(void)
{
}

int UDPAcceptor::open(unsigned short port,const char* stunsvr,unsigned short stun_port,UDPChannelFactory* chf)
{
	assert(chf);
	if(NULL==chf || NULL!=m_ctr)
		return -1;
	if(0!=open_sock(port,NULL))
		return -1;
	if(stunsvr && stun_port>0)
	{
		m_stun = new UDPStunClient(m_fd,stunsvr,stun_port);
		m_stun->add_listener(this);
	}
	m_ctr = new UDPConnector(m_fd,chf,m_stun);
	return 0;
}
int UDPAcceptor::close()
{
	close_sock();
	if(m_ctr)
	{
		delete m_ctr;
		m_ctr = NULL;
	}
	if(m_stun)
	{
		m_stun->remove_listener(this);
		delete m_stun;
		m_stun = NULL;
	}
	if(_tmp_block)
	{
		_tmp_block->free(UAC_THREAD_CORE);
		_tmp_block = NULL;
	}
	return 0;
}
int UDPAcceptor::handle_input()
{
	//ULONGLONG utick = GetUTickCount();
	int n = 0;
	while(1)
	{
		//todo:在此实现限速
		if(NULL==_tmp_block)
		{
			_tmp_block = memblock::alloc(UAC_THREAD_CORE);
			if(NULL==_tmp_block) return -1;
		}
		_tmp_n = recvfrom(m_fd,_tmp_block->buffer,_tmp_block->bufsize,0,(sockaddr*)&_tmp_addr,&_tmp_addr_len);
		if(_tmp_n>0)
		{
			n++;
			_tmp_block->datasize = _tmp_n;
			if(UDPS_CONN_HEAD_STX==(unsigned int)(_tmp_block->buffer[0]&0xf0))
			{
				m_ctr->on_data(_tmp_block,_tmp_addr);
				_tmp_block = NULL;
			}
			else if(m_stun && PTL_STUN_HEAD_STX==(unsigned char)(_tmp_block->buffer[0]))
			{
				m_stun->on_data(_tmp_block->buffer,_tmp_n,_tmp_addr);
				_tmp_block->datasize = 0;
			}
			else
			{
				_tmp_block->datasize = 0;
			}
			memset(&_tmp_addr,0,_tmp_addr_len);	
		}
		else
		{
			break;
		}
	}
	//printf("-%lld",GetUTickCount()-utick);
	return 0;
}

void UDPAcceptor::on(UDPStunClientListener::NatOk,int nat_type)
{
	UACLOG("#UAC check natttype = %d \n",nat_type);
	g_udps_conf.nattype = nat_type;
	if(g_udps_conf.callback_onnatok)
		g_udps_conf.callback_onnatok(nat_type);
}
void UDPAcceptor::on(UDPStunClientListener::BindingAddrChanged,unsigned int ip,unsigned short port)
{
	UACLOG("#UAC ipport changed(%s:%d) \n",Util::ip_htoa(ip),port);
	if(g_udps_conf.callback_onipportchanged)
		g_udps_conf.callback_onipportchanged(ip,port);
}
void UDPAcceptor::on(UDPStunClientListener::ReqHole,unsigned int connid,unsigned int nip,unsigned short nport)
{
	if(m_ctr) m_ctr->nat(nip,nport);
}
void UDPAcceptor::on(UDPStunClientListener::RspHole,unsigned int connid,unsigned int nip,unsigned short nport)
{
	if(m_ctr) m_ctr->on_rsp_nat_hole(connid,nip,nport);
}


}

