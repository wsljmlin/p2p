#pragma once
#include "User.h"
#include "SourceService.h"

class AutoFileManager
{
	friend class Singleton<AutoFileManager>;
private:
	AutoFileManager(void);
	~AutoFileManager(void);
	
	typedef list<UserInfo*> UserList;
	typedef UserList::iterator UserIter;
	typedef list<SourceInfo*> SourceList;
	typedef SourceList::iterator SourceIter;
	typedef map<hash_t,string> VipfileList;
	typedef VipfileList::iterator VipfileIter;
public:
	int init();
	void fini();
	
	int add_super(UserInfo* user);
	int del_super(UserInfo* user);
	int add_vipsource(SourceInfo* node);
	int del_vipsource(SourceInfo* node);
	int on_vipfile_start(PTL_P2T_ReportStartDownloadList& inf);
	int on_vipfile_stop(const hash_t& hash);

private:
	int handle_super_source(hash_t& hash,const string& url);
private:
	bool m_binit;
	UserList m_supers;
	SourceList m_vipsources;
	VipfileList m_vipfiles;
	int m_vipsuper_minnum;
};



typedef Singleton<AutoFileManager> AutoFileManagerSngl;

