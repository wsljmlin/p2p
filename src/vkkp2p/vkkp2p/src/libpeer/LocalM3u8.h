#pragma once
#include "commons.h"

class LocalM3u8
{
private:
	LocalM3u8(void);
	~LocalM3u8(void);
	typedef struct tagFileInfo
	{
		string path;
		hash_t hash;
	}FileInfo_t;
public:
	static LocalM3u8* open(const string& url,const string& path);
	void close();
	int p2p_share();
	int update();
	
private:
	string m_url,m_preurl;
	string m_path,m_rootdir;
	hash_t m_hash;
	list<FileInfo_t> m_file_list;
	string m_path_errlog,m_path_bak;
};

//**********************************************************************
class LocalM3u8Mgr : public TimerHandler
{
public:
	LocalM3u8Mgr(void);
	~LocalM3u8Mgr(void);
public:
	int init();
	void fini();

	virtual void on_timer(int e);
	void on_tracker_connected();
private:
	list<LocalM3u8*> m_lms;
};
typedef Singleton<LocalM3u8Mgr> LocalM3u8MgrSngl;

