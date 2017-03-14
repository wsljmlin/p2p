
#include "Httpc.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "File64.h"

#ifdef _WIN32
#include <winsock2.h>
#pragma warning(disable:4996)
#else

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h> 

#include <pthread.h>
#endif

#define HTTPC_MAX_HEADLEN 2047
typedef struct tag_httpc_response
{
	char header[HTTPC_MAX_HEADLEN+1];
	int retcode;
	unsigned long long Content_Length;
	char body[HTTPC_MAX_HEADLEN+1]; //记录多数的数据
	unsigned long long bodylen;
	char *pbody;
	tag_httpc_response(void)
		:retcode(0)
		,Content_Length(0)
		,bodylen(0)
		,pbody(NULL)
	{
	}
	~tag_httpc_response(void)
	{
		if(pbody)
			delete[] pbody;
	}
	
}httpc_response_t;

static int httpc_set_blocking(SOCKET s,bool bBlocking);
static int httpc_is_connected( SOCKET s, fd_set *rd, fd_set *wr, fd_set *ex);
static long long httpc_atoll(const char* _Str)
{
	long long i=0;
	if(NULL==_Str)
		return 0;
	if(1!=sscanf(_Str,"%lld",&i))
		return 0;
	return i;
}

SOCKET httpc_send_request(const string& url,const char* data,int datalen);
int httpc_recv_header(SOCKET fd,httpc_response_t* rsp);

Httpc::Httpc(void)
{
}

Httpc::~Httpc(void)
{
}
int Httpc::request(const string& url,const char* body,int bodylen,string& rspheader,string& rspbody)
{
	int n = 0;
	httpc_response_t rsp;
	SOCKET fd = httpc_send_request(url,body,bodylen);
	if(INVALID_SOCKET==fd)
		return -1;
	do
	{
		if(0!=httpc_recv_header(fd,&rsp))
			break;
		if(rsp.Content_Length>10240000)
			break;
		rsp.pbody = new char[(int)rsp.Content_Length+2];
		if(rsp.bodylen>0)
			memcpy(rsp.pbody,rsp.body,(int)rsp.bodylen);
		while(rsp.bodylen<rsp.Content_Length)
		{
			n = recv(fd,rsp.pbody+(int)rsp.bodylen,(int)(rsp.Content_Length-rsp.bodylen),0);
			if(n<=0)
				break;
			rsp.bodylen += n;
		}
		break;
	}while(0);
	closesocket(fd);
	rsp.pbody[rsp.bodylen] = '\0';
	if(rsp.bodylen == rsp.Content_Length && 200==rsp.retcode)
	{
		rspheader = rsp.header;
		rspbody = rsp.pbody;
		return 0;
	}
	return -1;
}
int Httpc::download_file(const string& url,const string& filepath)
{
	File64 file;
	int n = 0;
	httpc_response_t rsp;
	SOCKET fd = httpc_send_request(url,NULL,0);
	if(INVALID_SOCKET==fd)
		return -1;
	do
	{
		if(0!=httpc_recv_header(fd,&rsp))
			break;
		if(200!=rsp.retcode||rsp.Content_Length<=0)
			break;
		
		if(0!=file.open(filepath.c_str(),F64_RDWR|F64_TRUN))
			break;
		if(rsp.bodylen>0)
		{
			if(0!=file.write_n(rsp.body,(int)rsp.bodylen))
				break;
		}
		while(rsp.bodylen<rsp.Content_Length)
		{
			n = HTTPC_MAX_HEADLEN+1;
			n = recv(fd,rsp.body,n,0);
			if(n<=0)
				break;
			if(0!=file.write_n(rsp.body,n))
				break;
			rsp.bodylen += n;
		}
		break;
	}while(0);
	closesocket(fd);

	if(rsp.bodylen == rsp.Content_Length && 200==rsp.retcode)
	{
		return 0;
	}
	return -1;
}

