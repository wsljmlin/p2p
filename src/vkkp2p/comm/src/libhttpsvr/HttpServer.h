#pragma once

#include "HttpClient.h"

class HttpServer
{
public:
	HttpServer(void);
	~HttpServer(void);

public:
	void SetHandleReqFun(FUN_HANDLE_HTTP_REQ_PTR fun) {m_fun_handle_http_req=fun;}
	int open(unsigned short port,const char *ip=0,bool multi_thread=true);
	int stop();
	void wait(unsigned long milliseconds=(unsigned long)-1);
#ifdef _WIN32
	static unsigned long __stdcall _startT(void *p);
#else
	static void* _startT(void *p);
#endif
	int run();
private:
	char m_ip[64];
	bool m_brun;
#ifdef _WIN32
	void * m_hthread;
#else
	pthread_t m_hthread;
#endif
	int m_sock;
	bool m_multi_thread;
	FUN_HANDLE_HTTP_REQ_PTR m_fun_handle_http_req;
};
