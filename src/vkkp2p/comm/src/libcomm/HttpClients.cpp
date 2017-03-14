#include "HttpClients.h"
#include "Util.h"

#define HTTP_HEAD_MAX_SIZE 4095

HttpClients::HttpClients(void)
:Speaker<HttpClientListener>(1)
,m_state(HTTP_DISCONNECTED)
,m_headbuf_recv_size(0)
,m_body_size(0)
,m_body_recv_size(0)
{
	m_ch.add_listener(this);
	m_headbuf = new char[HTTP_HEAD_MAX_SIZE+1];
	m_bodybuf = new char[4096];
}

HttpClients::~HttpClients(void)
{
	assert(HTTP_DISCONNECTED==m_state);
	m_ch.remove_listener(this);
	delete[] m_headbuf;
	delete[] m_bodybuf;
}
int HttpClients::connect(const char* ip,unsigned short port)
{
	return m_ch.connect(ip,port);
}
int HttpClients::connect(unsigned int ip,unsigned short port)
{
	return m_ch.connect(ip,port);
}
int HttpClients::disconnect()
{
	if(DISCONNECTED==m_ch.get_state())
	{
		assert(HTTP_DISCONNECTED==m_state);
		//未连接时也要回调，上层在Disconnected()时才删除
		fire(HttpClientListener::HttpDisconnected(),this);
		return 0;
	}
	return m_ch.disconnect();
}
int HttpClients::send_simple_req_cgi(const char* cgi,const char* server)
{
	char *headbuf = new char[strlen(cgi) + 1024];
	
	sprintf(headbuf,"GET %s HTTP/1.1\r\n",cgi);
	sprintf(headbuf+strlen(headbuf),"Host: %s\r\n",server);
	strcat(headbuf,"Accept: */*\r\n");
	strcat(headbuf,"User-Agent: Mozilla/4.0 (compatible; eWatch 1.0;)\r\n");
	strcat(headbuf,"Pragma: no-cache\r\n");
	strcat(headbuf,"Cache-Control: no-cache\r\n");
	strcat(headbuf,"Connection: Close \r\n");
	strcat(headbuf,"\r\n");

	int ret = send_request_header(headbuf);
	delete[] headbuf;
	return ret;
}
int HttpClients::send_request_header(const char* reqheadbuf)
{
	if(HTTP_READY!=m_state)
		return -1;
	m_state = HTTP_RSPONSE_HEADER;
	m_headbuf_recv_size = 0;
	m_body_size = m_body_recv_size = 0;
	int len = (int)strlen(reqheadbuf);
	MemBlock* block = MemBlock::allot(len);
	memcpy(block->buf,reqheadbuf,len);
	block->datalen = len;
	return m_ch.send(block);
}

//**************************************************************
void HttpClients::on(Connected,Channel* ch)
{
	fire(HttpClientListener::HttpConnected(),this);
	m_state = HTTP_READY;
}
void HttpClients::on(Disconnected,Channel* ch)
{
	m_state = HTTP_DISCONNECTED;
	fire(HttpClientListener::HttpDisconnected(),this);
}
void HttpClients::on(Readable,Channel* ch,const int& wait)
{
	//tcp recv
	int ret = 0;
	while(CONNECTED==m_ch.get_state())
	{
		if(HTTP_RSPONSE_HEADER==m_state)
		{
			if(m_headbuf_recv_size>=HTTP_RSPONSE_HEADER)
			{
				DEBUGMSG("# *** biger http_header !!!\n");
				disconnect();
				return;
			}
			ret = m_ch.recv(m_headbuf+m_headbuf_recv_size,HTTP_HEAD_MAX_SIZE-m_headbuf_recv_size);
			if(ret>0)
			{
				m_headbuf_recv_size += ret;
				on_recv_header();
			}
			else
				return;
		}
		else if(HTTP_RSPONSE_BODY==m_state)
		{
			ret = m_ch.recv(m_bodybuf,4096);
			if(ret>0)
			{
				on_recv_body(m_bodybuf,ret);
			}
			else
				return;
		}
		else
		{
			assert(false);
		}
	}
}

void HttpClients::on_recv_header()
{
	m_headbuf[m_headbuf_recv_size] = '\0';
	char *ptr = strstr(m_headbuf,"\r\n\r\n");
	char *body;
	char szTxt[1024];
	if(!ptr)
		return;

	*(ptr+2) = '\0'; //只保留一个回车
	body = ptr + 4;
	m_body_recv_size = m_headbuf_recv_size - (int)(ptr-m_headbuf+4);
	if(0==get_field(m_headbuf,"Content-Length",szTxt,1024))
	{
		m_body_size = Util::atoll(szTxt);
	}
	m_state = HTTP_RSPONSE_BODY;
	fire(HttpClientListener::HttpResponseHeader(),this,m_headbuf);
	if(CONNECTED==m_ch.get_state() && m_body_recv_size>0)
	{
		if(m_body_recv_size>=m_body_size)
			m_state = HTTP_READY;
		fire(HttpClientListener::HttpResponseBody(),this,body,(int)m_body_recv_size);
	}
	
}
void HttpClients::on_recv_body(const char* buf,int len)
{
	m_body_recv_size += len;
	if(m_body_recv_size>=m_body_size)
		m_state = HTTP_READY;
	fire(HttpClientListener::HttpResponseBody(),this,buf,len);
}


int HttpClients::get_response_code(const char* szHead)
{
	//HTTP/1.1 200
	return atoi(szHead+9);
}
int HttpClients::get_field(const char* szHead,const char *szSession, char* szTxt,int nTxt)
{
	//取得某个域值
	const char *s1,*s2;
	s1 = strcasestr(szHead,szSession);
	if(!s1)
		return -1;
	s1 += (strlen(szSession)+2);
	s2 = strstr(s1,"\r\n");
	if(!s2)
		return -1;
	int len = (int)(s2-s1);
	if(len>=nTxt)
		return -1;
	memcpy(szTxt,s1,len);
	szTxt[len] = '\0';
	return 0;
}

