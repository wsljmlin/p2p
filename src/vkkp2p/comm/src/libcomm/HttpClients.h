#pragma once
#include "TCPChannel.h"
#include "ntypes.h"

class HttpClients;

class HttpClientListener
{
public:
	virtual ~HttpClientListener(void){}

	template<int I>
	struct S{enum{T=I};};
	
	typedef S<2> HttpConnected;
	typedef S<3> HttpDisconnected;
	typedef S<4> HttpResponseHeader;
	typedef S<5> HttpResponseBody;

	virtual void on(HttpConnected,HttpClients* h){}
	virtual void on(HttpDisconnected,HttpClients* h){}
	virtual void on(HttpResponseHeader,HttpClients* h,const char* header){}
	virtual void on(HttpResponseBody,HttpClients* h,const char* buf,int len){}
};


class HttpClients : public ChannelListener
	,public Speaker<HttpClientListener>
{
public:
	HttpClients(void);
	~HttpClients(void);
	enum {
		HTTP_DISCONNECTED=0,
		HTTP_READY,				//当前为可发送请求状态
		HTTP_RSPONSE_HEADER,	//当前为接收header状态，发送完请求之后
		HTTP_RSPONSE_BODY		//当前为接收body状态
	};
public:
	int connect(const char* ip,unsigned short port);
	int connect(unsigned int ip,unsigned short port);
	int disconnect();

	int send_simple_req_cgi(const char* cgi,const char* server); //server= 域名:port
	int send_request_header(const char* reqheadbuf);
public:
	virtual void on(Connected,Channel* ch);
	virtual void on(Disconnected,Channel* ch);
	virtual void on(Readable,Channel* ch,const int& wait);

private:
	void on_recv_header();
	void on_recv_body(const char* buf,int len);

	static int get_response_code(const char* szHead);
	static int get_field(const char* szHead,const char *szSession, char* szTxt,int nTxt);
private:
	int m_state;
	TCPChannel m_ch;
	char* m_headbuf;
	char* m_bodybuf;
	int m_headbuf_recv_size;
	uint64 m_body_size;
	uint64 m_body_recv_size;
};

