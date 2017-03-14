#pragma once
#include "Peer.h"
#include "FileInfo.h"
#include "Speedometer.h"
#include "DownloadSource2.h"




class Download : public DownloadSource2
{
public:
	Download(void);
	virtual ~Download(void);

public:
	bool create_new_download(const hash_t& hash,const string& path,const string& url="",int http_max_num=-1,size64_t size=0,size64_t offset=0,
		unsigned int blockSize=DEFAULT_BLOCK_SIZE,int ftype=FTYPE_VOD,bool isStart=true,int rdbftype=RDBF_AUTO);
	bool create_old_download(FileInfo *fi,const string& url="",int http_max_num=1,bool isStart=true);
	int start();
	int stop();
	int on_second();
	uint64 get_download_speed(int sec) {return m_speedometer.get_speed(sec);}
	unsigned int get_zerospeed_count() const {return m_iZeroSpeedCount;}
	const DownloadInfo& get_downloadinfo();

	int on_peer_ready(Peer* peer);
	void on_peer_close(Peer* peer);
	int on_tracker_connected();

	int ON_PTL_ResponseFileSource(Peer* peer,PTL_P2T_ResponseFileSource& inf);
	int ON_PTL_ResponseFileBlockTable(Peer* peer,PTL_P2P_ResponseFileBlockTable& inf);
	int ON_PTL_ResponseFileBlocks(Peer* peer,PTL_P2P_ResponseFileBlocks& inf);
	int ON_PTL_ResponseFileBlocksData(Peer* peer,PTL_P2P_ResponseFileBlocksData& inf);

	virtual int on_peer_readlimit(Peer* peer);
#ifdef SM_VOD
	void set_playtype(int playtype) { _set_playtype(playtype);}
	int get_playtype() { return _get_playtype();}
#endif

private:
	void search_source();

	template<typename T>
	int send_PTL_packet(Peer* peer,uint16 cmd,T& inf,int iMaxSize);

	int PTL_RequestFileBlockTable(Peer* peer);
	int PTL_RequestFileBlocks(Peer* peer);
	int PTL_CancelFileBlocks(Peer* peer,int index);

	unsigned int get_update_blocktable_index_offset(PeerData* data);
	int assign_job(Peer *peer,bool allowReqBlockTable=true);
	void resume_assign_job();
	void check_assign_fails();
	virtual int cancel_job(Peer *peer,int index=-1,bool allowTryAssign=false,bool sendReqCancel=false);
	virtual int cancel_job(int index,bool allowTryAssign=false,bool sendReqCancel=false); //取消占有此块任务的所有连接
	int auto_change_demand_point(bool isReAssign=true);
	bool check_need_win_done();

private:

	unsigned int              m_iStartTick;
	unsigned int              m_iLastSearchTick;
	unsigned int              m_iSearchCycle;
	unsigned int			  m_iZeroSpeedCount;
	PTL_Head				  m_head;
};

