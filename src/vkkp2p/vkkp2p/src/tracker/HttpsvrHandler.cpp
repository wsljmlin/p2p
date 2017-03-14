#include "HttpsvrHandler.h"
#include "HttpServer.h"
#include "HttpResponseHeader.h"
#include "HttpContentType.h"
#include "UrlCode.h"
#include "File64.h"
#include "Interface.h"
#include "Util.h"


HttpServer g_httpsvr;
bool g_bopen = false;
bool g_is_exit = false;  //用于linux版本通过http可以控制P2P程序退出

int handle_https_req(HttpRequest_t* req);

void response_file(int fd,const string& path);

void response_cgi(int fd,const string& cgi,const string& head);
void response_tracker_trackerinfo(int fd,const string& cgi,const string& head);
void response_tracker_serverinfo(int fd,const string& cgi,const string& head);
void response_tracker_playlistinfo(int fd,const string& cgi,const string& head);
void response_tracker_allfileinfo(int fd,const string& cgi,const string& head);

//*******************************************************
int httpsvr_start(int port,const char* ip/*=NULL*/)
{
	if(g_bopen)
		return 0;
	if(0==g_httpsvr.open(port,ip,false))
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

	IFM("GET","/tracker/tracker_info.do")
	{
		response_tracker_trackerinfo(fd,cgi,strline);
	}
	EIFM("GET","/tracker/server_info.do")
	{
		response_tracker_serverinfo(fd,cgi,strline);
	}
	EIFM("GET","/tracker/playlist_info.do")
	{
		response_tracker_playlistinfo(fd,cgi,strline);
	}
	EIFM("GET","/tracker/allfile_info.do")
	{
		response_tracker_allfileinfo(fd,cgi,strline);
	}
	else
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
	response_file(fd,path);
}

void response_file(int fd,const string& path)
{
	size64_t size = File64::get_file_size(path.c_str());
	if(size <= 0)
	{
		HttpClient::response_error(fd);
		return;
	}

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

	FILE *fp = fopen(path.c_str(),"rb");
	if(NULL!=fp)
	{
		char buf[4096];
		int n = 0;
		int ret = 0;
		int send_ret = 0;
		while(!feof(fp))
		{
			n = (int)fread(buf,1,4096,fp);
			if(n<=0)
				break;
			send_ret = 0;
			ret = 0;
			while(send_ret != n)
			{
				ret = send(fd,buf+send_ret,n-send_ret,0);
				if(ret<=0)
					break;
				send_ret += ret;
			}
			if(ret<=0)
				break;

		}
		fclose(fp);
	}
	HttpClient::socket_readible(fd,10000);//最多等10秒钟，当对方收完关闭时，会可读。
}
string xml_head = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n"
	"<response>\r\n";
string xml_tail = "</response>\r\n";
void response_tracker_trackerinfo(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:10080/tracker/tracker_info.do
	MsgTrackerInfo inf;
	TIF::instance()->get_msg_tracker_info(inf);
	char buf[2048];
	string xml = xml_head;
	sprintf(buf,"<begin_time>%s</begin_time>\r\n"
		"<tracker_id>%d</tracker_id>\r\n"
		"<tracker_ver>%d</tracker_ver>\r\n"
		"<user_num>%d</user_num>\r\n"
		"<peer_num>%d</peer_num>\r\n"
		"<file_num>%d</file_num>\r\n"
		"<usertype_>client,server,center,super</usertype_>\r\n"
		"<usertype>%d,%d,%d,%d</usertype>\r\n"
		"<nattype_>update timer(125 second) ; nat0~nat6</nattype_>"
		"<nattype>%d,%d,%d,%d,%d,%d,%d</nattype>\r\n"
		,inf.begin_time.c_str(),inf.tracker_id,inf.tracker_ver,inf.user_num,inf.peer_num,inf.file_num
		,inf.ut_client,inf.ut_server,inf.ut_center,inf.ut_super
		,inf.nt[0],inf.nt[1],inf.nt[2],inf.nt[3],inf.nt[4],inf.nt[5],inf.nt[6]);
	xml += buf;
	xml += xml_tail;
	HttpClient::response_message(fd,xml);
}
void response_tracker_serverinfo(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:10080/tracker/server_info.do
	MsgServerInfo inf;
	MsgServerNode *s;
	TIF::instance()->get_msg_server_info(inf);
	char buf[4096];
	char buf2[256],str[64];
	//string xml;
	strcpy(buf, xml_head.c_str());
	sprintf(buf2,"<servers num='%d'>\r\n",inf.svrs.size());
	strcat(buf,buf2);
	int i=0;
	tm *t;
	time_t tt;
	for(list<MsgServerNode*>::iterator it=inf.svrs.begin();it!=inf.svrs.end();++it)
	{
		s = *it;
		tt = s->beginTime;
		t = ::localtime(&tt);
		sprintf(str,"%d.%d.%d %d:%d:%d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		sprintf(buf2,"<svr n='%3d' ip='%s' uid='%s' ver='%d' utype='%d' menu='%d' logintime='%s' nat='%d' files='%d' tcp='%s:%d' udp='%s:%d'/>\r\n",++i,
			Util::ip_htoas(s->tcpRealIP).c_str(),s->uid,s->ver,(int)s->utype,s->menu,str,(int)s->ntype,s->sourceNum,
			Util::ip_htoas(s->tcpRealIP).c_str(),(int)s->tcpRealPort,Util::ip_htoas(s->udpRealIP).c_str(),(int)s->udpRealPort);
		strcat(buf,buf2);

	}

	strcat(buf,"</servers>\r\n");
	strcat(buf,xml_tail.c_str());
	TIF::instance()->free_msg_server_info(inf);
	
	HttpClient::response_message(fd,buf);
}