SOCKET httpc_send_request(const string& url,const char* body,int bodylen)
{
	string server,cgi,ip,header;
	unsigned short port;
	SOCKET sock = INVALID_SOCKET;
	//printf("#http request url=%s \n",url.c_str());
	int err = 0;
	do
	{
		Httpc::url_element_split(url,server,port,cgi);
		ip = Httpc::ip_explain_ex(server.c_str());
		Httpc::format_header(header,server,cgi,bodylen);
		sock = socket(AF_INET,SOCK_STREAM,0);
		if(INVALID_SOCKET==sock)
		{
			err = 2;
			break;
		}

		//设置超时:
#ifdef _WIN32
		int x = 20000;
#else
		struct timeval x;  
		x.tv_sec = 30;
		x.tv_usec = 0;
#endif
		if(-1==setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&x,sizeof(x)))
			perror("setsockopt SO_RCVTIMEO");
		if(-1==setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&x,sizeof(x)))
			perror("setsockopt SO_SNDTIMEO");

		sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		if(SOCKET_ERROR==connect(sock,(sockaddr*)&addr,sizeof(addr)))
		{
			err = 3;
			break;
		}

		if(0!=Httpc::send_n((int)sock,header.c_str(),(int)header.length()))
		{
			err = 4;
			break;
		}
		if(bodylen>0 && 0!=Httpc::send_n((int)sock,body,bodylen))
		{
			err = 5;
			break;
		}

	}while(0);
	if(0!=err)
	{
		if(sock!=INVALID_SOCKET)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
		}
		printf("***http request faild! url=%s \n",url.c_str());
	}
	return sock;
}
int httpc_recv_header(SOCKET fd,httpc_response_t* rsp)
{
	int n = 0;
	int readsize = 0;
	char *p = NULL;
	char* buf = rsp->header;
	bool bok = false;
	string str;
	while(readsize<HTTPC_MAX_HEADLEN)
	{
			n = recv(fd,buf+readsize,HTTPC_MAX_HEADLEN-readsize,0);
			if(n<=0)
			{
				return -1;
			}
			readsize+=n;
			buf[readsize] = '\0';
			p = strstr(buf,"\r\n\r\n");
			if(p)
			{
				rsp->bodylen = readsize - (int)(p-buf+4);
				assert(rsp->bodylen>=0);
				if(rsp->bodylen>0)
				{
					memcpy(rsp->body,p+4,(int)rsp->bodylen);
				}
				p[4] = '\0';
				bok = true;
				break;
			}
	}
	if(!bok)
		return -1;
	rsp->retcode = Httpc::get_server_response_code(rsp->header);
	if(0==Httpc::get_field(rsp->header,"Content-Length",str))
		rsp->Content_Length = httpc_atoll(str.c_str());
	else
		rsp->Content_Length = 0;
	return 0;
}
//************************************************************

