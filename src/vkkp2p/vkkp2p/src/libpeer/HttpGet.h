#pragma once
#include "HttpPeer.h"

class HttpGet;
class HttpGetListener
{
public:
	virtual ~HttpGetListener(void){}

	template<int I>
	struct S{enum{T=I};};

	typedef S<1> Response;
	typedef S<2> Url302;
	//virtual void on(Response,HttpGet* hg,int result,const char* rsp,int len){}
	virtual void on(Response,HttpGet* hg,int result,const string& save_path,const char* rsp,int len){}
	virtual void on(Url302,HttpGet* hg,const string& newurl){}
};

typedef struct tagUrlInfo
{
	string				url;
	string				server;
	unsigned int		ip;
	unsigned short		port;
	string				cgi;
	tagUrlInfo(void):ip(0),port(0) {}
}UrlInfo;
class HttpGet : public PeerListener
	,public Speaker<HttpGetListener>
{
public:
	HttpGet(void);
	virtual ~HttpGet(void);

public:
	void set_url(const string& url,const string& save_path);
	int open_request();
	void close();

	virtual void on(PeerListener::Connected,Peer* peer);
	virtual void on(PeerListener::Disconnected,Peer* peer);

	virtual void on(PeerListener::HttpHeader,Peer* peer,char* buf,int len);
	virtual void on(PeerListener::Data,Peer* peer,char* buf,int len);
protected:
	HttpPeer			m_peer;
	UrlInfo				m_uif;
	bool				m_jumping;
	int					m_state;
	string				m_save_path;
	//File64				m_file;
	char*				m_body;
	int				m_body_size;
	int				m_recv_size;
#ifdef SM_VOD
	bool m_nocontenlen;
#endif
};