//获取URL参数，假设url参数为cgi中的最后一个参数
string url_get_urlparam(const string& cgi)
{
	string url="";
	int pos = (int)cgi.find("url=");
	if(pos>0)
	{
		url = cgi.substr(pos+4);
	}
	url = UrlDecode(url);
	return url;
}
void response_tracker_playlistinfo(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:10080/tracker/playlist_info.do?url=
	//url既可以是URL也可以是strhash
	MsgFileInfo inf;
	string url = url_get_urlparam(cgi);
	if(url.empty())
	{
		HttpClient::response_error(fd);
		return;
	}
	if(0==strncmp(url.c_str(),"http://",7)||0==strncmp(url.c_str(),"HTTP://",7))
	{
		inf.hash.set_urldl_string(url.c_str());
	}
	else
	{
		if(0!=inf.hash.set_string_hash(url.c_str()))
		{
			HttpClient::response_error(fd);
			return;
		}
	}
	TIF::instance()->get_msg_file_info(inf);
	char buf[4096],buf2[128];
	string xml = xml_head;
	inf.hash.to_string(buf2,128);
	sprintf(buf,"<result>%d</result>\r\n"
		"<hash>%s</hash>\r\n"
		"<user_num>%d</user_num>\r\n"
		"<server_num>%d</server_num>\r\n"
		"<center_num>%d</center_num>\r\n"
		"<super_num>%d</super_num>\r\n"
		"<server_ip>",
		inf.result,buf2,inf.user_num,inf.server_num,inf.center_num,inf.super_num);
	int i=0;
	for(list<int>::iterator it=inf.svrs.begin();it!=inf.svrs.end();++it)
	{
		sprintf(buf2,"[%s] ",Util::ip_htoas(*it).c_str());
		strcat(buf,buf2);
		if(++i>100)
			break; //最多返回100个
	}
	sprintf(buf2,"</server_ip>\r\n");
	strcat(buf,buf2);
	xml += buf;
	xml += xml_tail;
	HttpClient::response_message(fd,xml);
}
void response_tracker_allfileinfo(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:10080/tracker/allfile_info.do
	//MsgFileInfo inf;
	//先保存到本地，再以文件回复
	FILE *fp = fopen("./allfileinfo.xml","wb+");
	if(!fp)
	{
		HttpClient::response_message(fd,"***save file wrong!!!");
		return;
	}
	list<MsgFileInfo*> ls;
	MsgFileInfo* inf;
	char strhash[64];
	TIF::instance()->get_msg_allfile_info(ls);
	char buf[1024];
	fwrite(xml_head.c_str(),xml_head.length(),1,fp);
	sprintf(buf,"<file_disc>hash - user_num - server_num - super_num - center_num </file_disc>\r\n");
	fwrite(buf,strlen(buf),1,fp);
	for(list<MsgFileInfo*>::iterator it=ls.begin();it!=ls.end();++it)
	{
		inf = *it;
		inf->hash.to_string(strhash,64);
		sprintf(buf,"<file>%s -%d -%d -%d -%d </file>\r\n",
			strhash,inf->user_num,inf->server_num,inf->super_num,inf->center_num);
		fwrite(buf,strlen(buf),1,fp);
	}

	fwrite(xml_tail.c_str(),xml_tail.length(),1,fp);
	fclose(fp);
	TIF::instance()->free_msg_allfile_info(ls);
	response_file(fd,"./allfileinfo.xml");
}