//int Httpc::http_download_file(const string& url,const string& filepath)
//{
//	FILE *fp = NULL;
//	string server,cgi,ip,header;
//	unsigned short port;
//	SOCKET sock = INVALID_SOCKET;
//
//	int err = 0;
//	do
//	{
//		//fstream析构会自动关闭文件,所以可以不显式file.close()也行
//		fp = fopen(filepath.c_str(),"wb+");
//		if(!fp)
//		{
//			err = 1;
//			break;
//		}
//
//		url_element_split(url,server,port,cgi);
//		//ip = ip_format(server.c_str());
//		ip = ip_explain_ex(server.c_str());
//		format_header(header,server,cgi);
//		
//		sock = socket(AF_INET,SOCK_STREAM,0);
//		if(INVALID_SOCKET==sock)
//		{
//			err = 2;
//			break;
//		}
//
//		//设置超时:
//#ifdef _WIN32
//		int x = 20000;
//#else
//		struct timeval x;  
//		x.tv_sec = 30;
//		x.tv_usec = 0;
//#endif
//		if(-1==setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&x,sizeof(x)))
//			perror("setsockopt SO_RCVTIMEO");
//		if(-1==setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&x,sizeof(x)))
//			perror("setsockopt SO_SNDTIMEO");
//
//		sockaddr_in addr;
//		memset(&addr,0,sizeof(addr));
//		addr.sin_family = AF_INET;
//		addr.sin_port = htons(port);
//		addr.sin_addr.s_addr = inet_addr(ip.c_str());
//		if(SOCKET_ERROR==connect(sock,(sockaddr*)&addr,sizeof(addr)))
//		{
//			err = 3;
//			break;
//		}
//
//		if(0!=send_n((int)sock,header.c_str(),(int)header.length()))
//		{
//			err = 4;
//			break;
//		}
//
//		//接收头
//		string str;
//		char buf[2048];
//		int ret;
//		int len = 0;
//		int pos = 0;
//		memset(buf,0,2048);
//		char *ptr=NULL;
//		int data_size=0;
//		int file_size=0;
//		while(1)
//		{
//			ret = recv(sock,buf+len,2048-len,0);
//			if(ret<=0)
//			{
//				err = 5;
//				break;
//			}
//			len += ret;
//
//			ptr = strstr(buf,"\r\n\r\n");
//			if(ptr)
//			{
//				*(ptr+2) = '\0';
//				header = buf;
//				pos = (int)(ptr-buf) + 4;
//				ptr += 4;
//				break;
//			}
//			else
//			{
//				if(len>=2048)
//				{
//					err = 6;
//					break;
//				}
//			}
//		}
//		if(0!=err)
//			break;
//		assert(ptr&&pos<=len);
//		if(ptr && pos<len)
//		{
//			//有多余数据,写入文件
//			fwrite(ptr,len-pos,1,fp);
//			data_size += (len-pos);
//		}
//
//		//解释头:
//		if(200!=get_server_response_code(header))
//		{
//			err = 7;
//			break;
//		}
//		if(0==get_field(header,"Content-Length",str))
//		{
//			file_size = atoi(str.c_str());
//		}
//
//		//接收数据
//		while(file_size<=0||(file_size>0 && data_size<file_size))
//		{
//			ret = recv(sock,buf,2048,0);
//			if(ret<=0)
//			{
//				break;
//			}
//			fwrite(buf,ret,1,fp);
//			data_size += ret;
//		}
//
//		//如果收不够指定数据即删除
//		if((file_size>0 && data_size<file_size)||data_size==0)
//		{
//			err = 8;
//			break;
//		}
//	}while(0);
//
//	if(0!=err)
//	{
//		if(sock!=INVALID_SOCKET)
//		{
//			closesocket(sock);
//			sock = INVALID_SOCKET;
//		}
//		if(fp)
//		{
//			fclose(fp);
//			fp = NULL;
//		}
//		unlink(filepath.c_str());
//		printf("***http down faild\n");
//		return -1;
//	}
//
//	if(sock!=INVALID_SOCKET)
//	{
//		closesocket(sock);
//		sock = INVALID_SOCKET;
//	}
//	if(fp)
//	{
//		fclose(fp);
//		fp = NULL;
//	}
//	return 0;
//}
//
//int Httpc::http_request(const string& url,const string& req_sign,string& strret,string& rsp_sign,int *perr/*=NULL*/)
//{
//	string server,cgi,ip,header;
//	unsigned short port;
//	SOCKET sock = INVALID_SOCKET;
//	strret = "";
//	//printf("#http request url=%s \n",url.c_str());
//	int err = 0;
//	do
//	{
//		url_element_split(url,server,port,cgi);
//		//ip = ip_format(server.c_str());
//		ip = ip_explain_ex(server.c_str());
//		//format_sign_header(header,server,cgi,req_sign);
//		
//		sock = socket(AF_INET,SOCK_STREAM,0);
//		if(INVALID_SOCKET==sock)
//		{
//			err = 2;
//			break;
//		}
//
//		//设置超时:
//#ifdef _WIN32
//		int x = 20000;
//#else
//		struct timeval x;  
//		x.tv_sec = 30;
//		x.tv_usec = 0;
//#endif
//		if(-1==setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&x,sizeof(x)))
//			perror("setsockopt SO_RCVTIMEO");
//		if(-1==setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&x,sizeof(x)))
//			perror("setsockopt SO_SNDTIMEO");
//
//		sockaddr_in addr;
//		memset(&addr,0,sizeof(addr));
//		addr.sin_family = AF_INET;
//		addr.sin_port = htons(port);
//		addr.sin_addr.s_addr = inet_addr(ip.c_str());
//		if(SOCKET_ERROR==connect(sock,(sockaddr*)&addr,sizeof(addr)))
//		{
//			err = 3;
//			break;
//		}
//
//		if(0!=send_n((int)sock,header.c_str(),(int)header.length()))
//		{
//			err = 4;
//			break;
//		}
//
//		//接收头
//		string str;
//		char buf[2048];
//		int ret;
//		int len = 0;
//		int pos = 0;
//		memset(buf,0,2048);
//		char *ptr=NULL;
//		int data_size=0;
//		int file_size=0;
//		while(1)
//		{
//			ret = recv(sock,buf+len,2047-len,0);
//			if(ret<=0)
//			{
//				err = 5;
//				break;
//			}
//			len += ret;
//
//			ptr = strstr(buf,"\r\n\r\n");
//			if(ptr)
//			{
//				*(ptr+2) = '\0';
//				header = buf;
//				pos = (int)(ptr-buf) + 4;
//				ptr += 4;
//				break;
//			}
//			else
//			{
//				if(len>=2047)
//				{
//					err = 6;
//					break;
//				}
//			}
//		}
//		if(0!=err)
//			break;
//		assert(ptr&&pos<=len);
//		if(ptr && pos<len)
//		{
//			//有多余数据,写入文件
//			ptr[len-pos] = '\0';
//			strret = ptr;
//			data_size += (len-pos);
//		}
//
//		get_field(header,"SIGN",rsp_sign);
//		//解释头:
//		if(200!=get_server_response_code(header))
//		{
//			err = 7;
//			break;
//		}
//		if(0==get_field(header,"Content-Length",str))
//		{
//			file_size = atoi(str.c_str());
//		}
//
//		//接收数据
//		while(file_size<=0||(file_size>0 && data_size<file_size))
//		{
//			ret = recv(sock,buf,2047,0);
//			if(ret<=0)
//			{
//				break;
//			}
//			buf[ret] = '\0';
//			strret += buf;
//			data_size += ret;
//		}
//
//		//如果收不够指定数据即删除
//		if(file_size>0 && data_size<file_size)
//		{
//			err = 8;
//			break;
//		}
//
//	}while(0);
//	if(perr)
//		*perr = err;
//	if(0!=err)
//	{
//		if(sock!=INVALID_SOCKET)
//		{
//			closesocket(sock);
//			sock = INVALID_SOCKET;
//		}
//		printf("***http request faild! url=%s \n",url.c_str());
//		return -1;
//	}
//
//	if(sock!=INVALID_SOCKET)
//	{
//		closesocket(sock);
//		sock = INVALID_SOCKET;
//	}
//
//	return 0;
//}
int Httpc::http_get(const string& url,char* sret,int retlen)
{

	string server,cgi,ip,header;
	unsigned short port;
	SOCKET sock = INVALID_SOCKET;
	int ret;

	int err = 0;
	do
	{
		url_element_split(url,server,port,cgi);
		//ip = ip_format(server.c_str());
		ip = ip_explain_ex(server.c_str());
		format_header(header,server,cgi,0);
		
		sock = socket(AF_INET,SOCK_STREAM,0);
		if(INVALID_SOCKET==sock)
		{
			err = 2;
			break;
		}

		httpc_set_blocking(sock,false);
		//设置超时:
#ifdef _WIN32
		int x = 10000;
#else
		struct timeval x;  
		x.tv_sec = 10;
		x.tv_usec = 0;
#endif
		if(-1==setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&x,sizeof(x)))
			perror("setsockopt SO_RCVTIMEO");
		if(-1==setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&x,sizeof(x)))
			perror("setsockopt SO_SNDTIMEO");

		sockaddr_in addr;
		memset(&addr,0,sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(ip.c_str());
		if(0!=(ret=connect(sock,(sockaddr*)&addr,sizeof(addr))))
		{
#ifdef _WIN32
			int ierr = WSAGetLastError();
			if(WSAEWOULDBLOCK != ierr)
#else
			if(EINPROGRESS != errno)
#endif
			{
				err = 3;
				break;
			}
		}

		if (ret==0)
		{
			httpc_set_blocking(sock,true);
		}
		else
		{
			fd_set rdevents;
			fd_set wrevents;
			fd_set exevents;
			struct timeval y;  
			y.tv_sec = 30;
			y.tv_usec = 0;
			FD_ZERO(&rdevents); 
			FD_SET(sock, &rdevents);
			wrevents = rdevents;
			exevents = rdevents;
			ret = select((int)sock+1, &rdevents, &wrevents, &exevents, &y); 
			if (ret<0)
			{
				err = 8;
				break;
			}
			else if (ret==0)
			{
				err = 9;
				break;
			}
			else
			{
				if (httpc_is_connected(sock, &rdevents, &wrevents, &exevents ))
				{
					httpc_set_blocking(sock,true);
				}
				else
				{
					err = 10;
					break;
				}
			}
		
		}

		if(0!=send_n((int)sock,header.c_str(),(int)header.length()))
		{
			err = 4;
			break;
		}

		//接收头
		string str;
		char buf[2048];
		int ret;
		int len = 0;
		int pos = 0;
		memset(buf,0,2048);
		char *ptr=NULL;
		int data_size=0;
		int file_size=0;
		while(1)
		{
			ret = recv(sock,buf+len,2048-len,0);
			if(ret<=0)
			{
				err = 5;
				break;
			}
			len += ret;

			ptr = strstr(buf,"\r\n\r\n");
			if(ptr)
			{
				*(ptr+2) = '\0';
				header = buf;
				pos = (int)(ptr-buf) + 4;
				ptr += 4;
				break;
			}
			else
			{
				if(len>=2048)
				{
					err = 6;
					break;
				}
			}
		}
		if(0!=err)
			break;
		assert(ptr&&pos<=len);
		if(ptr && pos<len)
		{
			//有多余数据,写入文件
			int write_len = len-pos;
			if(write_len>(retlen-1))
				write_len = retlen-1;
			if(write_len>0)
			{
				memcpy(sret,ptr,write_len);
				data_size += write_len;
			}
		}

		//解释头:
		if(200!=get_server_response_code(header))
		{
			err = 7;
			break;
		}
		if(0==get_field(header,"Content-Length",str))
		{
			file_size = atoi(str.c_str());
			if(file_size>retlen)
				file_size = retlen-1;
		}
		else
		{
			file_size = retlen - 1;
		}


		//接收数据
		while(file_size<0||(file_size>0 && data_size<file_size))
		{
			ret = recv(sock,sret+data_size,file_size-data_size,0);
			if(ret<=0)
			{
				break;
			}
			data_size += ret;
		}
		sret[data_size] = '\0';

	}while(0);
	if(0!=err)
	{
		if(sock!=INVALID_SOCKET)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
		}

		printf("***http get faild\n");
		return -1;
	}

	if(sock!=INVALID_SOCKET)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
	}

	return 0;
}

