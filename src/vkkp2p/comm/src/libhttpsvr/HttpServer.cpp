
#include "HttpServer.h"

#ifdef _WIN32
typedef int socklen_t;
#endif


HttpServer::HttpServer(void)
: m_brun(false)
, m_hthread(0)
, m_sock(INVALID_SOCKET)
, m_multi_thread(true)
, m_fun_handle_http_req(NULL)
{
}

HttpServer::~HttpServer(void)
{
	if(m_hthread)
	{
#ifdef _WIN32
		CloseHandle(m_hthread);
#endif
		m_hthread = 0;
	}
}
int HttpServer::open(unsigned short port,const char *ip/*=NULL*/,bool multi_thread/*=true*/)
{
	if(m_brun)
		return -1;
	if(ip)
		strcpy(m_ip,ip);
	else
		memset(m_ip,0,64);

	m_multi_thread = multi_thread;
	m_sock = (int)socket(AF_INET,SOCK_STREAM,0);
	if(m_sock == INVALID_SOCKET)
	{
		return -1;
	}
	//设置端口重用
	int flag = 1;
	if(-1==setsockopt(m_sock,SOL_SOCKET,SO_REUSEADDR,(char*)&flag,sizeof(flag)))
	{
		perror("set reuseaddr faild!");
	}

	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(0==strlen(m_ip))
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		addr.sin_addr.s_addr = inet_addr(m_ip);

	if(SOCKET_ERROR == bind(m_sock,(sockaddr*)&addr,sizeof addr))
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return -1;
	}

	if(SOCKET_ERROR == listen(m_sock,SOMAXCONN))
	//if(SOCKET_ERROR == listen(m_sock,5))
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		perror("#*** listen fail:");
		return -1;
	}

	m_brun = true;
#ifdef _WIN32
	DWORD threadId;
	m_hthread = CreateThread(NULL,0,_startT,this,0,&threadId);
#else
	if(0 != pthread_create(&m_hthread,NULL,_startT,(void*)this))
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		m_hthread = 0;
		m_brun = false;
		return -1;
	}
	//pthread_detach(m_hthread); //让线程退出时自动回收资源,执行此句的话pthread_join()无效
#endif
	
	DEBUGMSG("#:---https open (%s:%d)--- \n",m_ip,port);
	return 0;
}
int HttpServer::stop()
{
	if(!m_brun)
		return -1;
	m_brun = false;
	g_https_exiting = true;
	if(INVALID_SOCKET != m_sock)
	{
#ifndef _WIN32
		shutdown(m_sock,SHUT_RDWR);
#endif
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}

	wait();

	while(g_httpclient_amount)
		Sleep(200);
	return 0;
}
void HttpServer::wait(unsigned long milliseconds/*=INFINITE*/)
{
	if(m_hthread)
	{
#ifdef _WIN32
		if(WAIT_OBJECT_0 == WaitForSingleObject(m_hthread,milliseconds))
		{
			CloseHandle(m_hthread);
			m_hthread = 0;
		}
#else
		pthread_join(m_hthread,NULL);
		m_hthread = 0;
#endif
	}
}
#ifdef _WIN32
unsigned long __stdcall HttpServer::_startT(void *p)
#else
void* HttpServer::_startT(void *p)
#endif
{
	HttpServer *psvr = (HttpServer*)p;
	if(psvr)
	{
		psvr->run();
	}
	return 0;
}

int HttpServer::run()
{
	while(m_brun && INVALID_SOCKET!=m_sock)
	{
		SOCKET sock_client;
		sockaddr_in addr_client;
		memset(&addr_client,0,sizeof(addr_client));
		socklen_t addr_len = sizeof addr_client;
		sock_client = accept(m_sock,(sockaddr*)&addr_client,&addr_len);
		if(INVALID_SOCKET != sock_client)
		{
			HttpClient *pHttp = HttpClient::get_httpclient(sock_client,addr_client,m_fun_handle_http_req);
			if(pHttp)
			{
				if(m_multi_thread)
				{
					if(0!=pHttp->open())
					{
						pHttp->close();
					}
				}
				else
					pHttp->svc();
			}
			else
				closesocket(sock_client);
		}
	}
	return 0;

}

