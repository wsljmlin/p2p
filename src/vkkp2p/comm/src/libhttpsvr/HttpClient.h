#pragma once

#ifdef _WIN32
#include <winsock2.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#ifdef _ECOS_8203
#include <gsl.h>
#include <gsl/gl_task.h>
#else
#include <pthread.h>
#endif
#endif

#include "basetypes.h"

#define HTTP_MAX_HEADLEN 2047
typedef struct tagHttpRequest
{
	int fd;
	char header[HTTP_MAX_HEADLEN+1];
	char cgi[HTTP_MAX_HEADLEN+1];
	char params[HTTP_MAX_HEADLEN+1];
	char method[8];  //GET,POST
	unsigned long long Content_Length;
	char body[HTTP_MAX_HEADLEN+1];
	unsigned int bodylen;

	struct in_addr addr;

	tagHttpRequest(void)
	{
		reset();
	}
	void reset()
	{
		fd = 0;
		header[0] = '\0';
		cgi[0] = '\0';
		params[0] = '\0';
		method[0] = '\0';
		Content_Length = 0;
		body[0] = '\0';
		bodylen = 0;
	}
}HttpRequest_t;

typedef int (*FUN_HANDLE_HTTP_REQ_PTR)(HttpRequest_t*);
class HttpClient 
{
private:
	HttpClient(SOCKET sock,sockaddr_in &addr,FUN_HANDLE_HTTP_REQ_PTR fun);
	~HttpClient(void);

public:
	//static void SetHandleReqFun(FUN_HANDLE_HTTP_REQ_PTR fun);
	static HttpClient* get_httpclient(SOCKET sock,sockaddr_in &addr,FUN_HANDLE_HTTP_REQ_PTR fun){
		return new HttpClient(sock,addr,fun);
	}
	int open(){

#ifdef _WIN32
		DWORD threadId;
		HANDLE h = CreateThread(NULL,0,_openT,this,0,&threadId);
		if(h)
			CloseHandle(h);
		else
			return -1;
#elif defined(_ECOS_8203)
		GL_Task_t handle;
		if(GL_FAILURE == GL_TaskCreate(0,_openT,(void*)this,4,102400,FALSE,&handle))
			return -1;
		GL_TaskActivate(handle);
#else
		pthread_t hthread;
		if(0!=pthread_create(&hthread,NULL,_openT,(void*)this))
			return -1;
		pthread_detach(hthread);
#endif
		return 0;
	}

	int svc();

#ifdef _WIN32
	static DWORD __stdcall _openT(LPVOID p){
		HttpClient *tt = (HttpClient*)p;
		if(tt)
			tt->svc();
		return 0;
	}
#elif defined(_ECOS_8203)
	static void _openT(void* p){
		HttpClient *tt = (HttpClient*)p;
		if(tt)
			tt->svc();
		GL_TaskSelfDelete();
	}
#else
	static void* _openT(void* p){
		HttpClient *tt = (HttpClient*)p;
		if(tt)
			tt->svc();
		//pthread_exit(NULL); //valgrind 会导致有28B 可访问的泄漏问题
		return 0;
	}
#endif


	void close(){
		if(m_sock)
		{
			::closesocket(m_sock);
			m_sock = INVALID_SOCKET;
		}
		delete this;
	}
	static long long atoll(const char* _Str);
	static string& string_trim(string& str,char c=' ');
	static string get_string_index(const string& source,int index,const string& sp);
	static string url_get_name(const string& url);
	static string url_get_parameter(const string& url,const string& parameter);
	static string url_get_last_parameter(const string& url,const string& parameter);
#ifdef SM_MODIFY
	static string url_get_parameter(const string& url,const string& startparameter, const string &endparameter);
#endif
	static int get_header_field(const string& header,const string& session, string& text);
	static bool is_keeplive(const string& header);

	static int socket_readible(int fd,DWORD dwTimeout = 100);
	static int send_n(int fd,const char *buf,int size);
	static int recv_n(int fd,char *buf,int size);
	static void response_error(int fd,const char* msg=NULL,int code=404);
	static void response_message(int fd,const char* msg,int len=-1,int code=200);
	static void response_message(int fd,const string& msg,int code=200);
	static void response_file(int fd,const string& path);
private:
	void handle_request();
	int recv_head(HttpRequest_t* req);
	int  handle_head(HttpRequest_t* req);


private:
	SOCKET m_sock;
	sockaddr_in m_addr;
	FUN_HANDLE_HTTP_REQ_PTR m_fun_handle_http_req;
};
extern int g_httpclient_amount;
extern bool g_https_exiting;
