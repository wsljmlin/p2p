#pragma once
#include "DownloadList.h"

//test playlist_url: http://116.246.24.110/live1_2/data/HZSHT00010_.txt
//response:
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/1.hdr
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19742.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19743.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19744.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19745.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19746.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19747.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19748.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19749.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19750.dat
//http://shdemo.big-pineapple.com/live1_2/HZSHT00010/20110727/19751.dat




//本地url格式：
//http://127.0.0.1:7080/playlist/open.do?url=playlist_url
//http://127.0.0.1:7080/playlist/close.do?url=playlist_url
//http://127.0.0.1:7080/playlist/closeall.do
//http://127.0.0.1:7080/playlist/getlist.do?url=playlist_url
//http://127.0.0.1:7080/playlist/file/strhash.ts
class DownloadListManager
{
public:
	DownloadListManager(void);
	~DownloadListManager(void);
	typedef map<hash_t,DownloadList*> DLMap;
	typedef DLMap::iterator DLIter;
	typedef map<hash_t,string> DLMap2;
	typedef DLMap2::iterator DLIter2;
public:
	int init();
	void fini();
	int open_downloadlist(const string& url,const string& name,bool closeother,bool autoclose=false,bool bopenbytracker=false); //bopenbytracker:表示是由tracker调度启动的
#ifdef SM_VOD
	int open_downloadlist(const string& url,const string& name,bool closeother,int playtype,bool autoclose=false,bool bopenbytracker=false); //bopenbytracker:表示是由tracker调度启动的
	int get_downloadcachetime(const string& url, int& time);
	int get_fileurl(const hash_t& hash, string& url) ;
	int get_filepath(const hash_t& hash, string& path) ;
#endif
	int close_downloadlist(const string& url,bool bopenbytracker=false);
	int close_downloadlist(const hash_t& hash,bool bopenbytracker=false);
	int closeall_downloadlist();
	int closeall_downloadlist_exclude(const hash_t& hash);
	int get_downloadlist(const string& url,list<string>& ls);
	int get_downloadlist_info(const string& url,MsgDownloadlistInfo_t *inf);
	int get_all_downloadlist_info(list<MsgDownloadlistInfo2_t>& ls);
	int need_download(const char* strhash);
	int need_download_end(const char* strhash);
	int get_all_downloadlists(list<string>& ls);

	int on_response_source(const hash_t& hash,PTL_P2T_ResponseFileSource& rsp);
	int get_token_hash(const hash_t& hash,hash_t& token_hash);
	void on_tracker_connected();
	int on_file_done(const hash_t& hash,char done_type);
	int on_file_stop(const hash_t& hash,void* dl);
	int on_peer_reattach(const hash_t& hash,Peer* peer,int ctype);
	int on_http_peer_reattach(const hash_t& hash,HttpPeer* peer);
	int on_peer_connect_failed(const hash_t& hash,unsigned int sessionID); //连接失败计数，失败多的丢弃源
	int on_peer_req_nodata(const hash_t& hash,unsigned int sessionID); //一次连接完成，从连接中获取不到任何数据
	int on_peer_req_havedata(const hash_t& hash,unsigned int sessionID); //一次连接完成，从连接中成功获取到过数据
	int on_download_bytes(const hash_t& hash,int size,int iptype,int utype);
	int on_connection(const hash_t& hash,int ctype,bool succeed);
	int on_share_bytes(const hash_t& hash,int size,int iptype); 
	void on_second();
private:
private:
	DLMap m_dlmap;
	DLMap2 m_dlmap2;

};

typedef Singleton<DownloadListManager> DownloadListManagerSngl;

	//,private DownloadManagerListener
	//virtual void on(DownloadManagerListener::DLCreate,const DownloadInfo& inf);
	//virtual void on(DownloadManagerListener::DLStart,const DownloadInfo& inf);
	//virtual void on(DownloadManagerListener::DLStop,const DownloadInfo& inf);
	//virtual void on(DownloadManagerListener::DLDownloading,const DownloadInfo& inf);
	//virtual void on(DownloadManagerListener::DLBlockTable,const hash_t& hash,const BlockTable& inf);
	//virtual void on(DownloadManagerListener::DLBlockDone,const hash_t& hash,int index);
	//virtual void on(DownloadManagerListener::DLFileDone,const hash_t& hash);


