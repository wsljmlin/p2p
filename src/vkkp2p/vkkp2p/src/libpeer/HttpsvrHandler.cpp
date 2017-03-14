#include "HttpsvrHandler.h"
#include "HttpServer.h"
#include "HttpResponseHeader.h"
#include "HttpContentType.h"
#include "UrlCode.h"
#include "File64.h"
#include "Util.h"
#include "Interface.h"
#include "PFile.h"
#include "md5.h"
#include "DownManualManager.h"
#include "LocalService.h"
#include "Setting.h"


HttpServer g_httpsvr;
bool g_bopen = false;
bool g_is_exit = false;  //用于linux版本通过http可以控制P2P程序退出

static map<hash_t, int> g_list_playlist; // urecord programe playlist

int handle_https_req(HttpRequest_t* req);

int response_vod_file(int fd,const string& head,const hash_t& hash,const string& url,const string& content_type);
#ifdef SM_VOD
int response_vod_file_ex(int fd,const string& head,const hash_t& hash,int playtype,const string& url,const string& content_type);
void response_playlist_cachetime(int fd,const string& cgi,const string& head);
#endif

void response_vod_stream(int fd,const string& cgi,const string& head);
void response_check_state(int fd,const string& cgi,const string& head);
void response_execute_command(int fd,const string& cgi,const string& head);
void response_cgi(int fd,const string& cgi,const string& head);
void response_file(int fd,const string& path);

void response_crossdomain_xml(int fd,const string& cgi,const string& head);
void response_playlist_open(int fd,const string& cgi,const string& head);
void response_playlist_close(int fd,const string& cgi,const string& head);
void response_playlist_closeall(int fd,const string& cgi,const string& head);
void response_playlist_getlist(int fd,const string& cgi,const string& head);
void response_playlist_play(int fd,const string& cgi,const string& head);
void response_playlist_checkfileready(int fd,const string& cgi,const string& head);
int response_playlist_file(int fd,const string& cgi,const string& head);
void response_playlist_info(int fd,const string& cgi,const string& head);
void response_playlist_allinfo(int fd,const string& cgi,const string& head);

void response_vttcmd_downmanual_add(int fd,const string& cgi,const string& head);
void response_vttcmd_downmanual_setstate(int fd,const string& cgi,const string& head);
void response_vttcmd_downmanual_del(int fd,const string& cgi,const string& head);
void response_vttcmd_downmanual_inf(int fd,const string& cgi,const string& head); //下载中的列表

void response_vttcmd_state(int fd,const string& cgi,const string& head); //返回程序运行的一些状态
void response_vttcmd_downinfo(int fd,const string& cgi,const string& head); //返回一个文件的下载状态信息

void response_cmd_setini(int fd,string& cgi);

#ifndef _WIN32
#include <stdio.h>
#include <unistd.h>
int exec_file(const char *path,const char *cwd,char *const argv[]);
int exec_file(const char* cmd);
int exec_command(const char* cmd);
#endif

#ifdef SM_DBG
#define HTTP_SVR_HANDLER_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "HttpsvrHandler", __LINE__, ##arg)
#else
#define HTTP_SVR_HANDLER_PRT(fmt, arg...) 
#endif