int Httpc::url_element_split(const string& url,string& server,unsigned short& port,string& cgi)
{
	//解析url
	string str;
	int pos = 0,pos2 = 0, pos3 = 0;
	port = 80;
	server = "";
	cgi = "";

	pos = (int)url.find("http://",0);
	if(pos >= 0)
		pos += 7;
	else
	{
		pos = (int)url.find("HTTP://",0);
		if(pos >= 0)
			pos += 7;
		else
			pos = 0;
	}

	pos2 = (int)url.find(":",pos);
	pos3 = (int)url.find("/",pos);

	if(pos3 > 0 && pos2 > pos3)
		pos2 = -1;

	if(pos3 > pos)
	{
		if(pos2>pos)
		{
			server = url.substr(pos,pos2-pos);
			str = url.substr(pos2+1,pos3-pos2-1);
			port = atoi(str.c_str());
		}
		else
		{
			server = url.substr(pos,pos3-pos);
		}
		cgi = url.substr(pos3);
	}
	else
	{
		if(pos2>pos)
		{
			server = url.substr(pos,pos2-pos);
			str = url.substr(pos2+1);
			port = atoi(str.c_str());
		}
		else
		{
			server = url;
		}
		cgi = "/";
	}

	return 0;
}

//int Httpc::format_header1(string& header,const string& server,const string& cgi,int ibegin/*=0*/,int iend/*=-1*/,bool bkeepalive/*=false*/)
//{
//	header = "";
//	///第1行:方法,请求的路径,版本
//	header = "GET ";
//	header+=cgi;
//	header+=" HTTP/1.1";
//	header+="\r\n";
//
//	///第2行:主机
//	header+="Host: ";
//	header+=server;
//	header+="\r\n";
//
//	///第3行:
//	//if(pReferer != NULL)
//	//{
//	//	m_request_header+="Referer:";
//	//	m_request_header+=pReferer;
//	//	m_request_header+="\r\n";		
//	//}
//
//	///第4行:接收的数据类型
//	header+="Accept: */*\r\n";
//
//	///第5行:浏览器类型
//	header+="User-Agent: Mozilla/4.0 (compatible; httpc 1.0; )\r\n";
//
//	header += "Pragma: no-cache\r\n";
//	header += "Cache-Control: no-cache\r\n";
//
//
//	///第7行:Cookie.
//	//if(pCookie != NULL)
//	//{
//	//	m_request_header+="Set Cookie:0";
//	//	m_request_header+=pCookie;
//	//	m_request_header+="\r\n";
//	//}
//
//	///第8行:请求的数据起始字节位置(断点续传的关键)
//	if(ibegin == 0 && iend == -1)
//	{
//		//bKeepAlive = false;
//	}else
//	{
//		char szTemp[20];
//		header+="Range: bytes=";
//		//itoa(ibegin,szTemp,10);
//		sprintf(szTemp,"%d",ibegin);
//		header+=szTemp;
//		header+="-";
//		if(iend > ibegin)
//		{
//			//itoa(iend,szTemp,10);
//			sprintf(szTemp,"%d",iend);
//			header+=szTemp;
//		}
//		header+="\r\n";
//	}
//
//	///第6行:连接设置,保持
//	if (bkeepalive)
//	{
//		header+="Connection: Keep-Alive \r\n";
//	}else
//	{
//		header+="Connection: Close \r\n";
//	}
//
//	///最后一行:空行
//	header+="\r\n";
//	return 0;
//}
int Httpc::format_header(string& header,const string& server,const string& cgi,int bodylen)
{
	char buf[2048];
	header = "";
	sprintf(buf,"%s %s HTTP/1.1\r\n",bodylen>0?"POST":"GET",cgi.c_str());
	sprintf(buf+strlen(buf),"Host: %s\r\n",server.c_str());
	if(bodylen>0)
		sprintf(buf+strlen(buf),"Content-Length: %d\r\n",bodylen);
	//strcat(buf,"Accept: */*\r\n");
	//Accept-Encoding: gzip, deflate
	strcat(buf,"User-Agent: Mozilla/4.0 (compatible; httpc 1.0;)\r\n");
	strcat(buf,"Pragma: no-cache\r\n");
	strcat(buf,"Cache-Control: no-cache\r\n");
	strcat(buf,"Content-Type: application/x-www-form-urlencoded\r\n");
	//strcat(buf,"Connection: Close \r\n");
	strcat(buf,"\r\n");
	header = buf;
	return 0;
}
int Httpc::get_server_response_code(const string& header)
{
	int pos = (int)header.find(" ");
	if(pos<0)
		return -1;
	else
		return atoi(header.substr(pos+1).c_str());
}
int Httpc::get_field(const string& header,const string& session, string& text)
{
	//取得某个域值
	if(header.empty()) 
		return -1;
	int nPos = -1;
	nPos = (int)header.find(session,0);
	if(nPos != -1)
	{
		nPos += (int)session.length();
		nPos += 2;
		int nCr = (int)header.find("\r\n",nPos);
		text = header.substr(nPos,nCr - nPos);
		return 0;
	}
	else
	{
		return -1;
	}
}
string Httpc::ip_format(const char* ip_or_dns)
{
	if(INADDR_NONE != inet_addr(ip_or_dns))
	{
		return ip_or_dns;//就是ip
	}
	else
	{
		in_addr sin_addr;
		hostent* host;
		host = gethostbyname(ip_or_dns);
		if (host == NULL) {
			return ip_or_dns;
		}
		sin_addr.s_addr = *((unsigned long*)host->h_addr);
		return inet_ntoa(sin_addr);
	}
}
int Httpc::send_n(int sock,const char *buf,int size)
{
	int pos=0;
	int ret=0;
	while(pos<size)
	{
		ret = send(sock,buf+pos,size-pos,0);
		if(ret>0)
			pos += ret;
		else
			return -1;
	}
	return 0;
}

