
#include "HttpClient.h"
#include "HttpResponseHeader.h"
#include "HttpContentType.h"
#include "UrlCode.h"
#include "File64.h"

int g_httpclient_amount=0;
bool g_https_exiting=false;

//FUN_HANDLE_HTTP_REQ_PTR fun_handle_http_req = NULL;

#ifdef SM_DBG
#define HTTP_CLIENT_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "HttpClient", __LINE__, ##arg)
#else
#define HTTP_CLIENT_PRT(fmt, arg...) 
#endif

//*****************************************************

/////////////////////////////////////////////////////////////
long long HttpClient::atoll(const char* _Str)
{
	long long i=0;
	if(NULL==_Str)
		return 0;
	if(1!=sscanf(_Str,"%lld",&i))
		return 0;
	return i;
}
/////////////////////////////////////////////////////////////
string HttpClient::get_string_index(const string& source,int index,const string& sp)
{
	if(sp.empty() || index<0)
		return source;

	int splen = (int)sp.length();
	int pos1=0-splen,pos2=0-splen;

	int i=0;
	for(i=0;i<(index+1);i++)
	{
		pos1 = pos2+splen;
		pos2 = (int)source.find(sp,pos1);
		if(pos2<0)
			break;
	}
	if(i<index)
		return "";

	pos2 = pos2<pos1?(int)source.length():pos2;
	return source.substr(pos1,pos2-pos1);

}
string& HttpClient::string_trim(string& str,char c/*=' '*/)
{
	int pos1=0,pos2=(int)str.length()-1;
	for(; pos1<=pos2 && c==str.at(pos1);++pos1);//全空的话超越边界
	for(; pos2>pos1 && c==str.at(pos2);--pos2);//上面已经判断过=,此处不再
	if( pos1>pos2)
		str = "";
	else
		str = str.substr(pos1,pos2-pos1+1);
	return str;
}
string HttpClient::url_get_name(const string& url)
{
	if(url.empty())
		return "";
	int n = 0;
	string str = url;
	n = (int)str.find("?");
	if(n>=0)
		str.erase(n); //后面参数可能带有/,所以先去掉参数部分
	n = (int)str.rfind('/');
	return str.substr(n+1);
}
string HttpClient::url_get_parameter(const string& url,const string& parameter)
{
	if(url.empty() || parameter.empty())
		return "";
	int pos;
	string str;
	str = parameter;
	str += "=";
	pos = (int)url.find(str);
	if(pos < 0)
		return "";
	str = url.substr(pos + parameter.length()+1);

	pos = (int)str.find("&");
	if(pos >= 0)
		str = str.substr(0,pos);
	return str;
}

#ifdef SM_MODIFY
string HttpClient::url_get_parameter(const string& url,const string& startparameter, const string &endparameter)
{
	if(url.empty() || startparameter.empty() || endparameter.empty())
		return "";
	/* find the start */
	int pos_start;
	string str_start;
	str_start = startparameter;
	str_start += "=";
	pos_start = (int)url.find(str_start);
	if(pos_start < 0)
		return "";
	str_start = url.substr(pos_start + startparameter.length()+1);
	/* find the end */
	int pos_end;
	string str_end;
	str_end = endparameter;
	str_end += "=";
	pos_end = (int)str_start.find(str_end);
	
	if(pos_end >= 0)
		str_start = str_start.substr(0,pos_end-1);
	return str_start;
}
#endif

//获取URL参数，假设url参数为cgi中的最后一个参数
string HttpClient::url_get_last_parameter(const string& url,const string& parameter)
{
	string val;
	string str;
	str = parameter;
	str += "=";
	int pos = (int)url.find(str);
	if(pos>0)
	{
		val = url.substr(pos+4);
	}
	return val;
}

