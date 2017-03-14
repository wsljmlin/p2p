#include "Interface.h"
#include "Initializers.h"
#include "DownloadManager.h"
#include "FileStorage.h"
#include "Setting.h"
#include "Tracker.h"
#include "DownloadListManager.h"
#include "Util.h"
#include "PeerManager.h"
#include "Statistician.h"

#define CHECK_PEER_INIT_RETURN if(!is_peer_init()) return
#define CHECK_PEER_INIT_RETURN_V(v) if(!is_peer_init()) return v


#ifdef SM_DBG
#define INTERFACKE_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "PIF", __LINE__, ##arg)
#else
#define INTERFACKE_PRT(fmt, arg...)
#endif


enum {
	CMD_CREATE_DOWNLOAD,
	CMD_READ_REFER,
	CMD_READ_RELEASE,
	CMD_SHARE_FILE,
	CMD_DELETE_FILE,
	CMD_DOWNLOADLIST_OPEN,
	CMD_DOWNLOADLIST_CLOSE,
	CMD_DOWNLOADLIST_CLOSEALL,
	CMD_DOWNLOADLIST_GETLIST,
	CMD_DOWNLOADLIST_GETLIST_INFO,
	CMD_DOWNLOADLIST_NEED_DOWNLOAD,
	CMD_DOWNLOADLIST_NEED_DOWNLOAD_END,
	CMD_DOWNLOADLIST_GET_ALL_DLINFO,
#ifdef SM_VOD
	CMD_DOWNLOADLIST_GET_CACHETIME,
	CMD_DOWNLOADLIST_GET_FILEURL,
	CMD_DOWNLOADLIST_GET_FILEPATH
#endif
};
typedef struct tagCmdInfo{
	hash_t hash;
	string path;
	string url;
	uint64 fsize;
	uint64 offset;
	int blockSize;
	int ftype;
	bool isStart;
	int *presult;
	void *data;
#ifdef SM_VOD
	string *purl;
	string *ppath;
	int playtype;
	int *time;
#endif

	tagCmdInfo(){
		path="";
		fsize=0;
		offset=0;
		blockSize=0;
		ftype=0;
		isStart=false;
		presult=NULL;
		data = NULL;
#ifdef SM_VOD
		playtype = 0;
#endif

	}
}CmdInfo;

int socket_init()
{
#ifdef _WIN32
	WSADATA wsaData;
	if(0!=WSAStartup(0x202,&wsaData))
	{
		perror("WSAStartup false! : ");
		return -1;
	}
	return 0;
#endif
	return 0;
}

void socket_fini()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

//**********************************************************
PIF* PIF::__instance = NULL;
PIF::PIF(void)
{
}

PIF::~PIF(void)
{
	m_queue.ClearMessage(_clear_message_func);
}

int PIF::load_arg(int argc,char** argv)
{
	SettingSngl::instance()->load_arg(argc,argv);
	return 0;
}
int PIF::init()
{
	socket_init();
	return peer_init();
}
void PIF::fini()
{
	peer_fini();
	socket_fini();
}
int PIF::create_download(const hash_t& hash,const string& path,const string& url/*=""*/,uint64 size/*=0*/,uint64 offset/*=0*/,
		unsigned int blockSize/*=DEFAULT_BLOCK_SIZE*/,int ftype/*=FTYPE_VOD*/,bool isStart/*=true*/)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	if(SettingSngl::instance()->get_is_minor_version())
		return -1;
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	inf->url = url;
	inf->fsize = size;
	inf->offset = offset;
	inf->blockSize = blockSize;
	inf->ftype = ftype;
	inf->isStart = isStart;

	char buf[45];
	hash.to_string(buf,45);
	string str = buf;
	if(path.empty())
	{
		inf->path = SettingSngl::instance()->get_cache_path() + str.substr(2) + ".vkk";
	}
	else
	{
		inf->path = path;
	}
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_CREATE_DOWNLOAD,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}
int PIF::read_refer(const hash_t& hash)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	this->m_queue.AddMessage(new Message(CMD_READ_REFER,inf,0));
	return 0;
}
int PIF::read_release(const hash_t& hash)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	this->m_queue.AddMessage(new Message(CMD_READ_RELEASE,inf,0));
	return 0;
}
uint64 PIF::get_filesize(const hash_t& hash)
{
	CHECK_PEER_INIT_RETURN_V(0);
	return FileStorageSngl::instance()->get_file_size(hash);
}
int PIF::get_filedata(const hash_t& hash,char *buf,int size,uint64 offset)
{
	CHECK_PEER_INIT_RETURN_V(0);
	return FileStorageSngl::instance()->read_filedata_trymemcache(hash,buf,size,offset);
}
int PIF::get_downloadinfo(const hash_t& hash,DownloadInfo& inf)
{
	return DownloadManagerSngl::instance()->get_downloadinfo(hash,inf);
}
int PIF::check_fileready(const hash_t& hash)
{
	return NULL==FileStorageSngl::instance()->get_readyinfo(hash)?0:1;
}
int PIF::get_sharespeed()
{
	return  (int)StatisticianSngl::instance()->m_sendSpeedometer.get_speed(3);
}