int httpc_set_blocking(SOCKET s,bool bBlocking)
{
#ifdef _WIN32
	//NONBLOCKING=1
	u_long val = bBlocking?0:1;
	if(INVALID_SOCKET!=s)
		return ioctlsocket(s,FIONBIO,&val);
	return -1;
#else
	int opts;
	opts = fcntl(s,F_GETFL);
	if(-1 == opts)
	{
		perror("fcntl(s,GETFL)");
		return -1;
	}
	if(!bBlocking)
		opts |= O_NONBLOCK;
	else
		opts &= ~O_NONBLOCK;
	if(-1 == fcntl(s,F_SETFL,opts))
	{
		printf("***error s=%d ***\n",s);
		perror("fcntl(s,SETFL,opts); ");
		return -1;
	}
	return 0;
#endif
}

#ifdef _WIN32
int httpc_is_connected(SOCKET s,fd_set *rd,fd_set *wr,fd_set *ex)
{
	WSASetLastError(0);
	if ( !FD_ISSET(s, rd) && !FD_ISSET(s, wr) )
		return 0;
	if (FD_ISSET(s, ex))
		return 0;
	return 1;
}
#else
int httpc_is_connected(SOCKET s,fd_set *rd,fd_set *wr,fd_set *ex)
{
	int err;
	socklen_t len = sizeof( err );

	errno = 0;
	if ( !FD_ISSET( s, rd ) && !FD_ISSET( s, wr ) )
		return 0;
	if ( getsockopt( s, SOL_SOCKET, SO_ERROR, &err, &len ) < 0 )
		return 0;
	errno = err;
	return err == 0;
}
#endif


