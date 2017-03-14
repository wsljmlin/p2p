#pragma once
#include "basetypes.h"

class Httpc
{
private:
	Httpc(void);
	~Httpc(void);

	
public:
	//有data时使用post,
	static int request(const string& url,const char* body,int bodylen,string& rspheader,string& rspbody);
	static int download_file(const string& url,const string& filepath);

	//static int http_download_file(const string& url,const string& filepath);
	//static int http_request(const string& url,const string& req_sign,string& strret,string& rsp_sign,int *perr=NULL);
	static int http_get(const string& url,char* sret,int retlen);

public:
	static int get_field(const string& header,const string& session, string& text);
	static int url_element_split(const string& url,string& server,unsigned short& port,string& cgi);
	//static int format_header1(string& header,const string& server,const string& cgi,int ibegin=0,int iend=-1,bool bkeepalive=false);
	static int format_header(string& header,const string& server,const string& cgi,int bodylen);
	static int get_server_response_code(const string& header);

	static string ip_format(const char* ip_or_dns);
	static int send_n(int sock,const char *buf,int size);

#ifdef _WIN32
	static DWORD WINAPI ip_explain_ex_t1(void *p);
#else
	static void* ip_explain_ex_t1(void *p);
#endif
	static string ip_explain(const char* s);
	static string ip_explain_ex(const char* s,int maxTick=10000);
	
};
