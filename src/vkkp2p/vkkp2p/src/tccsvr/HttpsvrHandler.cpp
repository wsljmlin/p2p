#include "HttpsvrHandler.h"

#include "HttpServer.h"
#include "HttpResponseHeader.h"
#include "HttpContentType.h"
#include "UrlCode.h"
#include "Util.h"


HttpServer g_httpsvr;
bool g_bopen = false;
bool g_is_exit = false;  //用于linux版本通过http可以控制P2P程序退出

int handle_https_req(HttpRequest_t* req);


void response_cgi(int fd,const string& cgi,const string& head);
void response_stun_checkstate(int fd,const string& cgi,const string& head);
//*******************************************************
int httpsvr_start(int port,const char* ip/*=NULL*/)
{
	if(g_bopen)
		return 0;
	if(0==g_httpsvr.open(port,ip))
	{
		g_httpsvr.SetHandleReqFun(handle_https_req);
		g_bopen = true;
		return 0;
	}
	return -1;
}
int httpsvr_stop()
{
	if(g_bopen)
	{
		g_httpsvr.stop();
		g_bopen = false;
	}
	return 0;
}
int handle_https_req(HttpRequest_t* req)
{
	int fd = req->fd;
	string strline = req->header;
	string cgi     = req->cgi;
	cgi += req->params;

#define IFM(M,CGI) if(0==strcmp(M,req->method) && strstr(req->cgi,CGI))
#define EIFM(M,CGI) else if(0==strcmp(M,req->method) && strstr(req->cgi,CGI))
#define IF(CGI) if(strstr(req->cgi,CGI))
#define EIF(CGI) else if(strstr(req->cgi,CGI))


	//IFM("GET","/checkstate.do")
	//{
	//}
	//else
	{
		response_cgi(fd,cgi,strline);
	}
	return 0;
}

void response_cgi(int fd,const string& cgi,const string& head)
{
	//目前只当他为文件
	string path = ".";
	path += cgi;
	int pos = (int)path.find("?");
	if(pos>0)
		path.erase(pos);
	HttpClient::response_file(fd,path);
}