int PIF::share_file(const hash_t& hash,const string& path)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	inf->path = path;
	this->m_queue.AddMessage(new Message(CMD_SHARE_FILE,inf,0));
	return 0;
}
int PIF::delete_file(const hash_t& hash,bool isDelPhy)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	inf->isStart = isDelPhy;
	this->m_queue.AddMessage(new Message(CMD_DELETE_FILE,inf,0));
	return 0;
}
int PIF::check_state()
{
	return TrackerSngl::instance()->is_login()?0:-1;
}
int PIF::get_mystate(MyState_t& st)
{
	st.ver = VERSION_NUM;
	st.user_type = SettingSngl::instance()->get_user_type();
	st.islogin = TrackerSngl::instance()->is_login();
	st.login_times = TrackerSngl::instance()->get_login_times();
	st.begin_time = TrackerSngl::instance()->m_strBeginTime;
	st.last_login_time = TrackerSngl::instance()->m_strLastLoginTime;
	st.is_minor_version = SettingSngl::instance()->get_is_minor_version();
	st.tracker_ip = SettingSngl::instance()->get_tracker_ip();
	st.tracker_port = SettingSngl::instance()->get_tracker_port();
	st.nat_type = g_netLiveInfo.natType;
	st.real_ip = Util::ip_htoas(g_netLiveInfo.tcpRealIP);
	st.udp_local_port = g_netLiveInfo.udpLocalPort;
	st.tcp_local_port = g_netLiveInfo.tcpLocalPort;
	st.new_peer_times = PeerManagerSngl::instance()->get_new_peer_times();
	st.online_peers = PeerManagerSngl::instance()->get_online_peers();
	return 0;
}
int PIF::get_all_downloadlists(list<string>& ls)
{
	return DownloadListManagerSngl::instance()->get_all_downloadlists(ls);
}
int PIF::downloadlist_open(const string& url,const string& name,bool closeother,bool autoclose/*=false*/)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	if(SettingSngl::instance()->get_is_minor_version())
		return -1;
	CmdInfo *inf=new CmdInfo();
	inf->url = url;
	inf->path = name;
	inf->ftype = closeother?1:0;
	inf->isStart = autoclose;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_OPEN,inf,0));
	while(-2==result)
		Sleep(10);
	return result;
}

#ifdef SM_VOD
int PIF::downloadlist_open(const string& url,const string& name,bool closeother, int playtype,bool autoclose/*=false*/)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	if(SettingSngl::instance()->get_is_minor_version())
		return -1;
	CmdInfo *inf=new CmdInfo();
	inf->url = url;
	inf->path = name;
	inf->ftype = closeother?1:0;
	inf->isStart = autoclose;
	inf->playtype = playtype;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_OPEN,inf,0));
	while(-2==result)
		Sleep(10);
	return result;
}