int HttpClient::get_header_field(const string& header,const string& session, string& text)
{
	//取得某个域值,session 不带":"号
	if(header.empty()) 
		return -1;
	int nPos = -1;
	nPos = (int)header.find(session,0);
	if(nPos != -1)
	{
		nPos += (int)session.length();
		nPos += 1; //加1忽略:号
		int nCr = (int)header.find("\r\n",nPos);
		text = header.substr(nPos,nCr - nPos);
		string_trim(text);
		return 0;
	}
	else
	{
		return -1;
	}
}
bool HttpClient::is_keeplive(const string& header)
{
	//是否keep-alive
	string str;
	if(0==get_header_field(header,"Connection",str))
	{
		if(0==stricmp(str.c_str(),"keep-alive"))
			return true;
	}
	return false;
}
int HttpClient::socket_readible(int fd,DWORD dwTimeout/* = 100*/)
{
	//assert(fd != INVALID_SOCKET);

	timeval timeout;
	timeout.tv_sec = dwTimeout / 1000;
	timeout.tv_usec = dwTimeout % 1000;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	int nStatus = ::select(fd+1, &fds, NULL, NULL, &timeout);
	if (nStatus != SOCKET_ERROR)
	{
		if(FD_ISSET(fd,&fds))
			return 1;
		return 0;
	}
	return -1;
}
int HttpClient::send_n(int fd,const char *buf,int size)
{
	int pos=0;
	int ret=0;
	while(pos<size)
	{
		ret = send(fd,buf+pos,size-pos,0);
		if(ret>0)
		{
			pos += ret;
		}
		else
		{
			return -1;
		}
	}
	return 0;
}
int HttpClient::recv_n(int fd,char *buf,int size)
{
	int pos=0;
	int ret=0;
	while(pos<size)
	{
		ret = recv(fd,buf+pos,size-pos,0);
		if(ret>0)
		{
			pos += ret;
		}
		else
		{
			return -1;
		}
	}
	return 0;
}
//*****************************************************
HttpClient::HttpClient(SOCKET sock,sockaddr_in &addr,FUN_HANDLE_HTTP_REQ_PTR fun) 
:m_sock(sock)
,m_fun_handle_http_req(fun)
{
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sin_addr.s_addr = addr.sin_addr.s_addr;
	m_addr.sin_family = addr.sin_family;
	m_addr.sin_port = addr.sin_port;

	DEBUGMSG(".........new request \n");
	g_httpclient_amount++;
}

HttpClient::~HttpClient(void)
{
	DEBUGMSG(".........~~~end request \n");
	g_httpclient_amount--;
}


//void HttpClient::SetHandleReqFun(FUN_HANDLE_HTTP_REQ_PTR fun)
//{
//	fun_handle_http_req = fun;
//}
int HttpClient::svc()
{
	if(INVALID_SOCKET == m_sock)
		return 0;

	handle_request();

	close();
	return 0;
}

void HttpClient::handle_request()
{
	
	HttpRequest_t* req = new HttpRequest_t();
	do
	{
		//只支持Get 方法头，不支持头含数据，也就是收到"\r\n\r\n"后认为是一个完整请求
		req->reset();
		req->addr.s_addr = m_addr.sin_addr.s_addr;

		req->fd = (int)m_sock;
		if(0!=recv_head(req))
			break;

		//如果body数据比较少，则收完它
		if(req->Content_Length>req->bodylen && req->Content_Length<=HTTP_MAX_HEADLEN)
		{
			if(0!=recv_n(req->fd,req->body+req->bodylen,(int)(req->Content_Length-req->bodylen)))
				break;
			req->bodylen = (int)req->Content_Length;
		}
		req->body[req->bodylen] = '\0';
		HTTP_CLIENT_PRT("head=%s\n", req->header);
		HTTP_CLIENT_PRT("parms=%s\n", req->params);

		if(0!=handle_head(req)) //返回0才表示正确完整处理，才可能继续循环处理(如果keepalive)
			break;
	}while(is_keeplive(req->header));
	delete[] req;
}
int HttpClient::recv_head(HttpRequest_t* req)
{
	int n = 0;
	int readsize = 0;
	DWORD begintick = GetTickCount();
	char *p = NULL;
	char* buf = req->header;
	while(readsize<HTTP_MAX_HEADLEN)
	{
		if(1==socket_readible((int)m_sock))
		{
			n = recv(m_sock,buf+readsize,HTTP_MAX_HEADLEN-readsize,0);
			if(n<=0)
			{
				break;
			}
			readsize+=n;
			buf[readsize] = '\0';
			p = strstr(buf,"\r\n\r\n");
			if(p)
			{
				req->bodylen = readsize - (int)(p-buf+4);
				assert(req->bodylen>=0);
				if(req->bodylen>0)
				{
					memcpy(req->body,p+4,req->bodylen);
				}
				p[4] = '\0';
				return 0;
			}
		}

		//超时 10秒
		if((begintick + 10000) < GetTickCount())
			break;
	}
	return -1;
}
int HttpClient::handle_head(HttpRequest_t* req)
{
	//DEBUGMSG("request------- \n %s \n",header);
	string text = "";
	if(0==get_header_field(req->header,"Content-Length",text))
	{
		req->Content_Length = atoll(text.c_str());
	}
	char buf[HTTP_MAX_HEADLEN+1];
	char* ptr = strchr(req->header,'\r');
	if(NULL==ptr) return -1;
	memcpy(buf,req->header,(int)(ptr-req->header));
	buf[(int)(ptr-req->header)] = '\0';
	string src = buf;
	string str;
	str = get_string_index(src,0," ");
	if(str.empty()||str.length()>7) return -1;
	strcpy(req->method,str.c_str());
	str = get_string_index(src,1," ");
	if(str.empty()||str.length()>HTTP_MAX_HEADLEN) return -1;
	str = UrlDecode(str);
	int pos = (int)str.find("?");
	if(pos>0)
	{
		strcpy(req->cgi,str.substr(0,pos).c_str());
		strcpy(req->params,str.substr(pos).c_str());
	}
	else if(pos==0)
	{
		return -1;
	}
	else
	{
		strcpy(req->cgi,str.c_str());
		req->params[0]='\0';
	}

	if (m_fun_handle_http_req)
	{
		return m_fun_handle_http_req(req);
	}
	else
	{
		response_error((int)m_sock);
		return -1;
	}
}



