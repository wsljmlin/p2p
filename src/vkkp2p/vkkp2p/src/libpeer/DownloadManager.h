#pragma once
#include "PeerManager.h"
#include "Tracker.h"
#include "Download.h"

typedef struct tagTurnInfo
{
	hash_t         hash;
	uint32         sessionID;
	int            type; //0:tur;1:nat
	unsigned int   beginTick;
	tagTurnInfo(void) {}
	tagTurnInfo(const hash_t& _hash,uint32 _sessionID,int _type,unsigned int _beginTick)
		: hash(_hash),sessionID(_sessionID),type(_type),beginTick(_beginTick) {}
}TurnInfo;

typedef struct tagReleaseInfo
{
	hash_t         hash;
	unsigned int   beginTick;
	tagReleaseInfo(void) {}
	tagReleaseInfo(const hash_t& _hash,unsigned int _beginTick)
		:hash(_hash),beginTick(_beginTick){}
}ReleaseInfo;

class DownloadManagerListener
{
public:
	virtual ~DownloadManagerListener(void){}
public:
	template<int I>
	struct S{enum {T=I};};

	typedef S<1> DLCreate;
	typedef S<3> DLStart;//真正开始搜索下载
	typedef S<4> DLStop;
	typedef S<5> DLDownloading;
	typedef S<6> DLBlockTable;
	typedef S<7> DLBlockDone;
	typedef S<8> DLFileDone;

	virtual void on(DLCreate,const DownloadInfo& inf) {}
	virtual void on(DLStart,const DownloadInfo& inf) {}
	virtual void on(DLStop,const DownloadInfo& inf) {}
	virtual void on(DLDownloading,const DownloadInfo& inf) {}
	virtual void on(DLBlockTable,const hash_t& hash,const BlockTable& inf) {}
	virtual void on(DLBlockDone,const hash_t& hash,int index) {}
	virtual void on(DLFileDone,const hash_t& hash) {}
};


class DownloadManager : public TimerHandler
	,public PeerDownloadHandler
	,private TrackerListener
	,private DownloadListener
	,public Speaker<DownloadManagerListener>
{
	//friend class Speaker<TrackerListener>;
	//friend class Speaker<DownloadListener>;
public:
	DownloadManager(void);
	~DownloadManager(void);
	
	typedef map<hash_t,Download*> DownloadMap;
	typedef DownloadMap::iterator DownloadIter;
	typedef list<TurnInfo> TurnList;
	typedef TurnList::iterator TurnIter;
	typedef list<ReleaseInfo> ReleaseList;
	typedef ReleaseList::iterator ReleaseIter;
public:
	int init();
	int fini();

	int create_download(const hash_t& hash,const string& path,const string& url="",int http_max_num=-1,size64_t size=0,size64_t offset=0,
		unsigned int blockSize=DEFAULT_BLOCK_SIZE,int ftype=FTYPE_VOD,bool isStart=true,int rdbftype=RDBF_AUTO);
#ifdef SM_VOD
	int create_download(const hash_t& hash,int playtype,const string& path,const string& url="",int http_max_num=-1,size64_t size=0,size64_t offset=0,
		unsigned int blockSize=DEFAULT_BLOCK_SIZE,int ftype=FTYPE_VOD,bool isStart=true,int rdbftype=RDBF_AUTO);
	int on_file_done(const hash_t& hash,int playtype, const hash_t& newHash,char done_type);
	int delete_file(const hash_t& hash,bool isDelPhy, bool isclear);
#endif
	int delete_download(const hash_t& hash);
	int start_download(const hash_t& hash);
	int stop_download(const hash_t& hash);
	Download* get_download(const hash_t& hash);
	int get_downloadinfo(const hash_t& hash,DownloadInfo& inf);
	int stop_download_all();
	int clear_download_all();

	int share_file(const hash_t& hash,const string& path);
	int delete_file(const hash_t& hash,bool isDelPhy);
	int read_refer(const hash_t& hash);
	int read_release(const hash_t& hash);
	int read_release_i(const hash_t& hash);
	//int auto_clear_cache(const hash_t& hashExclude);
	int get_source_list(const hash_t& hash,list<SourceInfo*>& ls);
	int add_source_list(const hash_t& hash,list<SourceInfo*>& ls);
	int http_add_source(const hash_t& hash,const string& url,bool trycreate);
	int on_attach_peer(const hash_t& hash,Peer* peer,int ctype);
	int on_attach_http_peer(const hash_t& hash,HttpPeer* peer,const string& url);

	int update_http_max_num(const hash_t& hash,int n);
	int get_connect_num(const hash_t& hash,int& p2pconn_num,int& httpconn_num);
public:
	virtual int on_peer_ready(Peer* peer,void* dref);
	virtual void on_peer_close(Peer* peer,void* dref);
	virtual int on_peer_turn(Peer* peer,uint32 sessionID);
	virtual int on_ptl_packet(Peer* peer,uint16 cmd,PTLStream& ss);

	bool create_turn_channel(unsigned int trackerID,unsigned int sessionID,const hash_t& hash);

	int on_file_done(const hash_t& hash,const hash_t& newHash,char done_type);
	
	virtual void on_timer(int e);
private:
	void TimerDownloadHeartbeat();
	void TimerClearTimeoutTurn();
	void TimerRelease();
	bool ExistDownload(const hash_t& hash,const string& path);

	virtual void on(TrackerListener::Connecting){}
	virtual void on(TrackerListener::Disconnected){}
	virtual void on(TrackerListener::Connected);
	
	virtual void on(DownloadListener::DLCreate,const DownloadInfo& inf);
	virtual void on(DownloadListener::DLQueue,const DownloadInfo& inf) {}
	virtual void on(DownloadListener::DLStart,const DownloadInfo& inf);
	virtual void on(DownloadListener::DLStop,void* dl,const DownloadInfo& inf);
	virtual void on(DownloadListener::DLDownloading,const DownloadInfo& inf);
	virtual void on(DownloadListener::DLBlockTable,const hash_t& hash,const BlockTable& inf);
	virtual void on(DownloadListener::DLBlockDone,const hash_t& hash,int index);
private:
	DownloadMap    m_downloads;
	TurnList       m_turns;
	ReleaseList    m_releases;
	unsigned int   m_iCurrTick;
	hash_t         m_hash;

};
typedef Singleton<DownloadManager> DownloadManagerSngl;