//*******************************************************
int httpsvr_start(int port,const char* ip/*=NULL*/)
{
	if(g_bopen)
		return 0;
	int i = 0;
	while(++i<100)
	{
		if(0==g_httpsvr.open(port,ip))
		{
#ifdef SM_MODIFY
				HTTP_SVR_HANDLER_PRT("port=%d\n", port);
#endif

			g_httpsvr.SetHandleReqFun(handle_https_req);
			g_bopen = true;
#ifdef SM_DBG
			return port;
#else
			return 0;
#endif
		}
		else
		{
			port++;
		}
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
	if ( SettingSngl::instance()->get_localonly() == 0 ) {
		unsigned long addr = req->addr.s_addr;
		DEBUGMSG("check http access : %s \n", Util::ip_htoa(addr) );

		if ( addr != inet_addr("127.0.0.1") ) {
			DEBUGMSG("deny access not from localhost\n");
			return -1;
		}
	}

	int fd = req->fd;
	string strline = req->header;
	string cgi     = req->cgi;
	cgi += req->params;

	HTTP_SVR_HANDLER_PRT("\n\ncgi=%s parms=%s header=%s\n", req->cgi, req->params, req->header);

#define IFM(M,CGI) if(0==strcmp(M,req->method) && strstr(req->cgi,CGI))
#define EIFM(M,CGI) else if(0==strcmp(M,req->method) && strstr(req->cgi,CGI))
#define IF(CGI) if(strstr(req->cgi,CGI))
#define EIF(CGI) else if(strstr(req->cgi,CGI))

	IFM("GET","/vttv/")
	{
		response_vod_stream(fd,cgi,strline);
	}
	EIFM("GET","/version")
	{
#ifdef SM_MODIFY
		/* response vodpeer version */
		HttpClient::response_message(fd, VERSION_STR);
#else
		HttpClient::response_message(fd,"vkkvodpeer-20160422");
#endif
	}
	EIFM("GET","/state/checkstate.js")
	{
		response_check_state(fd,cgi,strline);
	}
	EIFM("GET","/execute_command.js")
	{
		response_execute_command(fd,cgi,strline);
	}
	EIFM("GET","/proccess_close.js")
	{
		HttpClient::response_message(fd,"closing...");
		g_is_exit = true;
	}
	EIFM("GET","/crossdomain.xml")
	{
		response_crossdomain_xml(fd,cgi,strline);
	}
	EIFM("GET","/playlist/open.do")
	{
		response_playlist_open(fd,cgi,strline);
	}
	EIFM("GET","/playlist/close.do")
	{
		response_playlist_close(fd,cgi,strline);
	}
	EIFM("GET","/playlist/closeall.do")
	{
		response_playlist_closeall(fd,cgi,strline);
	}
	EIFM("GET","/playlist/info.do")
	{
		response_playlist_info(fd,cgi,strline);
	}
	EIFM("GET","/playlist/allinfo.do")
	{
		response_playlist_allinfo(fd,cgi,strline);
	}
	EIFM("GET","/playlist/getlist.")
	{
		response_playlist_getlist(fd,cgi,strline);
	}
	EIFM("GET","/playlist/play.")
	{
		response_playlist_play(fd,cgi,strline);
	}
#ifdef SM_VOD
	EIFM("GET","/playlist/cachetime")
	{
		response_playlist_cachetime(fd,cgi,strline);
	}
#endif /* end of SM_VOD */
	EIFM("GET","/playlist/checkfileready.do")
	{
		response_playlist_checkfileready(fd,cgi,strline);
	}
	EIFM("GET","/playlist/file/")
	{
		return response_playlist_file(fd,cgi,strline);
	}
	EIFM("GET","/vttcmd/downmanual_add.do")
	{
		response_vttcmd_downmanual_add(fd,cgi,strline);
	}
	EIFM("GET","/vttcmd/downmanual_setstate.do")
	{
		response_vttcmd_downmanual_setstate(fd,cgi,strline);
	}
	EIFM("GET","/vttcmd/downmanual_del.do")
	{
		response_vttcmd_downmanual_del(fd,cgi,strline);
	}
	EIFM("GET","/vttcmd/downmanual_inf.do")
	{
		response_vttcmd_downmanual_inf(fd,cgi,strline);
	}
	EIFM("GET","/vttcmd/state.do")
	{
		response_vttcmd_state(fd,cgi,strline);
	}
	EIFM("GET","/vttcmd/downinfo.do")
	{
		response_vttcmd_downinfo(fd,cgi,strline);
	}
	EIFM("GET","/cmd/setini.do")
	{
		response_cmd_setini(fd,cgi);
	}
	else
	{
		response_cgi(fd,cgi,strline);
	}
	return 0;
}

//<!--  code 0表示成功， 1表示失败 服务器上没有这个sha1值，也返回0 -->
string g_sxml_rsp_success = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n"
	"<response>\r\n"
	"<code>0</code>\r\n"
	"<message><![CDATA[success]]></message>\r\n"
	"</response>\r\n";

string g_sxml_rsp_failed = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n"
	"<response>\r\n"
	"<code>1</code>\r\n"
	"<message><![CDATA[failed]]></message>\r\n"
	"</response>\r\n";

string g_sxml_rsp_fail_sign = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n"
	"<response>\r\n"
	"<code>1</code>\r\n"
	"<message><![CDATA[fail sign]]></message>\r\n"
	"</response>\r\n";

string xml_head = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n"
	"<response>\r\n";
string xml_tail = "</response>\r\n";

bool md5_check_ok(const string& data,const string& md5_val)
{
	char buf[33];
	memset(buf,0,33);
	if(md5_str_sum(data.c_str(),buf,33))
	{
		if(md5_val==buf)
			return true;
	}
	return false;
}


//*******************************************************

string format_http_header(sint64 ibegin,sint64 iend,sint64 file_size,bool brange,bool bkeeplive,const string& content_type)
{
	char s[256];
	SYSTEMTIME st;
	GetLocalTime(&st);
	HttpResponseHeader responseHdr;

	sint64 iResponeSize = iend - ibegin + 1;

	if (brange)
		responseHdr.AddStatusCode(206);
	else
		responseHdr.AddStatusCode(200);
	//responseHdr.AddLastModified(m_st);
	//responseHdr.AddString("ETag: \"02964d7e54c81:b10\"\r\n");
	//responseHdr.AddMyAllowFields();
	responseHdr.AddString("Accept-Ranges: bytes\r\n");
	responseHdr.AddDate(st);
	responseHdr.AddServer("vtthserver");
	responseHdr.AddContentLength(iResponeSize);
	if(brange)
	{
		sprintf(s,"Content-Range: bytes %lld-%lld/%lld\r\n", ibegin, iend, file_size);
		responseHdr.AddString(s);
	}
	if(bkeeplive)
		responseHdr.AddString("Keep-Alive: timeout=15, max=100\r\nConnection: Keep-Alive\r\n");
	else
		responseHdr.AddString("Connection: close\r\n");
	responseHdr.AddContentType(g_content_type[content_type]);
	//responseHdr.AddContentType("text/plain");
	return responseHdr.m_sHeader+"\r\n";
}
int send_response_data(int fd,PFile* fp,sint64 ibegin,sint64 iend,const string& http_header/*=""*/)
{
	const int BUFLEN = 10*1024;
	char*     buf = new char[BUFLEN];
	int       ret=0,sret=0;
	sint64    n=0;
	DWORD     iLastActionTick = GetTickCount();
	sint64    pos = ibegin;

	string header=http_header;
	while(pos <= iend && !g_https_exiting)
	{
		n = iend-pos+1;
		if(n>BUFLEN) n = BUFLEN;
		fp->seek(pos);
		ret = fp->read(buf,(int)n);
		if(ret == 0)
		{
			Sleep(500);
			//连接60秒获取不到数据即返回失败
			if (iLastActionTick+60000 < GetTickCount())
			{
				break;
			}
			continue;
		}
		if(ret < 0)
			break;

		if(!header.empty())
		{
			char *newbuf = new char[ret + header.length()];
			memcpy(newbuf,header.c_str(),header.length());
			memcpy(newbuf+header.length(),buf,ret);
			sret = send(fd, newbuf, ret + (int)header.length(), 0);
			sret -= (int)header.length();
			header.erase(0);
			delete[] newbuf;
		}
		else
		{
			sret = send(fd, buf, ret, 0);
		}
		if(sret <= 0)
			break;
		iLastActionTick = GetTickCount();
		pos += sret;		
		Sleep(2);
	}
	//DEBUGMSG("# -------> pos = %lld\n",pos);
	delete[] buf;
	if(pos<=iend)
		return -1;
	return 0;
}
int response_vod_file(int fd,const string& head,const hash_t& hash,const string& url,const string& content_type)
{
	//Range: bytes=123-567
	string str;
	bool brange = false;
	sint64 size=0,ibegin=0,iend=0;
	int n = 0;
	if(0==HttpClient::get_header_field(head,"Range",str))
	{
		brange = true;
		str = HttpClient::get_string_index(str,1,"=");
		ibegin = Util::atoll(HttpClient::get_string_index(str,0,"-").c_str());
		iend = Util::atoll(HttpClient::get_string_index(str,1,"-").c_str());
	}

	PFile *fp = NULL;
	fp = PFile::open(hash,NULL,url.c_str());
	if(!fp)
	{
		HttpClient::response_error(fd);
		return -1;
	}
	while(n++<200)
	{
		//最长等待20秒
		size = fp->size();
		if(size>0)
			break;
		Sleep(100);
	}
	if(0==size)
	{
		HttpClient::response_error(fd);
		fp->close();
		return -1;
	}

	if(ibegin<0) ibegin = 0;
	if(ibegin>size) ibegin = size;
	if(iend>=size || iend<=0) iend = size-1;
	//send_response_head(ibegin,iend,size,content_type);
	//send_response_data(fp,ibegin,iend);
	//为了支持暴风影音播放wmv文件，把http头与部分数据一起send。怪问题？？？
	int ret = send_response_data(fd,fp,ibegin,iend,
		format_http_header(ibegin,iend,size,brange,HttpClient::is_keeplive(head),content_type));
	//由于支持keeplive的一些播放会不停重复断开重连，这里需要考虑延迟结束，避免P2P任务频繁开停
	//DEBUGMSG("#------------> rang(%lld,%lld),fp pos = %lld \n",ibegin,iend,fp->tell());
	fp->close();
	return ret;
}

#ifdef SM_VOD
int response_vod_file_ex(int fd,const string& head,const hash_t& hash,int playtype,const string& url,const string& content_type)
{
	//Range: bytes=123-567
	string str;
	bool brange = false;
	sint64 size=0,ibegin=0,iend=0;
	int n = 0;
	if(0==HttpClient::get_header_field(head,"Range",str)) {
		brange = true;
		str = HttpClient::get_string_index(str,1,"=");
		ibegin = Util::atoll(HttpClient::get_string_index(str,0,"-").c_str());
		iend = Util::atoll(HttpClient::get_string_index(str,1,"-").c_str());
	}
	
	//get http url and path
	string fileurl=NULL;
	string filepath=NULL;
	PIF::instance()->downloadlist_getfileurl(hash, fileurl);
	assert(!(fileurl==""));
	PIF::instance()->downloadlist_getfilepath(hash, filepath);
	HTTP_SVR_HANDLER_PRT("furl=%s file=%s\n", fileurl.c_str(), filepath.c_str());
	if(filepath=="") {
		hash_t dlhash;
		hash.url2hash_to_urldlhash(dlhash);
		char bufdl[45];
		dlhash.to_string(bufdl,45); /*convert urldl to string */
		string bufstr = bufdl;
		bufstr += "_";
		filepath = SettingSngl::instance()->get_cache_path() + bufstr + Util::url_get_name(fileurl);
	}
	HTTP_SVR_HANDLER_PRT("need file name:%s\n", filepath.c_str());
	
	PFile *fp = NULL;
	//fp = PFile::open(playtype,hash,NULL,url.c_str());
	fp = PFile::open(playtype,hash,filepath.c_str(),fileurl.c_str());
	if(!fp) {
		HttpClient::response_error(fd);
		return -1;
	}
	while(n++<200)
	{
		//最长等待20秒
		size = fp->size();
		if(size>0)
			break;
		Sleep(100);
	}
	if(0==size)
	{
		HttpClient::response_error(fd);
		fp->close();
		return -1;
	}

	if(ibegin<0) ibegin = 0;
	if(ibegin>size) ibegin = size;
	if(iend>=size || iend<=0) iend = size-1;
	
	//为了支持暴风影音播放wmv文件，把http头与部分数据一起send。怪问题？？？
	int ret = send_response_data(fd,fp,ibegin,iend,
		format_http_header(ibegin,iend,size,brange,HttpClient::is_keeplive(head),content_type));
	//由于支持keeplive的一些播放会不停重复断开重连，这里需要考虑延迟结束，避免P2P任务频繁开停
	//DEBUGMSG("#------------> rang(%lld,%lld),fp pos = %lld \n",ibegin,iend,fp->tell());
	fp->close();
	return ret;
}

#endif /* end of SM_VOD */
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
void response_vod_stream(int fd,const string& cgi,const string& head)
{
	//如果没有sha1串，就直接使用url
	//http://127.0.0.1/vttv/575827e6f4cfc7421a259b2f2198227fac44cf08.rmvb?url=...
	//http://127.0.0.1/vttv/.rmvb?url=...
	printf("#: req file: cgi=%s \n",cgi.c_str());
	string str;
	str = HttpClient::url_get_name(cgi);
	string tth = HttpClient::get_string_index(str,0,".");
	string content_type = ".";
	content_type += HttpClient::get_string_index(str,1,".");
	//允许url带"&参数"
	//string url = HttpClient::url_get_parameter(cgi,"url");
	string url = url_get_urlparam(cgi);

	hash_t hash;
	if(0!=hash.set_string_hash(tth.c_str()))
	{
		if(url.empty())
			HttpClient::response_error(fd);
		hash.set_url_string(url.c_str());
	}
	response_vod_file(fd,head,hash,url,content_type);
}
string g_crossdomain_xml = "<?xml version=\"1.0\"?>"
	"<!DOCTYPE cross-domain-policy SYSTEM \" http://www.adobe.com/xml/dtds/cross-domain-policy.dtd\">"
	"<cross-domain-policy>"
    "<site-control permitted-cross-domain-policies=\"all\"/>"
    "<allow-access-from domain=\"*\"/>"
    "<allow-http-request-headers-from domain=\"*\" headers=\"*\"/>" 
	"</cross-domain-policy> ";

void response_crossdomain_xml(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/crossdomain.xml
	HttpClient::response_message(fd,g_crossdomain_xml.c_str());
}

void response_playlist_open(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/open.do?url=playlist_url&closeother=
	//closeother=1关闭其它
#ifdef SM_MODIFY
	string url = HttpClient::url_get_parameter(cgi, "url", "closeother");
#else
	string url = url_get_urlparam(cgi);
#endif 
	string str = HttpClient::url_get_parameter(cgi,"closeother");
	string name = HttpClient::url_get_parameter(cgi,"name");
#ifdef SM_VOD
	string playtype = HttpClient::url_get_parameter(cgi,"type");//UrlDecode( HttpClient::url_get_parameter(cgi,"type");
	HTTP_SVR_HANDLER_PRT("get playtype=%s\n", playtype.c_str());
#endif

	bool closeother = false;
	if(str=="1")
		closeother = true;

#ifdef SM_VOD
	closeother = (playtype=="1") ? false:true;
	bool autoclose = (playtype=="1") ? false:true;
	int ret = PIF::instance()->downloadlist_open(url,name,closeother, (playtype=="1") ? PLAYTYPE_VOD:PLAYTYPE_LIVE, autoclose);
	/* record current programe */
	if(0!=ret) {
		HttpClient::response_error(fd);
		return;
	}
	hash_t hashdl;
	hashdl.set_urldl_string(url.c_str());
	if(g_list_playlist.find(hashdl)==g_list_playlist.end()) 
		g_list_playlist[hashdl] = (playtype=="1") ? PLAYTYPE_VOD:PLAYTYPE_LIVE; 
#else
	int ret = PIF::instance()->downloadlist_open(url,name,closeother);
#endif /* end of SM_VOD */
		
	char buf[256];
	sprintf(buf,"result=%d",ret); //0表示成功，2表示繁忙
	HttpClient::response_message(fd,buf);
}
void response_playlist_close(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/close.do?url=playlist_url
	string url = url_get_urlparam(cgi);
	int ret = PIF::instance()->downloadlist_close(url);
	char buf[256];
	sprintf(buf,"result=%d",ret);
	HttpClient::response_message(fd,buf);
}
void response_playlist_closeall(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/closeall.do
	int ret = PIF::instance()->downloadlist_closeall();
	char buf[256];
	sprintf(buf,"result=%d",ret);
	HttpClient::response_message(fd,buf);
}

void response_playlist_info(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/info.do?url=playlist_url
	string url = url_get_urlparam(cgi);
	MsgDownloadlistInfo_t inf;
	int ret = PIF::instance()->downloadlist_getlist_info(url,inf);
	if(0!=ret)
	{
		HttpClient::response_message(fd,g_sxml_rsp_failed);
		return;
	}
	
	char *buf = new char[4096];
	char buf2[128];
	char shash[64];
	inf.hash.to_string(shash,64);
	time_t ti = (time_t)inf.begintime;
	tm *t = localtime(&ti);
	sprintf(buf2,"%04d.%02d.%02d %02d:%02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	sprintf(buf,"%s<code>0</code>\r\n"
		"<hash>%s</hash>\r\n"
		"<begintime>%s</begintime>\r\n"
		"<seconds>%d</seconds>\r\n"
		"<downing_num>%d</downing_num>\r\n"
		"<httpc_p2pc_p2psrc_num>%d--%d / %d</httpc_p2pc_p2psrc_num>\r\n"
		"<downsizeB>%lld</downsizeB>\r\n"
		"<sharesizeB>%lld</sharesizeB>\r\n"
		"<downspeedKB>%d</downspeedKB>\r\n"
		"<sharespeedKB>%d</sharespeedKB>\r\n"
		"<file_allnum>%d</file_allnum>\r\n"
		"<file_fininum>%d</file_fininum>\r\n"
		"<fini_i num='%d'>"
		,xml_head.c_str(),shash,buf2,inf.seconds,inf.downing_num
		,inf.httpconn_num,inf.p2pconn_num,inf.p2psrc_num
		,inf.downSizeB,inf.shareSizeB
		,inf.downSpeed_KB,inf.shareSpeed_KB,inf.file_all_num,inf.file_fini_num,inf.fini_i_ls.size());
	for(list<int>::iterator it=inf.fini_i_ls.begin(); it!=inf.fini_i_ls.end();++it)
	{
		sprintf(buf2,"%d,",*it);
		strcat(buf,buf2);
	}
	strcat(buf,"</fini_i>\r\n");

        strcat(buf,"<downsizeBbyType>");
        for(int i=0;i<3;i++) {
                sprintf(buf2,"%lld,", inf.downSizeB_Con[i] );
                strcat(buf,buf2);
        }
        strcat(buf,"</downsizeBbyType>\r\n");
        strcat(buf,"<downsizeBbyUser>");
        for(int i=0;i<6;i++) {
                sprintf(buf2,"%lld,", inf.downSizeB_User[i] );
                strcat(buf,buf2);
        }
        strcat(buf,"</downsizeBbyUser>\r\n");

	strcat(buf,xml_tail.c_str());

	HttpClient::response_message(fd,buf);
	delete[] buf;
}

void response_playlist_allinfo(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/allinfo.do
	list<MsgDownloadlistInfo2_t> ls;
	int ret = PIF::instance()->downloadlist_get_all_dlinfo(ls);
	if(0!=ret)
	{
		HttpClient::response_message(fd,g_sxml_rsp_failed);
		return;
	}

	char *xmlbuf = new char[10240];
	char buf[1024];
	strcpy(xmlbuf,xml_head.c_str());
	sprintf(buf,"<playlist num='%d'>\r\n",ls.size());
	strcat(xmlbuf,buf);
	time_t curr_time = time(NULL);
	for(list<MsgDownloadlistInfo2_t>::iterator it=ls.begin();it!=ls.end();++it)
	{
		MsgDownloadlistInfo2_t& inf = *it;
		sprintf(buf,"<pl url='%s' name='%s' hash='%s' RS='%d' SS='%d' UTIME='%s -- %d'> %d/%d - %d </pl>\r\n",
			inf.url.c_str(),inf.name.c_str(),inf.strhash.c_str(),
			inf.downSpeed_KB,inf.shareSpeed_KB,
			Util::time_to_datetime_string(inf.last_update_pl_time).c_str(),(int)((curr_time-inf.last_update_pl_time)/60),
			inf.token_i,inf.max_i,inf.file_fini_num); 
		strcat(xmlbuf,buf);
	}
	strcat(xmlbuf,"</playlist>\r\n");
	strcat(xmlbuf,xml_tail.c_str());
	HttpClient::response_message(fd,xmlbuf);
	delete[] xmlbuf;
}
void response_playlist_getlist(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/getlist.do?url=playlist_url
	//也可以是http://127.0.0.1:7080/playlist/getlist.m3u8?url=playlist_url
	string url = url_get_urlparam(cgi);
	list<string> ls;
	PIF::instance()->downloadlist_getlist(url,ls);
	DEBUGMSG("#playlist_getlist: list size=%d \n",ls.size());
	string str = "",tmp;
	string host;
	HttpClient::get_header_field(head,"Host",host);
	size_t pos = host.rfind(':');
	if(pos!=string::npos)
		host.erase(pos);
	if(host == "127.0.0.1") host = "";
	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
	{
		tmp = *it;
		if(!host.empty())
			Util::str_replace(tmp,"127.0.0.1",host);
		str += tmp;
		str += "\r\n";
	}
	//类型
	//HttpClient::response_message(fd,str);
	HttpResponseHeader responseHdr;
    SYSTEMTIME st;
	GetLocalTime(&st);
	responseHdr.AddStatusCode(200);
    responseHdr.AddDate(st);
    responseHdr.AddServer("vkkserver");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength((int)str.length());
    responseHdr.AddContentType("application/vnd.apple.mpegurl");

    responseHdr.Send(fd);
	send(fd,str.c_str(),(int)str.length(),0);
}
void response_playlist_play(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/play.m3u8?url=playlist_url
	string url = url_get_urlparam(cgi);

// new url format
// http://127.0.0.1:7080/playlist/play.m3u8?m3u8end=%3F&tssegtailer=&url=playlist_url
// tssegtailer parameter is not ok now.
// TODO fix tssegtailer parameters

        string m3u8tailer = UrlDecode( HttpClient::url_get_parameter(cgi,"m3u8end") );
        SettingSngl::instance()->set_m3u8_tailer( m3u8tailer );
        string tssegtailer   = UrlDecode( HttpClient::url_get_parameter(cgi,"tsend") );
        SettingSngl::instance()->set_tsseg_tailer( tssegtailer );
#ifdef SM_VOD
	string playtype =UrlDecode( HttpClient::url_get_parameter(cgi,"type"));  /*HttpClient::url_get_parameter(cgi,"type");*/
	HTTP_SVR_HANDLER_PRT("get playtype=%s\n", playtype.c_str());
#endif
	HTTP_SVR_HANDLER_PRT("m3u8end=%s tsend=%s url=%s\n", m3u8tailer.c_str(), tssegtailer.c_str(), url.c_str());

        string eurl   = UrlDecode( HttpClient::url_get_parameter(cgi,"eurl") );
        if(eurl.length()>8)
                url = eurl;

        DEBUGMSG("playlist_play , get m3u8 endstr : %s : ts endstr : %s \n", m3u8tailer.c_str(), tssegtailer.c_str() );

	size_t p1 = url.find( m3u8tailer );
	if ( p1 != 0 && p1 != string::npos) {
		string clean_url = url.substr(0, p1 );
		DEBUGMSG("#*** playlist play knowned plurl (%s) \n", clean_url.c_str() );
		SettingSngl::instance()->m_plurls[ clean_url ] = url;
		url = clean_url;
	}

	static string last_url = "";
	static int rcount = 0;

	if ( last_url != url )  {
		last_url = url;
		rcount = 0;
		DEBUGMSG("#*** playlist play start to play (%s) \n", url.c_str() );
	} else {
		rcount++;
	}

// end new url fixed

	bool closeother = true;
	string name = HttpClient::url_get_parameter(cgi,"name");
#ifdef SM_VOD
	//closeother = (playtype=="1") ? false:true;
	//bool autoclose = (playtype=="1") ? false:true;
	int ret = PIF::instance()->downloadlist_open(url,name,closeother, (playtype=="1") ? PLAYTYPE_VOD:PLAYTYPE_LIVE, true);
#else
	int ret = PIF::instance()->downloadlist_open(url,name,closeother,true);
#endif
	if(0!=ret)
	{
		HttpClient::response_error(fd);
		return;
	}

	list<string> ls;
	int n = 0;
	PIF::instance()->downloadlist_getlist(url,ls);
	while(ls.empty() && ++n<50)
	{
		Sleep(200);
		PIF::instance()->downloadlist_getlist(url,ls);
	}
	DEBUGMSG("#playlist_play: list size=%d \n",ls.size());
	string str = "",tmp;
	string host;
	HttpClient::get_header_field(head,"Host",host);
	size_t pos = host.rfind(':');
	if(pos!=string::npos)
		host.erase(pos);
	if(host == "127.0.0.1") host = "";


	//int max_listsize = SettingSngl::instance()->get_playlist_size();
	int count = 0;
	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
	{
		tmp = *it;
		if(!host.empty())
			Util::str_replace(tmp,"127.0.0.1",host);
		str += tmp;
		str += "\r\n";
		if ( tmp.at(0) != '#' ) {
			count ++;
		}
		
	//	if ( count >= max_listsize ) {
	//		DEBUGMSG("#playlist_play: list size %d, max %d\n", count, max_listsize );
	//		break;
	//	}
		if ( count >= rcount+2 ) {
			// when player first request, return 1 items, it seems the stream just start 
			DEBUGMSG("#playlist_play: list size %d, is bigger when the %d req\n", count, rcount );
			//break;
		}
	}

	/* record current programe */
#ifdef SM_VOD
	hash_t hashdl;
	hashdl.set_urldl_string(url.c_str());
	if(g_list_playlist.find(hashdl)==g_list_playlist.end()) 
		g_list_playlist[hashdl] = (playtype=="1") ? PLAYTYPE_VOD:PLAYTYPE_LIVE; 
#endif /* end of SM_VOD */

	//类型
	//HttpClient::response_message(fd,str);
	HttpResponseHeader responseHdr;
    SYSTEMTIME st;
	GetLocalTime(&st);
	responseHdr.AddStatusCode(200);
    responseHdr.AddDate(st);
    responseHdr.AddServer("vkkserver");
    responseHdr.AddMyAllowFields();
	responseHdr.AddContentLength((int)str.length());
    responseHdr.AddContentType("application/vnd.apple.mpegurl");
	HTTP_SVR_HANDLER_PRT("#######send######### :\n%s\n", str.c_str());

    responseHdr.Send(fd);
	send(fd,str.c_str(),(int)str.length(),0);
}

#ifdef SM_VOD
void response_playlist_cachetime(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/cachetime?type=&url=playlist_url
	string url = url_get_urlparam(cgi);

	// new url format
	// http://127.0.0.1:7080/playlist/cachetime?m3u8end=%3F&tssegtailer=%3F&type=&url=playlist_url

	/* check whether the requested playlist is in now */
	/*
	string m3u8tailer = UrlDecode( HttpClient::url_get_parameter(cgi,"m3u8end") );
	SettingSngl::instance()->set_m3u8_tailer( m3u8tailer );
	string tssegtailer	 = UrlDecode( HttpClient::url_get_parameter(cgi,"tsend") );
	SettingSngl::instance()->set_tsseg_tailer( tssegtailer );
	string clean_url;
	size_t p1 = url.find( m3u8tailer );
	if ( p1 != 0 && p1 != string::npos) {
		clean_url = url.substr(0, p1 );
	} */
	HTTP_SVR_HANDLER_PRT("get progame=%s\n", url.c_str());
	map<string, string>::iterator it = SettingSngl::instance()->m_plurls.find(url);
	if(it==SettingSngl::instance()->m_plurls.end()) {
		HttpClient::response_message(fd,"no such programe!"); 
		return ;
	}

	string playtype = UrlDecode( HttpClient::url_get_parameter(cgi,"type"));/*HttpClient::url_get_parameter(cgi,"type");*/
	

	/* get cache time, live will return time 0 */
	int time = 0;
	char time_str[32];
	if(playtype=="1") {
		int ret = PIF::instance()->downloadlist_getcachetime(url, time);
		if(0!=ret) {
			HttpClient::response_error(fd);
			return;
		}
	} else {
		time = 0;
	}

	sprintf(time_str, "cachetime=%d", time);
	HttpClient::response_message(fd,time_str);
}

#endif /* end of SM_VOD */
void response_playlist_checkfileready(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/checkfileready.do?url=file_url
	//注意把url参数放到最后
	string url = url_get_urlparam(cgi);;
	string strhash = HttpClient::get_string_index(url,5,"/");
	if(strhash.length()!=42)
	{
		HttpClient::response_error(fd);
		return;
	}
	hash_t hash;
	hash.set_url2_string_hash(strhash.c_str()+2);
	int ret = PIF::instance()->check_fileready(hash);
	char buf[256];
	sprintf(buf,"result=%d",ret);
	HttpClient::response_message(fd,buf);
}
int response_playlist_file(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/playlist/file/strhash/name
	string str = HttpClient::url_get_name(cgi);
	string content_type = ".";
	content_type += HttpClient::get_string_index(str,1,".");
	string name = HttpClient::get_string_index(cgi,3,"/");
	if(name.length()!=42)
	{
		HttpClient::response_error(fd);
		return -1;
	}
#ifdef SM_VOD
	HTTP_SVR_HANDLER_PRT("need:%s\n", name.c_str());
	DEBUGMSG("#==>playlist_file:%s \n",cgi.c_str());
	int ret=-1;
	hash_t hashurl2;
	hash_t hashdl;
	hashurl2.set_url2_string_hash(name.c_str()+2);
	hashdl.set_urldl_string_hash(name.c_str()+2);
	
	assert(g_list_playlist.end()!=g_list_playlist.find(hashdl));
	if(g_list_playlist.end()!=g_list_playlist.find(hashdl)){
		HTTP_SVR_HANDLER_PRT("the program have created............!!\n" );
		/* the programe  have created, this branch should always enter */
		if(0!=PIF::instance()->downloadlsit_need_download(name) ){
			PIF::instance()->downloadlsit_need_download_end(name);
			HttpClient::response_error(fd);
			return -1;
		}
		ret = response_vod_file_ex(fd,head,hashurl2,g_list_playlist[hashdl], "",content_type);
		if(-1==ret) {
			//检查是否是队尾，是队尾即删除掉它
			DEBUGMSG("#*** response_playlist_file() error **** \n");
		}

	} 
#else
	if(0!=PIF::instance()->downloadlsit_need_download(name))
	{
		PIF::instance()->downloadlsit_need_download_end(name);
		HttpClient::response_error(fd);
		return -1;
	}
	HTTP_SVR_HANDLER_PRT("need:%s\n", name.c_str());
	DEBUGMSG("#==>playlist_file:%s \n",cgi.c_str());
	hash_t hash;
	hash.set_url2_string_hash(name.c_str()+2);
	int ret = response_vod_file(fd,head,hash,"",content_type);
	if(-1==ret)
	{
		//检查是否是队尾，是队尾即删除掉它
		DEBUGMSG("#*** response_playlist_file() error **** \n");
	}
#endif /* end of SM_VOD */
	PIF::instance()->downloadlsit_need_download_end(name);
	return ret;
}
void response_vttcmd_downmanual_add(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/vttcmd/downmanual_add.do?sha1=&filename=&autocache=0&url=
	//autocache:0-download; 1-autocache会被自动管理删除
	//0对应FTYPE_DOWNLOAD，1对应FTYPE_VOD
	string name,tth,url,str;
	int state,ftype;

	tth = HttpClient::url_get_parameter(cgi,"sha1");
	name = HttpClient::url_get_parameter(cgi,"filename");
	str = HttpClient::url_get_parameter(cgi,"autocache");

	//注意把url参数放到最后
	url = url_get_urlparam(cgi);

	if(str=="1")
		ftype = FTYPE_VOD;
	else
		ftype = FTYPE_DOWNLOAD;
	state = DS_QUEUE;
	HTTP_SVR_HANDLER_PRT("url=%s tth=%s name=%s\n", url.c_str(), tth.c_str(), name.c_str());
	if( tth.empty()  && url.empty())
	{
		HttpClient::response_message(fd,g_sxml_rsp_success);
		return ;
	}
	
	if(tth.empty())
	{
		hash_t hash;
		hash.set_url_string(url.c_str());
		char buf[64];
		hash.to_string(buf,64);
		tth = buf;
	}
	hash_t hash;
	if(0==hash.set_string_hash(tth.c_str()))
	{
		if(-1!=DownManualManagerSngl::instance()->add_down(hash,name,"",url,state,ftype))
		{
			HTTP_SVR_HANDLER_PRT("XXXXXXXXXXXXXXXXXXXX\n");

			HttpClient::response_message(fd,g_sxml_rsp_success);
			return;
		}
	}
	HttpClient::response_message(fd,g_sxml_rsp_failed);
}
void response_vttcmd_downmanual_setstate(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/vttcmd/downmanual_setstate.do?sha1=&state=&url=
	//state:0-stop,1-start,2-queue
	string tth,str,url;
	int state;
	tth = HttpClient::url_get_parameter(cgi,"sha1");
	str = HttpClient::url_get_parameter(cgi,"state");
	//注意把url参数放到最后
	url = url_get_urlparam(cgi);
	if(str=="0")
		state = DS_STOP;
	else if(str=="1")
		state = DS_START;
	else if(str=="2")
		state = DS_QUEUE;
	else
	{
		HttpClient::response_message(fd,g_sxml_rsp_failed);
		return;
	}
	if(tth.empty() && url.empty())
	{
		HttpClient::response_message(fd,g_sxml_rsp_success);
		return ;
	}
	if(tth.empty())
	{
		hash_t hash;
		hash.set_url_string(url.c_str());
		char buf[64];
		hash.to_string(buf,64);
		tth = buf;
	}
	hash_t hash;
	if(0==hash.set_string_hash(tth.c_str()))
	{
		if(-1!=DownManualManagerSngl::instance()->set_state(hash,state))
		{
			HttpClient::response_message(fd,g_sxml_rsp_success);
			return;
		}
	}
	HttpClient::response_message(fd,g_sxml_rsp_failed);

}
void response_vttcmd_downmanual_del(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/vttcmd/downmanual_del.do?sha1=&systime=&sign=&url=
	//sha1不存在也当成功
	string tth = HttpClient::url_get_parameter(cgi,"sha1");
	string sign = HttpClient::url_get_parameter(cgi,"sign");
	string systime = HttpClient::url_get_parameter(cgi,"systime");
	//注意把url参数放到最后
	string url = url_get_urlparam(cgi);

	//string des_data = tth + url +systime+"DeL";
	//if(!md5_check_ok(des_data,sign))
	//{
	//	HttpClient::response_message(fd,g_sxml_rsp_fail_sign);
	//	return;
	//}
	if(tth.empty() && url.empty())
	{
		HttpClient::response_message(fd,g_sxml_rsp_success);
		return ;
	}
	if(tth.empty())
	{
		hash_t hash;
		hash.set_url_string(url.c_str());
		char buf[64];
		hash.to_string(buf,64);
		tth = buf;
	}
	hash_t hash;
	if(0==hash.set_string_hash(tth.c_str()))
	{
		DownManualManagerSngl::instance()->del_down(hash,true);
		HttpClient::response_message(fd,g_sxml_rsp_success);
		return;
	}
	HttpClient::response_message(fd,g_sxml_rsp_failed);
}
void response_vttcmd_downmanual_inf(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/vttcmd/downmanual_inf.do
	string strxml = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n"
	"<response>\r\n"
	"<code>0</code>\r\n"
	"<message><![CDATA[success]]></message>\r\n"
	"<fileinfo>hash|url|size|ftype|state(0:1:2)|progress(1/1000)|speed(B)|createtime</fileinfo>\r\n";

	char buf[1024];
	DownManualManager::FileList ls;
	DownManualManager::FileIter it;
	DownManualInfo* inf;

	//增加共享速度节点
	sprintf(buf,"<sharespeed_kb>%d</sharespeed_kb>\r\n",(PIF::instance()->get_sharespeed()>>10));
	strxml += buf;
	DownManualManagerSngl::instance()->down_file_get_info(0,DownManualManagerSngl::instance()->down_file_get_count(),ls);
	sprintf(buf,"<num>%d</num>\r\n<files>\r\n",ls.size());
	strxml += buf;
	for(it=ls.begin();it!=ls.end();++it)
	{
		inf = *it;
		sprintf(buf,"<file>%s|%s|%lld|%d|%d|%d|%d|%d</file>\r\n",inf->tth.c_str()
			,inf->url.c_str()
			,inf->size
			,inf->ftype
			,inf->state
			,inf->progress
			,inf->speed
			,inf->createtime);
		strxml += buf;
		delete inf;
	}
	strxml += "</files>\r\n</response>\r\n";
	HttpClient::response_message(fd,strxml);
}
void response_vttcmd_state(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/vttcmd/state.do
	string strxml = "<?xml version=\"1.0\" encoding=\"gbk\" ?>\r\n"
	"<response>\r\n"
	"<code>0</code>\r\n";


	string last_login_time;
	char buf[1024];
	MyState_t st;
	list<string> ls1,ls2;
	PIF::instance()->get_mystate(st);
	PIF::instance()->get_all_downloadlists(ls1);
	LocalServiceSngl::instance()->get_pll(ls2);

	sprintf(buf,"<ver>%d</ver>\r\n",st.ver);
	strxml += buf;
	sprintf(buf,"<user_type>%d - [%s] </user_type>\r\n",st.user_type,g_str_usertype[st.user_type]);
	strxml += buf;
	sprintf(buf,"<minor_ver>%d</minor_ver>\r\n",(int)st.is_minor_version);
	strxml += buf;
	sprintf(buf,"<login>%s</login>\r\n",st.islogin?"true":"false");
	strxml += buf;
	sprintf(buf,"<login_times>%d</login_times>\r\n",st.login_times);
	strxml += buf;
	sprintf(buf,"<tracker>%s:%d</tracker>\r\n",st.tracker_ip.c_str(),st.tracker_port);
	strxml += buf;
	sprintf(buf,"<accept>%s:%d:%d</accept>\r\n",st.real_ip.c_str(),st.tcp_local_port,st.udp_local_port);
	strxml += buf;
	sprintf(buf,"<nat_type>%d</nat_type>\r\n",st.nat_type);
	strxml += buf;
	sprintf(buf,"<begin_time>%s</begin_time>\r\n",st.begin_time.c_str());
	strxml += buf;
	sprintf(buf,"<last_login_time>%s</last_login_time>\r\n",st.last_login_time.c_str());
	strxml += buf;
	sprintf(buf,"<new_peer_times>%d</new_peer_times>\r\n",st.new_peer_times);
	strxml += buf;
	sprintf(buf,"<online_peers>%d</online_peers>\r\n",st.online_peers);
	strxml += buf;

	list<string>::iterator it;
	strxml += "<downloadlist>\r\n";
	for(it=ls1.begin();it!=ls1.end();++it)
	{
		sprintf(buf,"<url>%s</url>\r\n",(*it).c_str());
		strxml += buf;
	}
	strxml += "</downloadlist>\r\n";

	strxml += "<share_downloadlist>\r\n";
	for(it=ls2.begin();it!=ls2.end();++it)
	{
		sprintf(buf,"<url>%s</url>\r\n",(*it).c_str());
		strxml += buf;
	}
	strxml += "</share_downloadlist>\r\n";

	strxml += "</response>\r\n";
	HttpClient::response_message(fd,strxml);
}
void response_vttcmd_downinfo(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/vttcmd/downinfo.do?url=[url/strhash]
	string url = url_get_urlparam(cgi);
	if(url.empty())
		HttpClient::response_error(fd);
	hash_t hash;
	if(0!=hash.set_string_hash(url.c_str()))
	{
		hash.set_url_string(url.c_str());
	}

	DownloadInfo inf;
	int ret = PIF::instance()->get_downloadinfo(hash,inf);
	if(0!=ret)
	{
		HttpClient::response_message(fd,g_sxml_rsp_failed);
		return;
	}
	
	char *buf = new char[4096];
	char buf2[128];
	char shash[64];
	inf.hash.to_string(shash,64);
	time_t ti = (time_t)inf.begintime;
	tm *t = localtime(&ti);
	sprintf(buf2,"%04d.%02d.%02d %02d:%02d:%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	sprintf(buf,"%s<code>0</code>\r\n"
		"<hash>%s</hash>\r\n"
		"<ftype>%d</ftype>\r\n"
		"<begintime>%s</begintime>\r\n"
		"<seconds>%d</seconds>\r\n"
		"<sizeB>%lld</sizeB>\r\n"
		"<blocks>%d</blocks>\r\n"
		"<downblocks>%d</downblocks>\r\n"
		"<downspeedKB>%d</downspeedKB>\r\n"
		"<src_num>%d</src_num>\r\n"
		"<conn_num>%d</conn_num>\r\n"\
		,xml_head.c_str(),shash,inf.ftype,buf2,inf.seconds,inf.size,inf.blocks,inf.down_blocks
		,(int)((inf.speedB+512)>>10),inf.connNum,inf.connNum);

	strcat(buf,xml_tail.c_str());

	HttpClient::response_message(fd,buf);
	delete[] buf;
}
void response_check_state(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1:7080/state/checkstate.js
	string str;
	if(PIF::instance()->check_state()!=0)
	{
		str = "false";
	}
	else
	{
		str = "ok";
	}
	HttpClient::response_message(fd,str);
}
void response_cmd_setini(int fd,string& cgi)
{
	//http://127.0.0.1:7080/cmd/setini.do?filepath=&appname=&keyname=&keyvalue=
	//test:
	//http://127.0.0.1:7080/cmd/setini.do?filepath=./peerconfig.ini&appname=limit&keyname=downloadhttpi_cnns&keyvalue=0
	string filepath = HttpClient::url_get_parameter(cgi,"filepath");
	string appname = HttpClient::url_get_parameter(cgi,"appname");
	string keyname = HttpClient::url_get_parameter(cgi,"keyname");
	string keyvalue = HttpClient::url_get_parameter(cgi,"keyvalue");
	WritePrivateProfileStringA(appname.c_str(),keyname.c_str(),keyvalue.c_str(),filepath.c_str());
	HttpClient::response_message(fd,g_sxml_rsp_success);
}
void response_execute_command(int fd,const string& cgi,const string& head)
{
	//http://127.0.0.1/execute_command.js?command=
	string strret = "false!";
	///////////
#if !defined(_WIN32)
	string cmd = "";
	cmd = HttpClient::url_get_parameter(cgi,"command");
	if(0==exec_command(cmd.c_str()))
		strret = "true";
	//printf("$: --->: execute cmd : %s \n",cmd.c_str());
	//system(cmd.c_str());
	//strret = "true";
#endif
	///////////
	HttpClient::response_message(fd,strret);
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


//*******************************************************
#ifndef _WIN32
int exec_file(const char *path,const char *cwd,char *const argv[])
{
	int pid;
	pid=fork();
	switch(pid)
	{
	case -1:
		{
			perror("fork faild");
			return -1;
		}
		break;
	case 0:
		{
			//再次创建子进程,让中间进程结束,第三层进程(应用进程)的父进程将被init接管,此时的进程即使远程操作断开,也不会被kill掉了
			pid = fork();
			if(pid < 0 || pid>0)
				exit(0); 

			//由于改变进程的工作目录,如果原来的path是相对目录,先变成绝对目录
			char old_cwd[1024];
			char buf[1024];
			getcwd(old_cwd,1024);
			if(path[0] != '/')
			{
				if(old_cwd[strlen(old_cwd)-1] != '/')
					sprintf(buf,"%s/%s",old_cwd,path);
				else
					sprintf(buf,"%s%s",old_cwd,path);
			}
			else
				sprintf(buf,"%s",path);

			if(cwd&&strlen(cwd))
				chdir(cwd);

			char log[2048];
			sprintf(log,"open proccess [path=%s,execv_path=%s,cwd=%s]",path,buf,cwd?cwd:"");
			printf("$: %s\n",log);
			execv(buf,argv);
			
			printf("***start [path=%s] failed!",path);
			exit(0);
		}
		break;
	default:
		{
		}
		break;
	}
	waitpid(pid,NULL,0);
	printf("$: open proccess = %s finished \n",path);
	return 0;
}

int exec_file(const char* cmd)
{
	if(NULL==cmd)
		return -1;
	printf("$: ----->: execute cmd : %s \n",cmd);
	///////////////
	char **exec_argv;
	char name[128];
	char *cmd_tmp = new char[strlen(cmd)+2];
	char *ptr=NULL,*ptr2=NULL;
	int n = 1,i=0;

	//将命令行拷到临时串,前面的空格忽略
	ptr = (char*)cmd;
	while(*ptr == ' ') ptr++;
	if(strlen(ptr)==0)
		return -1;
	strcpy(cmd_tmp,ptr);

	//参数计数,第一个命令也算参数
	n=1;
	ptr = cmd_tmp;
	while(ptr && (ptr=strstr(ptr," ")))
	{
		ptr++;
		while(*ptr == ' ') ptr++;
		if('\0' != *ptr)
			n++;
	}
	exec_argv = new char*[n+1];

	//拷贝参数
	ptr = ptr2 = cmd_tmp;
	while(ptr && (ptr=strstr(ptr," ")))
	{
		*ptr = '\0';
		ptr++;
		if(0==i)
			strcpy(name,ptr2);

		exec_argv[i] = new char[ptr - ptr2 + 2];
		strcpy(exec_argv[i],ptr2);
		i++;
		while(*ptr == ' ') ptr++;
		ptr2 = ptr;
	}
	if(ptr2!='\0')
	{
		if(0==i)
			strcpy(name,ptr2);
		exec_argv[i] = new char[strlen(ptr2) + 2];
		strcpy(exec_argv[i],ptr2);
		i++;
	}
	exec_argv[i] = NULL;
	if(i==n)
		printf("$: cmd explain ok ! n = %d \n", n);

	//系统调用：
	exec_file(name,NULL,exec_argv);
	
	for(int i=0;i<n;++i)
		delete[] exec_argv[i];
	delete[] exec_argv;
	delete[] cmd_tmp;
	///////////////

	return 0;
};
int exec_command(const char* cmd)
{
	if(!cmd)
		return -1;
	
	int pid;
	pid=fork();
	switch(pid)
	{
	case -1:
		{
			perror("fork faild:");
			return -1;
		}
		break;
	case 0:
		{
			//再次创建子进程,让中间进程结束,第三层进程(应用进程)的父进程将被init接管,此时的进程即使远程操作断开,也不会被kill掉了
			pid = fork();
			if(pid < 0 || pid>0)
				exit(0); 

			system(cmd);

			printf("$: system(%s) exit\n",cmd);
			exit(0);
		}
		break;
	default:
		{
		}
		break;
	}
	waitpid(pid,NULL,0);
	printf("$: #execute_command : %s \n",cmd);
	return 0;
}
#endif

//*******************************************************

