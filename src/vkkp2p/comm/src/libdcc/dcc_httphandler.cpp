#include "dcc_httphandler.h"
#include "HttpServer.h"
#include "dcc.h"
#include "SerialPortFile.h"

HttpServer g_dcc_httpsvr;
bool g_dcc_httpsvr_bopen = false;

int dcc_httpsvr_handle_req(HttpRequest_t* req);

void dcc_httpsvr_req_call(int fd,HttpRequest_t* req);
void dcc_httpsvr_req_enum_com(int fd,HttpRequest_t* req);

//**********************************************************
int dcc_httpsvr_open(unsigned int port)
{
	if(g_dcc_httpsvr_bopen)
		return 0;
	if(0==g_dcc_httpsvr.open(port,NULL))
	{
		g_dcc_httpsvr_bopen = true;
		g_dcc_httpsvr.SetHandleReqFun(dcc_httpsvr_handle_req);
		return 0;
	}
	return -1;
}
int dcc_httpsvr_close()
{
	if(g_dcc_httpsvr_bopen)
	{
		g_dcc_httpsvr.stop();
		g_dcc_httpsvr_bopen = false;
	}
	return 0;
}

int dcc_httpsvr_handle_req(HttpRequest_t* req)
{
	int fd = req->fd;
#define IF(M,CGI) if(0==strcmp(M,req->method) && strstr(req->cgi,CGI))
#define EIF(M,CGI) else if(0==strcmp(M,req->method) && strstr(req->cgi,CGI))
	IF("GET","/version")
	{
		HttpClient::response_message(fd,"dccver-20141210");
	}
	EIF("GET","/dcc/call.do")
	{
		dcc_httpsvr_req_call(fd,req);
	}
	EIF("GET","/dcc/enum_com.do")
	{
		dcc_httpsvr_req_enum_com(fd,req);
	}
	else
	{
		//http://127.0.0.1:8080/dcc_config.xml
		if(0==strcmp("GET",req->method))
		{
			string path = ".";
			path += req->cgi;
			HttpClient::response_file(fd,path);
			return 0;
		}
		else
		{
			HttpClient::response_error(fd);
			return -1;
		}
	}

	return 0;
}

//*************************************************************************
string g_sxml_header = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n<root>\r\n";
string g_sxml_tail = "</root>";

//<message><![CDATA[ %s ]]></message>
#define _XML_CODE_MSG_	"<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n<root>\r\n<code>%d</code>\r\n<message> %s </message>\r\n</root>"

void dcc_httpsvr_req_call(int fd,HttpRequest_t* req)
{
	string strid = HttpClient::url_get_parameter(req->params,"id");
	string outmsg = "";
	char buf[2048];
	int code = -1;
	if(!strid.empty())
	{
		code = dcc_call(atoi(strid.c_str()),outmsg);
	}
	else
	{
		outmsg = "<ERR_call code=\"1\" msg=\"*** id is empty !\"/>";
	}
	sprintf(buf,_XML_CODE_MSG_,code,outmsg.c_str());
	HttpClient::response_message(fd,buf);
}
void dcc_httpsvr_req_enum_com(int fd,HttpRequest_t* req)
{
	int code = 0;
	SerialDevice_t sd;
	sd.count = 100;
	SerialPortFile::enum_port(&sd);
	char buf[1024];
	strcpy(buf,g_sxml_header.c_str());
	strcat(buf,"<!-- \"code = -1:UnknownError 0:Available;1:NotAvailable;2:InUse;\" -->\r\n");
	for(int i=0;i<sd.count;++i)
	{
		code = SerialPortFile::check_port(sd.paths[i]);
		sprintf(buf+strlen(buf),"<com path=\"%s\" code=\"%d\" />\r\n",sd.paths[i],code);
	}
	strcat(buf,g_sxml_tail.c_str());
	HttpClient::response_message(fd,buf);
}


