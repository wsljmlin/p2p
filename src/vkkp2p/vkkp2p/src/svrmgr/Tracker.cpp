#include "StdAfx.h"
#include "Tracker.h"

Tracker::Tracker(void)
{
}

Tracker::~Tracker(void)
{
}

int Tracker::get_server(const char *ip,short port,PTL_P2T_ResponseServerList& rsp,PTL_P2T_ServerInfo *svr,int size)
{
	SOCKET m_sock = socket(AF_INET,SOCK_STREAM,0);
	if(m_sock == INVALID_SOCKET)
	{
		return -1;
	}
	
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	if(SOCKET_ERROR==connect(m_sock,(sockaddr*)&addr,sizeof(addr)))
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return -1;
	}
	int x = 5000;
	setsockopt(m_sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&x,sizeof(x));

	send_http_req(m_sock);
	recv_http_rsp(m_sock);

	//发请求
	PTL_Head head;
	PTL_P2T_RequestServerList req;
	req.maxnum = size;
	PTLStream pack(2048);
	head.cmd = PTL_P2T_REQUEST_SERVER_LIST;
	pack << head;
	pack << req;
	pack.fitsize32(4);

	if(0!=send_n(m_sock,pack.buffer(),pack.length()))
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return -1;
	}

	int pos = 0;
	char buf[2048];
	int buflen=0;
	while(pos<size)
	{
		if(0!=recv_pack(m_sock,buf,2048,buflen))
		{
			pos=-1;
			break;
		}
		pack.attach(buf,buflen,buflen);
		pack >> head;
		if(PTL_P2T_RESPONSE_SERVER_LIST==head.cmd && 0==pack >> rsp)
		{
			for(int i=0;i<(int)rsp.num;++i)
			{
				svr[pos++] = rsp.servers[i];
				if(pos>=size)
					break;
			}
			if(rsp.startNum+rsp.num==rsp.allNum)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	closesocket(m_sock);
	m_sock = INVALID_SOCKET;

	return pos;
}
//int Tracker::update_server(const char *ip,short port,P2TRspUpdateServer *svr,int size)
//{
//	SOCKET m_sock = socket(AF_INET,SOCK_STREAM,0);
//	if(m_sock == INVALID_SOCKET)
//	{
//		return -1;
//	}
//	
//	sockaddr_in addr;
//	addr.sin_family = AF_INET;
//	addr.sin_port = htons(port);
//	addr.sin_addr.s_addr = inet_addr(ip);
//	if(SOCKET_ERROR==connect(m_sock,(sockaddr*)&addr,sizeof(addr)))
//	{
//		closesocket(m_sock);
//		m_sock = INVALID_SOCKET;
//		return -1;
//	}
//	int x = 5000;
//	setsockopt(m_sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&x,sizeof(x));
//
//	//发请求
//	CommHead head;
//	P2TReqUpdateServer req;
//	NetPacket pack;
//	head.cmd = V_P2T_REQ_UPDATE_SERVER;
//	
//	char buf[2048];
//	int pos=0;
//	while(pos<size)
//	{
//		int num = size-pos;
//		if(num>20) num=20;
//		for(int i=0;i<num;++i)
//			req.svr[i] = svr[pos+i];
//		req.num = num;
//		pack.attach((uchar*)buf,2048);
//		pack << head;
//		pack << req;
//		pack.fini();
//
//		if(0!=send_n(m_sock,(char*)pack.get_buf(),pack.length()))
//		{
//			break;
//		}
//		
//		pos += num;
//	}
//
//	closesocket(m_sock);
//	m_sock = INVALID_SOCKET;
//
//	return 0;
//}
int Tracker::send_n(SOCKET sock,char *buf,int size)
{
	int pos=0;
	int ret=0;
	while(pos<size)
	{
		ret = send(sock,buf+pos,size-pos,0);
		if(ret>0)
			pos += ret;
		else
			return -1;
	}
	return 0;
}
int Tracker::recv_pack(SOCKET m_sock,char *m_buf,int m_maxlen,int& m_buflen)
{
	if(m_maxlen<32)
		return -1;
	m_buflen=0;
	int ret = 0;
	while(m_buflen<sizeof(PTL_Head))
	{
		ret = recv(m_sock,m_buf+m_buflen,sizeof(PTL_Head) - m_buflen,0);
		if(ret > 0)
		{
			m_buflen += ret;
		}
		else
		{
			return -1;
		}
	}
	uint32 *pstx = (uint32*)(m_buf);
	if(*pstx != PTL_HEAD_STX_32)
		return -1;

	PTL_Head *phead = (PTL_Head*)pstx;
	int size = SerialStream::ltoh32(phead->size);
	if(size > m_maxlen)
		return -1;

	while(m_buflen < size)
	{
		ret = recv(m_sock,m_buf+m_buflen,size-m_buflen,0);
		if(ret > 0)
		{
			m_buflen += ret;
		}
		else
		{
			return -1;
		}
	}
	return 0;
}
#define FW_HTTP_SIZE 256
int Tracker::send_http_req(SOCKET sock)
{
	char buf[FW_HTTP_SIZE];
	memset(buf,0,FW_HTTP_SIZE);
	if(FW_HTTP_SIZE == send(sock,buf,FW_HTTP_SIZE,0))
		return 0;
	return -1;

}
int Tracker::recv_http_rsp(SOCKET sock)
{
	char buf[FW_HTTP_SIZE];
	memset(buf,0,FW_HTTP_SIZE);
	int readn = 0;
	int ret = 0;
	while(readn<FW_HTTP_SIZE)
	{
		ret = recv(sock,buf+readn,FW_HTTP_SIZE - readn,0);
		if(ret > 0)
		{
			readn += ret;
		}
		else
		{
			return -1;
		}
	}
	return 0;
}