int PIF::create_download(const hash_t& hash, int playtype, const string& path,const string& url/*=""*/,uint64 size/*=0*/,uint64 offset/*=0*/,
		unsigned int blockSize/*=DEFAULT_BLOCK_SIZE*/,int ftype/*=FTYPE_VOD*/,bool isStart/*=true*/)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	if(SettingSngl::instance()->get_is_minor_version())
		return -1;
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	inf->url = url;
	inf->fsize = size;
	inf->offset = offset;
	inf->blockSize = blockSize;
	inf->ftype = ftype;
	inf->isStart = isStart;
	inf->playtype = playtype;

	char buf[45];
	hash.to_string(buf,45);
	string str = buf;
	if(path.empty())
	{
		inf->path = SettingSngl::instance()->get_cache_path() + str.substr(2) + ".vkk";
	}
	else
	{
		inf->path = path;
	}
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_CREATE_DOWNLOAD,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}

int PIF::downloadlist_getcachetime(const string url,int& time)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->url = url;
	inf->time = &time;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_GET_CACHETIME,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}

int PIF::downloadlist_getfileurl(const hash_t& hash,string& url) {
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	inf->purl = &url;
	//等待结果 
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_GET_FILEURL,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}

int PIF::downloadlist_getfilepath(const hash_t& hash,string& path) {
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->hash = hash;
	inf->ppath = &path;
	//等待结果 
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_GET_FILEPATH,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;

}

#endif

int PIF::downloadlist_close(const string& url)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->url = url;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_CLOSE,inf,0));
	return 0;
}
int PIF::downloadlist_closeall()
{
	CHECK_PEER_INIT_RETURN_V(-1);
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_CLOSEALL,0,0));
	return 0;
}

int PIF::downloadlist_getlist(const string& url,list<string>& ls)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->url = url;
	inf->data = (void*)&ls;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_GETLIST,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}

int PIF::downloadlsit_need_download(const string& strhash)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->url = strhash;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_NEED_DOWNLOAD,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}
int PIF::downloadlsit_need_download_end(const string& strhash)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->url = strhash;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_NEED_DOWNLOAD_END,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}
int PIF::downloadlist_getlist_info(const string& url,MsgDownloadlistInfo_t& di)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->url = url;
	inf->data = (void*)&di;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_GETLIST_INFO,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;
}

int PIF::downloadlist_get_all_dlinfo(list<MsgDownloadlistInfo2_t>& ls)
{
	CHECK_PEER_INIT_RETURN_V(-1);
	CmdInfo *inf=new CmdInfo();
	inf->data = (void*)&ls;
	//等待结果
	int result = -2;
	inf->presult = &result;
	this->m_queue.AddMessage(new Message(CMD_DOWNLOADLIST_GET_ALL_DLINFO,inf,0));
	while(-2==result)
		Sleep(10);
	if(result>=0)
		return 0;
	return result;

}

void PIF::handle_root()
{
	CHECK_PEER_INIT_RETURN;
	Message* msg = NULL;
	while((msg=m_queue.GetMessage(0)))
	{
		switch(msg->cmd)
			{
		case CMD_CREATE_DOWNLOAD:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				INTERFACKE_PRT("create download by player!!\n");
#ifdef SM_VOD
				int ret = DownloadManagerSngl::instance()->create_download(inf->hash,inf->playtype,inf->path,inf->url,-1,inf->fsize,inf->offset
					,inf->blockSize,inf->ftype,inf->isStart,SettingSngl::instance()->get_cache_filetype());
#else
				int ret = DownloadManagerSngl::instance()->create_download(inf->hash,inf->path,inf->url,-1,inf->fsize,inf->offset
					,inf->blockSize,inf->ftype,inf->isStart,SettingSngl::instance()->get_cache_filetype());
#endif /* end of SM_VOD */
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
		case CMD_READ_REFER:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				DownloadManagerSngl::instance()->read_refer(inf->hash);
			}
			break;
		case CMD_READ_RELEASE:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				DownloadManagerSngl::instance()->read_release(inf->hash);
			}
			break;
		case CMD_SHARE_FILE:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				DownloadManagerSngl::instance()->share_file(inf->hash,inf->path);
			}
			break;
		case CMD_DELETE_FILE:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