void HttpClient::response_message(int fd,const char* msg,int len/*=-1*/,int code/*=200*/)
{
	HttpResponseHeader responseHdr;
	if(len<0)
	{
		if(msg)
			len = (int)strlen(msg);
		else
			len = 0;
	}
	if(0==len) code = 204;
    
    SYSTEMTIME st;
	GetLocalTime(&st);

	responseHdr.AddStatusCode(code);
    responseHdr.AddDate(st);
    responseHdr.AddServer("sphttpsvr");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength(len);
    responseHdr.AddContentType("text/html");

    responseHdr.Send(fd);
	if(len>0)
		send(fd,msg,len,0);
}
void HttpClient::response_message(int fd,const string& msg,int code/*=200*/)
{
	response_message(fd,msg.c_str(),(int)msg.length(),code);
}
void HttpClient::response_error(int fd,const char* msg/*=NULL*/,int code/*=404*/)
{
	printf("response %d",code);

	HttpResponseHeader responseHdr;

	string str = "error";
	if(msg)
		str = msg;
    
    SYSTEMTIME st;
	GetLocalTime(&st);

	responseHdr.AddStatusCode(code);
    responseHdr.AddDate(st);
    responseHdr.AddServer("sphttpsvr");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength((int)str.length());
    responseHdr.AddContentType("text/html");

    responseHdr.Send(fd);
	send(fd,str.c_str(),(int)str.length(),0);
}
void HttpClient::response_file(int fd,const string& path)
{
	File64 file;
	if(0!=file.open(path.c_str(),F64_READ))
	{
		HttpClient::response_error(fd);
		return;
	}
	size64_t size = file.seek(0,SEEK_END);
	if(size<=0)
	if(size <= 0)
	{
		HttpClient::response_error(fd);
		return;
	}
	file.seek(0,SEEK_SET);

	HttpResponseHeader responseHdr;
	SYSTEMTIME tm;
	GetLocalTime(&tm);

	responseHdr.AddStatusCode(200);
    responseHdr.AddDate(tm);
    responseHdr.AddServer("sphttpsvr");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength((long long)size);
    responseHdr.AddContentType("text/html");
	responseHdr.Send(fd);

	char *buf=new char[4096];
	size64_t rdsize = 0;
	//
	int n = 0;
	while(rdsize<size)
	{
		n = file.read(buf,4096);
		if(n<=0)
			break;
		if(0!=HttpClient::send_n(fd,buf,n))
			break;
		rdsize += n;
	}
	delete[] buf;
	file.close();
	HttpClient::socket_readible(fd,10000);//最多等10秒钟，当对方收完关闭时，会可读。
}

