#pragma once
#include "DownloadInfo.h"
#include "consts.h"
#include "MessageQueue.h"
#include "MessageInfo.h"

typedef struct tag_MyState
{
	int ver;
	int user_type;
	bool islogin; //Ê±·ñµÇÂ½ÉÏtracker
	int login_times;
	string real_ip;
	int tcp_local_port;
	int udp_local_port;
	string tracker_ip;
	int tracker_port;
	int nat_type;
	char is_minor_version;
	string begin_time;
	string last_login_time;
	int new_peer_times;
	int online_peers;
	
}MyState_t;

//peer interface ->PIF
class Scheduler;
class PIF
{
	friend class Scheduler;
public:
	PIF(void);
	virtual ~PIF(void);
	static PIF* __instance;
	static PIF* instance() { if(!__instance) __instance=new PIF();return __instance;}
	static void destroy() { if(__instance){delete __instance;__instance=NULL;}}
public:
	int load_arg(int argc,char** argv);
	int init();
	void fini();
	virtual int create_download(const hash_t& hash,const string& path,const string& url="",uint64 size=0,uint64 offset=0,
		unsigned int blockSize=DEFAULT_BLOCK_SIZE,int ftype=FTYPE_VOD,bool isStart=true);
	virtual int read_refer(const hash_t& hash);
	virtual int read_release(const hash_t& hash);
	virtual uint64 get_filesize(const hash_t& hash);
	virtual int get_filedata(const hash_t& hash,char *buf,int size,uint64 offset);
	virtual int get_downloadinfo(const hash_t& hash,DownloadInfo& inf);
	virtual int check_fileready(const hash_t& hash);//0:unready ;1:ready
	virtual int get_sharespeed();

	virtual int share_file(const hash_t& hash,const string& path);
	virtual int delete_file(const hash_t& hash,bool isDelPhy);
	virtual int check_state();
	virtual int get_mystate(MyState_t& st);
	virtual int get_all_downloadlists(list<string>& ls);

	virtual int downloadlist_open(const string& url,const string& name,bool closeother,bool autoclose=false);
#ifdef SM_VOD
	virtual int downloadlist_open(const string& url,const string& name,bool closeother, int playtype,bool autoclose=false);
	virtual int create_download(const hash_t& hash, int playtype, const string& path,const string& url="",uint64 size=0,uint64 offset=0,
		unsigned int blockSize=DEFAULT_BLOCK_SIZE,int ftype=FTYPE_VOD,bool isStart=true);
	virtual int downloadlist_getcachetime(const string url,int& time);
	virtual int downloadlist_getfileurl(const hash_t& hash,string& url);
	virtual int downloadlist_getfilepath(const hash_t& hash,string& path);
#endif
	virtual int downloadlist_close(const string& url);
	virtual int downloadlist_closeall();
	virtual int downloadlist_getlist(const string& url,list<string>& ls);
	virtual int downloadlsit_need_download(const string& strhash);
	virtual int downloadlsit_need_download_end(const string& strhash);
	virtual int downloadlist_getlist_info(const string& url,MsgDownloadlistInfo_t& di);
	virtual int downloadlist_get_all_dlinfo(list<MsgDownloadlistInfo2_t>& ls);
private:
	void handle_root();
	static void _clear_message_func(Message* msg);
private:
	MessageQueue m_queue;
};
