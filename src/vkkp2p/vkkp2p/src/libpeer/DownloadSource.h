#pragma once
#include "Peer.h"
#include "DownloadInfo.h"
#include "FileInfo.h"
#include "Speedometer.h"

//视频播放速度
#define DEFAULT_STREAM_SPEED  60000
#define STREAM_SIZE_PER_SECOND (60<<10)

enum DownloadState{DOWNS_INIT=0,DOWNS_STOP,DOWNS_QUEUE,DOWNS_START};

class DownloadListener
{
public:
	virtual ~DownloadListener(void){}
public:
	template<int I>
	struct S{enum {T=I};};

	typedef S<1> DLCreate;
	typedef S<2> DLQueue;
	typedef S<3> DLStart;//真正开始搜索下载
	typedef S<4> DLStop;
	typedef S<5> DLDownloading;
	typedef S<6> DLBlockTable;
	typedef S<7> DLBlockDone;

	virtual void on(DLCreate,const DownloadInfo& inf) {}
	virtual void on(DLQueue,const DownloadInfo& inf) {}
	virtual void on(DLStart,const DownloadInfo& inf) {}
	virtual void on(DLStop,void* dl,const DownloadInfo& inf) {}
	virtual void on(DLDownloading,const DownloadInfo& inf) {}
	virtual void on(DLBlockTable,const hash_t& hash,const BlockTable& inf) {}
	virtual void on(DLBlockDone,const hash_t& hash,int index) {}
};

typedef struct tagSourceInfo
{
	PTL_P2T_PeerInfo   source;
	int                ctype; //连接类型如:TCP_CONN
	int                connectTimes;  //连接次数
	int                connectFailTimes;
	int					noDataTimes; //无法获得数据次数
	int                ref;
	unsigned int       lastUseTick;
	tagSourceInfo(void) : ctype(UNKNOW),connectTimes(0),connectFailTimes(0),noDataTimes(0),ref(0),lastUseTick(0) {}
}SourceInfo;

class DownloadSource : public Speaker<DownloadListener>
	, public PeerReadLimitSinker
{
public:
	DownloadSource(void);
	virtual ~DownloadSource(void);

	typedef struct tagBlockInfo
	{
		unsigned int index;
		unsigned int pos;
		unsigned int blockSize;
		tagBlockInfo(void) : index(0),pos(0),blockSize(0) {}
	}BlockInfo;
	typedef struct tagTableInfo
	{
		unsigned int index_offset;
		unsigned int index_num;
		unsigned int last_update_tick;
		unsigned int state; //0-未完整的，1-已经完整的表
		unsigned int assign_fails; //分配任何失败次数
		tagTableInfo(void):index_offset(0),index_num(0),last_update_tick(0),state(0),assign_fails(0) {}
	}TableInfo;

	enum PeerState
	{
		PS_DISCONNECT,
		PS_READY,
		PS_REQUESTING_FILE_TABLE,
		PS_REQUESTING_FILE_BLOCK,
		PS_DOWNLOADING,
		PS_ASSIGN_JOB_PAUSE,
		PS_ASSIGN_FAIL_WAITING,
		PS_NOTHING_TO_DOWNLOAD
	};
	typedef struct tagPeerData
	{
		
		PeerState                    state;
		int                          ctype;
		list<BlockInfo>              blockList;
		list<BlockInfo>              blockListNew; //分配任务时先分到这些,PTL时再合到blockList
		BlockTable                   bt_finished;
		TableInfo					 bt_info;
		int                          lagTimes;  //竞争下载时的落后次数
		Speedometer<unsigned int>    speedometer;
		uint32                       sourceSessionID;
		bool                         isReady;    //判断是否连接可用
		bool                         isMySource; //由我创建
		int                          lagBlockI;  //竞争落后之后下次不再下载此块 no longer download

		tagPeerData(void) : state(PS_DISCONNECT),ctype(UNKNOW)
			,lagTimes(0),sourceSessionID(0),isReady(false),isMySource(false),lagBlockI(-1) {}

		bool exist_job(int i)
		{
			if(i==lagBlockI) return true;
			for(list<BlockInfo>::iterator it=blockList.begin();it!=blockList.end();++it)
			{
				if((int)(*it).index == i)
					return true;
			}
			return false;
		}
	}PeerData;

	typedef map<uint32,SourceInfo*> SourceMap;
	typedef SourceMap::iterator SourceIter;

	typedef map<Peer*,PeerData*> PeerMap;
	typedef PeerMap::iterator PeerIter;

	typedef map<int,list<Peer*> > BlockRefMap;
	typedef BlockRefMap::iterator BlockRefIter;
public:
	int on_peer_turn(Peer *peer,int sessionID);
	int get_source_num() const { return m_sources.size();}
	int get_source_list(list<SourceInfo*>& ls);
	int add_source_list(list<SourceInfo*>& ls);
	int get_p2pconnect_amount()const {return m_iConnectAmount;}

	int attach_peer(Peer *peer,bool isMySource=false,int ctype=0);
	int detach_peer(Peer *peer,bool reattach=false,bool bunsed=false,bool bput_all=false);
protected:
	SourceInfo* add_source(PTL_P2T_PeerInfo* source);
	int clear_source_all();
	void load_source_server_cache();
	void load_client_sn();//点播前写入到clientconf.ini中的sn
	int put_peer_all();

	int check_create_channel();
	int create_channel();
	int create_channel_i(SourceInfo& inf);
	int get_connect_type(const SourceInfo& inf);

	PeerData* get_peerdata(Peer* peer);

	virtual int cancel_job(Peer *peer,int index=-1,bool allowTryAssign=false,bool sendReqCancel=false){return 0;}
	virtual int cancel_job(int index,bool allowTryAssign=false,bool sendReqCancel=false){return 0;}//取消占有此块任务的所有连接

	int update_filesize(size64_t size);
	virtual int get_block_speed(int index,int seconds);
	int add_block_ref(int index,Peer* peer);
	int del_block_ref(int index,Peer* peer);
	int get_block_ref_num(int index);
	bool is_urgent_win_ok(int need_i);

	int on_block_done(int index,int type);
	int on_file_done();
	int on_file_memcache_done();
#ifdef SM_VOD
	void _set_playtype(int playtype) { m_playtype= playtype;}
	int _get_playtype() {return m_playtype;}
#endif

protected:
	DownloadState             m_state;
	BlockRefMap               m_blockRefs;
	Speedometer<uint64>       m_speedometer;
	FileInfo*                 m_fi;
	PeerMap                   m_peers;
	SourceMap                 m_sources;
	
	DownloadInfo              m_downInfo;
	bool                      m_isFinished;
	bool                      m_isUseClientSNOnly;

	bool                      m_isAssignJobPause;
	int                       m_lastPauseNeedI;

	unsigned int              m_iConnectAmount;
	unsigned int              m_iCurrTick;
	//unsigned int              m_iStartTick;
	//unsigned int              m_iLastSearchTick;
	//unsigned int              m_iSearchCycle;
	unsigned int              m_iLastCreateConnectTick;
	unsigned int			  m_assign_fails;
#ifdef SM_VOD
	int 				m_playtype;
#endif

};
