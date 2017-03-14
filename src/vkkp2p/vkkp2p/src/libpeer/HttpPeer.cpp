#include "HttpPeer.h"
#include "TCPChannel.h"
#include "Util.h"

#ifdef SM_DBG
#define HTTPPEER_PRT(fmt, arg...) //fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "HttpPeer",__LINE__, ##arg)
#else
#define HTTPPEER_PRT(fmt, arg...)
#endif


HttpPeer::HttpPeer(void)
:Peer(IPT_HTTP,TCP_CONN,-1)
,m_is_pause(false)
,m_req_begin(0)
,m_req_end(-1)
,m_req_times(0)
,m_recv_size(0)
{
	DEBUGMSG("#new httppeer  \n");
}

HttpPeer::~HttpPeer(void)
{
	DEBUGMSG("#delete httppeer \n");
}

int HttpPeer::send_request(const string& server,const string& cgi,sint64 ibegin/*=0*/,sint64 iend/*=-1*/,int httpver/*=1*/)
{
	m_header_string = "";
	string header;
	MemBlock *block = MemBlock::allot(1024);
	if(!block)
		return -1;
	m_req_begin = ibegin;
	m_req_end = iend;
	m_req_times ++;
	m_block->limit(m_block->buflen-2);
	format_head(header,server,cgi,ibegin,iend,httpver);
	memcpy(block->buf,header.c_str(),header.length());
	block->datalen = (int)header.length();
	HTTPPEER_PRT("----send server:%s---headbuf=%s\n", server.c_str(),header.c_str());
	return m_ch->send(block);
}

void HttpPeer::format_head(string& header,const string& server,const string& cgi,sint64 ibegin/*=0*/,sint64 iend/*=-1*/,int httpver/*=1*/)
{
	header = "";
	///1:方法,请求的路径,版本
	header = "GET ";
	header+=cgi;
	if(0==httpver)
		header+=" HTTP/1.0";
	else
		header+=" HTTP/1.1";
	header+="\r\n";

	///2:主机
	header+="Host: ";
	header+=server;
	header+="\r\n";

	///3:接收的数据类型
	header+="Accept: */* \r\n";
	//header+="Accept-Encoding: \r\n";

	///4:浏览器类型
	header+="User-Agent: Mozilla/4.0 (compatible; vkk 1.1;)\r\n";

	header += "Pragma: no-cache\r\n";
	header += "Cache-Control: no-cache\r\n";

	///5:请求的数据起始字节位置(断点续传的关键)
	if(ibegin!=0 || iend!=-1)
	{
		char szTemp[64];
		header+="";
		sprintf(szTemp,"Range: bytes=%lld-",ibegin);
		header+=szTemp;
		if(iend > ibegin)
		{
			sprintf(szTemp,"%lld",iend);
			header+=szTemp;
		}
		header+="\r\n";
	}

	///6:连接设置,保持
	header+="Connection: Keep-Alive \r\n";
	///最后一行:空行
	header+="\r\n";
}

void HttpPeer::on(Readable,Channel* ch,const int& wait)
{
	//tcp recv
	int ret = 0;
	int& wait1 = (int&)wait;
	wait1 = 0;
	while(CONNECTED==m_ch->get_state())
	{
		if(m_is_pause)
		{
			wait1 = 1;
			return;
		}
		ret = m_block->remaining();
		if(m_pPeerReadLimitSinker)
		{
			int n = m_pPeerReadLimitSinker->on_peer_readlimit(this);
			if(n>=0 && ret>n)
				ret = n;
			if(ret==0)
			{
				wait1 = 1;
				return;
			}
		}
		ret = m_ch->recv(m_block->pointer(),ret);
		if(ret<=0)
			return;
		m_block->increase(ret);
		on_data();
	}
}

void HttpPeer::on_data()
{
	if(m_header_string.empty())
	{
		HTTPPEER_PRT("xxxxxxx_________________\n%s\n", m_block->buf);
		m_block->buf[m_block->position()] = '\0';
#ifdef SM_MODIFY
		// for letv, only have a "\r\n", not two
		char* p = strstr(m_block->buf,"\r\n\r\n");
		if(!p) {
			p = strstr(m_block->buf,"\r\n");
			if(!p) {
				if(m_block->remaining()==0)
					this->disconnect();
				return;
			}
		}
		*(p+1) = '\0';
#else
		char* p = strstr(m_block->buf,"\r\n\r\n");
		if(!p)
		{
			if(m_block->remaining()==0)
				this->disconnect();
			return;
		}
		*(p+2) = '\0';
#endif /* end of SM_MODIFY */
		m_header_string = m_block->buf;
		m_recv_size = 0;
		fire(PeerListener::HttpHeader(),this,(char*)m_header_string.c_str(),(int)m_header_string.length());
		HTTPPEER_PRT("-----head=%s len=%d\n", (char*)m_header_string.c_str(), (int)m_header_string.length());
		if(CONNECTED!=m_ch->get_state())
			return;
		unsigned int len = (unsigned int)(m_block->position() - (p+4-m_block->buf));
		if(len>0)
		{
			HTTPPEER_PRT("boday begin: %s\n", p+4);
			m_recv_size += len;
			fire(PeerListener::Data(),this,p+4,len);
		}
	}
	else
	{
		HTTPPEER_PRT("%s\n", m_block->buf);
		m_recv_size += m_block->position();
		fire(PeerListener::Data(),this,m_block->buf,m_block->position());
	}
		
	m_block->position(0);
}
int HttpPeer::get_server_response_code()
{
	if(m_header_string.length()<9) return -1;
	return atoi(m_header_string.c_str()+9);
}
int HttpPeer::get_field(const char *szSession, string& text)
{
	//取得某个域值
	if(m_header_string.empty()) 
		return -1;
	size_t nPos = string::npos;
	
	char* s = strcasestr((char*)m_header_string.c_str(),(char*)szSession);
	if(s)
		nPos = (size_t)(s-m_header_string.c_str());
	//nPos = m_header_string.find(szSession,0);
	if(nPos != string::npos)
	{
		nPos += strlen(szSession);
		nPos += 2;
		size_t nCr = m_header_string.find("\r\n",nPos);
		text = m_header_string.substr(nPos,nCr - nPos);
		Util::string_trim(text);
		return (int)text.length();
	}
	else
	{
		return -1;
	}
}