string Httpc::ip_explain(const char* s)
{
	if(INADDR_NONE != inet_addr(s))
	{
		return s;
	}
	else
	{
		in_addr sin_addr;
		hostent* host = gethostbyname(s);
		if (host == NULL) 
		{
			return s;
		}
		sin_addr.s_addr = *((unsigned long*)host->h_addr);
		return inet_ntoa(sin_addr);
	}
}
typedef struct tagIPFD
{
	string dns;
	string ip;
	int is_set;
	int del;
	tagIPFD(void)
	{
		is_set = 0;
		del = 0;
	}
}IPFD;

#ifdef _WIN32
DWORD WINAPI Httpc::ip_explain_ex_t1(void *p)
#else
void* Httpc::ip_explain_ex_t1(void *p)
#endif
{
	IPFD *ipf = (IPFD*)p;
	ipf->ip = ip_explain(ipf->dns.c_str());
	ipf->is_set = 1;
	while(!ipf->del) 
		Sleep(100);
	delete ipf;

#ifdef _WIN32
	return 0;
#else
	return (void*)0;
#endif
}
string Httpc::ip_explain_ex(const char* s,int maxTick/*=5000*/)
{
	string ip;
	if(INADDR_NONE != inet_addr(s))
	{
		return s;//就是ip
	}
	else
	{
		IPFD *ipf = new IPFD();
		if(!ipf)
			return s;
		ipf->dns = s;
#ifdef _WIN32
		DWORD thid=0;
		HANDLE h = CreateThread(NULL,0,Httpc::ip_explain_ex_t1,(void*)ipf,0,&thid);
		if(INVALID_HANDLE_VALUE==h)
			return s;
		else
			CloseHandle(h);
#else
		pthread_t hthread;
		if(0==pthread_create(&hthread,NULL,Httpc::ip_explain_ex_t1,(void*)ipf))
			pthread_detach(hthread);
		else
			return s;
#endif
		Sleep(10);
		int i=maxTick/100;
		
		while(1)
		{
			if(ipf->is_set)
			{
				ip = ipf->ip;
				ipf->del = 1; 
				return ip;
			}
			if(i<=0)
				break;
			Sleep(100);
			i--;
		}
		ipf->del = 1;
		return s;
	}
}
