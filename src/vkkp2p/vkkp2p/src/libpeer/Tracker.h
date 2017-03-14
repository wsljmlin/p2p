#pragma once
#include "Peer.h"
#include "Handler.h"

string GetCurrStrTime();

class TrackerListener
{
public:
	virtual ~TrackerListener(void){}

	template<int I>
	struct S{ enum{T=I}; };

	typedef S<1> Connecting;
	typedef S<2> Connected;
	typedef S<3> Disconnected;

	virtual void on(Connecting){}
	virtual void on(Connected){}
	virtual void on(Disconnected){}
};

typedef struct tagTrackAddr
{
	string ip;
	unsigned short port;
}TrackAddr_t;

class Tracker : public TimerHandler
	, private PeerListener
	, public Speaker<TrackerListener>

{
	friend class Speaker<PeerListener>;
public:
	Tracker(void);
	virtual ~Tracker(void);

	typedef list<TrackAddr_t> TrackAddrList;
	typedef TrackAddrList::iterator TrackAddrIter;

	typedef list<PTL_P2T_FileInfo*> ShareFileList;
	typedef ShareFileList::iterator ShareFileIter;
public:
	int init();
	void fini();

	int on_update_nattype();
	
#ifdef SM_VOD
	int PTL_ReportShareFile(const hash_t& hash, int filetype);
	int PTL_ReportShareFile(const hash_t& hash, int filetype, uint64 size);
	int PTL_ReportRemoveFile(const hash_t& hash, int filetype);
	int PTL_ReportStartDownloadFile(const hash_t& hash, int filetype);
	int PTL_ReportStartDownloadList(const hash_t& hash,int filetype,const string& url);
	int PTL_ReportStopDownloadFile(const hash_t& hash, int filetype);
	int PTL_RequestFileSource(const hash_t& hash, int filetype);
	int PTL_ReportDownloadWrong(const hash_t& hash,const hash_t& newHash, int filetype);
#endif

	int PTL_ReportShareFile(const hash_t& hash);
	int PTL_ReportShareFile(const hash_t& hash,uint64 size);
	int PTL_ReportRemoveFile(const hash_t& hash);
	int PTL_ReportStartDownloadFile(const hash_t& hash);
	int PTL_ReportStartDownloadList(const hash_t& hash,const string& url);
	int PTL_ReportStopDownloadFile(const hash_t& hash);
	int PTL_RequestFileSource(const hash_t& hash);
	int PTL_ReportDownloadWrong(const hash_t& hash,const hash_t& newHash);
	int PTL_ReportDownloadFileSpeed(PTL_P2T_ReportDownloadFileSpeed& inf);
	int PTL_ReportDownloadFileInfo(PTL_P2T_ReportDownloadFileInfo& inf);
	int PTL_RequestConnTurn(uint32 trackID,uint32 sessionID);

	virtual void on_timer(int e);
	bool is_login() const { return m_isLogin;}
	int get_login_times() const {return m_loginTimes;}
private:
	int AutoLogin();
	int Login(const char* ip,unsigned short port);
	int LogOut();
	int OnReadData(const char* buf,int len);
	template<typename T>
	int SendPTLPacket(uint16 cmd,T& inf,int iMaxSize);

	void OnSencod();
	void OnShareFileTimer();
	void ClearShareFileList();
	void ClearShareFile(const hash_t& hash);

	int PTL_RequestLogin();
	int PTL_ReportNat();
	int PTL_ReportShareFileAll();
	int PTL_RequestKeeplive();
	int PTL_ReportStat();
	int PTL_ReportError();
	int PTL_ReportDownloadListMaxnum();

	int ON_PTL_ResponseLogin(PTLStream& ss);
	int ON_PTL_ResponseFileSource(PTLStream& ss);
	int ON_PTL_ResponseConnTurn(PTLStream& ss);
	int ON_PTL_ResponseKeeplive(PTLStream& ss);
	int ON_PTL_RequestStartDownloadList(PTLStream& ss);
	int ON_PTL_RequestStopDownloadList(PTLStream& ss);

	virtual void on(PeerListener::Connecting,Peer* peer);
	virtual void on(PeerListener::Connected,Peer* peer);
	virtual void on(PeerListener::Disconnected,Peer* peer);
	virtual void on(PeerListener::Data,Peer* peer,char* buf,int len);
	virtual void on(PeerListener::Writable,Peer* peer);

private:
	Peer*            m_peer;
	bool             m_isLogin;
	unsigned int	 m_loginTimes;
	bool             m_isLastLoginSucceed;
	bool             m_isReportShareAll;
	TrackAddr_t      m_trackAddr;
	ShareFileList    m_shareFiles;
	unsigned int     m_iCurrTick;
	unsigned int     m_iLastReadTick; //最后一次读取到数据的时间
	unsigned int     m_iOfflineCount; //离线时长计数
	int              m_iUseTrackAddr_i;
	PTL_Head	     m_head;
public:
	string           m_strBeginTime;
	string           m_strLastLoginTime;
};
typedef Singleton<Tracker> TrackerSngl;

