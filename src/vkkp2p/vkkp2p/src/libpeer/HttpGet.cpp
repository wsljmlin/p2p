#include "HttpGet.h"
#include "Util.h"

#ifdef SM_DBG
#define HTTPGET_PRT(fmt, arg...) //fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "HttpGet",__LINE__, ##arg)
#else
#define HTTPGET_PRT(fmt, arg...)
#endif

#ifdef SM_VOD
#define HTTP_MAX_BODY_SIZE (2*1024*1024)
#endif




HttpGet::HttpGet(void)
: Speaker<HttpGetListener>(1)
, m_jumping(false)
, m_state(0)
, m_body(NULL)
, m_body_size(0)
, m_recv_size(0)
#ifdef SM_VOD
,m_nocontenlen(false)
#endif
{
	m_peer.add_listener(this);
}

HttpGet::~HttpGet(void)
{
	assert(NULL==m_body);
	m_peer.remove_listener(this);
	assert(DISCONNECTED==m_peer.get_channel()->get_state());
}
void HttpGet::set_url(const string& url,const string& save_path)
{
	m_uif.url = url;
	Util::url_element_split(url,m_uif.server,m_uif.port,m_uif.cgi);
	m_uif.ip = 0;
	m_save_path = save_path;
}
int HttpGet::open_request()
{
	//DEBUGMSG("#HttpGet::open_request()=> %s \n",m_uif.cgi.c_str());
	if(m_uif.url.empty())
		return -1;
	close();
	unsigned int hip = 0;
	if(m_uif.ip)
		hip = m_uif.ip;
	else
	{
		string ip = Util::ip_explain_ex(m_uif.server.c_str(),15000);
		hip = Util::ip_atoh(ip.c_str());
	}
	if(0!=m_peer.connect(hip,m_uif.port,0))
	{
		m_peer.disconnect(); //�����detach
		return -1;
	}
	m_state = 1;
	return 0;
}
void HttpGet::close()
{
	if(DISCONNECTED!=m_peer.get_channel()->get_state())
		m_peer.disconnect();
}
void HttpGet::on(PeerListener::Connected,Peer* peer)
{
	m_recv_size = 0;
	File64::remove_file(m_save_path.c_str());
	//m_file.open(m_save_path.c_str(),F64_RDWR|F64_TRUN);
	char buf[128];
	if(80==m_uif.port)
		strcpy(buf,m_uif.server.c_str());
	else
		sprintf(buf,"%s:%d",m_uif.server.c_str(),m_uif.port);
	HTTPGET_PRT("------on connected buf=%s cgi=%s len=%d\n", buf,m_uif.cgi.c_str(), int(strlen(buf)+m_uif.cgi.length()));
	m_peer.send_request(buf,m_uif.cgi,0,-1,0);
}
void HttpGet::on(PeerListener::Disconnected,Peer* peer)
{
	//ע�⣺�����������ת
	if(m_jumping)
	{
		HTTPGET_PRT("on disconnected jump, this will return directly\n");
		m_jumping = false;
		return;
	}
	//m_file.close();
	if(m_body)
		m_body[m_recv_size] = '\0';
	HTTPGET_PRT("on disconnected stat=%d size=%d\n", m_state,m_recv_size);
	if(2==m_state || (m_body_size==-1 && m_recv_size>0))
	{
		//fire(HttpGetListener::Response(),this,0,m_body,m_recv_size);
		fire(HttpGetListener::Response(),this,0,m_save_path,m_body,m_recv_size);
	}
	else
	{
		//fire(HttpGetListener::Response(),this,-1,m_body,m_recv_size);
		fire(HttpGetListener::Response(),this,-1,m_save_path,m_body,m_recv_size);
	}
	m_state = 0;

	
	if(m_body)
	{
		delete[] m_body;
		m_body = NULL;
	}
}
void HttpGet::on(PeerListener::HttpHeader,Peer* peer,char* buf,int len)
{
	HttpPeer* httppeer = static_cast<HttpPeer*>(peer);
	string header = buf;
	int result = httppeer->get_server_response_code();
	
	if (301==result || 302 == result)
	{
		m_jumping = true;
		httppeer->disconnect();
		string newurl;
		httppeer->get_field("Location",newurl);
		//if (newurl.find("http://") != string::npos)
		{
			DEBUGMSG("#change url to: %s",newurl.c_str());
			fire(HttpGetListener::Url302(),this,newurl);
			set_url(newurl,m_save_path);
			open_request();
			HTTPGET_PRT("---on head- skip-head:%s\n", buf);
		}
		return;
	}
	if (result != 200)//����
	{
		httppeer->disconnect();
		return;
	}
	string text;
	httppeer->get_field("Content-Length",text);
	int contentsize = atol(text.c_str());
	
#ifdef SM_MODIFY 
	// for letv source, have not Content-Length
	if(contentsize>0)
		m_body_size = contentsize;
	 else {
		m_body_size = HTTP_MAX_BODY_SIZE;
		contentsize = m_body_size;
		m_nocontenlen = true;
	}
	if(contentsize>HTTP_MAX_BODY_SIZE || contentsize<=0) {
		httppeer->disconnect();
	}
#else
	if(contentsize>0)
		m_body_size = contentsize;
	else
		m_body_size = 1024000;
	if(contentsize>1024000 || contentsize<=0)
	{
		httppeer->disconnect();
	}
#endif /* end of SM_MODIFY */	
	
	m_body = new char[m_body_size+1];
	HTTPGET_PRT("-----on head-getheadlen=%d buf=%s\n", contentsize, buf);
	m_recv_size = 0;
}
void HttpGet::on(PeerListener::Data,Peer* peer,char* buf,int len)
{
	HTTPGET_PRT("on data------%s\n", buf);
	//�����Сδ֪�ֳ���1M���ݵĻ���֧��.�����������Ƚ��ѷ���
	if(m_recv_size+len>m_body_size)
	{
		assert(false);
		len = m_body_size-m_recv_size;
	}
	memcpy(m_body+m_recv_size,buf,len);
	m_recv_size += len;
	//if(m_file.is_open())
	//	m_file.write(buf,len);
#ifdef SM_MODIFY
	// this only for source that http response have no Content-Length
	if(true==m_nocontenlen) {
		// vod will have endlist, live have no, how to decide the end of live, T.B.D
		char *p = strstr(m_body, "#EXT-X-ENDLIST");
		if(p) {
			m_nocontenlen = false;
			m_state = 2;
			m_peer.disconnect();
			return;
		}
	}
#endif /* end of SM_MODIFY */
	if(m_recv_size == m_body_size)
	{
		m_state = 2;
		m_peer.disconnect();
	}
}