#ifdef SM_VOD
				DownloadManagerSngl::instance()->delete_file(inf->hash,inf->isStart, inf->isStart);
#else
				DownloadManagerSngl::instance()->delete_file(inf->hash,inf->isStart);
#endif  /* end of SM_VOD */
			}
			break;
		case CMD_DOWNLOADLIST_OPEN:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
#ifdef SM_VOD
				int ret = DownloadListManagerSngl::instance()->open_downloadlist(inf->url,inf->path,inf->ftype?true:false,inf->playtype,inf->isStart);
#else
				int ret = DownloadListManagerSngl::instance()->open_downloadlist(inf->url,inf->path,inf->ftype?true:false,inf->isStart);
#endif /* end of SM_VOD */
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
#ifdef SM_VOD
			case CMD_DOWNLOADLIST_GET_CACHETIME:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				int ret = DownloadListManagerSngl::instance()->get_downloadcachetime(inf->url, *(inf->time));
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
			case CMD_DOWNLOADLIST_GET_FILEURL:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				int ret = DownloadListManagerSngl::instance()->get_fileurl(inf->hash, *(inf->purl));
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
			case CMD_DOWNLOADLIST_GET_FILEPATH:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				int ret = DownloadListManagerSngl::instance()->get_filepath(inf->hash, *(inf->ppath));
				if(inf->presult)
					*(inf->presult) = ret;
			}
				break;
#endif /* end of SM_VOD */
		case CMD_DOWNLOADLIST_CLOSE:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				DownloadListManagerSngl::instance()->close_downloadlist(inf->url);
			}
			break;
		case CMD_DOWNLOADLIST_CLOSEALL:
			{
				DownloadListManagerSngl::instance()->closeall_downloadlist();
			}
			break;
		case CMD_DOWNLOADLIST_GETLIST:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				list<string>& ls = *(list<string>*)inf->data;
				int ret = DownloadListManagerSngl::instance()->get_downloadlist(inf->url,ls);
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
		case CMD_DOWNLOADLIST_NEED_DOWNLOAD:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				int ret = DownloadListManagerSngl::instance()->need_download(inf->url.c_str());
				
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
		case CMD_DOWNLOADLIST_NEED_DOWNLOAD_END:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				int ret = DownloadListManagerSngl::instance()->need_download_end(inf->url.c_str());
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
		case CMD_DOWNLOADLIST_GETLIST_INFO:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				MsgDownloadlistInfo_t *di = (MsgDownloadlistInfo_t*)inf->data;
				int ret = DownloadListManagerSngl::instance()->get_downloadlist_info(inf->url,di);
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
		case CMD_DOWNLOADLIST_GET_ALL_DLINFO:
			{
				CmdInfo *inf = (CmdInfo*)msg->data;
				list<MsgDownloadlistInfo2_t>& ls = *((list<MsgDownloadlistInfo2_t>*)inf->data);
				int ret = DownloadListManagerSngl::instance()->get_all_downloadlist_info(ls);
				if(inf->presult)
					*(inf->presult) = ret;
			}
			break;
		default:
			break;
		}
		_clear_message_func(msg);
	}
}
void PIF::_clear_message_func(Message* msg)
{
	if(!msg)
	{
		assert(0);
		return;
	}
	switch(msg->cmd)
		{
	case CMD_CREATE_DOWNLOAD:
	case CMD_READ_REFER:
	case CMD_READ_RELEASE:
	case CMD_SHARE_FILE:
	case CMD_DELETE_FILE:
	case CMD_DOWNLOADLIST_OPEN:
	case CMD_DOWNLOADLIST_CLOSE:
	case CMD_DOWNLOADLIST_GETLIST:
	case CMD_DOWNLOADLIST_NEED_DOWNLOAD:
	case CMD_DOWNLOADLIST_NEED_DOWNLOAD_END:
	case CMD_DOWNLOADLIST_GETLIST_INFO:
	case CMD_DOWNLOADLIST_GET_ALL_DLINFO:
		{
			delete (CmdInfo*)msg->data;
		}
		break;
	default:
		break;
	}
	delete msg;
}
