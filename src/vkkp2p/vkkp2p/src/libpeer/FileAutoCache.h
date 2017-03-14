#pragma once
#include "commons.h"


class FileAutoCache
{
public:
	FileAutoCache(void);
	~FileAutoCache(void);

	typedef struct tagAutoInfo
{
	string path; //用于实时获取文件
	unsigned int score;
#ifdef SM_VOD
	int filetype;
#endif
}AutoInfo;

	typedef map<hash_t,AutoInfo*> AutoMap;
	typedef AutoMap::iterator AutoMapIter;
#ifdef SM_VOD
	typedef map<hash_t,AutoMap*> AutoMapvod;
	typedef AutoMapvod::iterator AutoMapItervod;
#endif

public:
	int init();
	void fini();

	int load_autoinfo();
	int save_autoinfo();
	void clear();

#ifdef SM_VOD
	int add_autoinfo(const hash_t& hash,int filetype,const string& path);
	void remove_autoinfo(const hash_t& hash, int filetype);
	void on_read_refer(const hash_t& hash, int filetype);
	bool get_autocache_vodmap(AutoMapvod& mp) const { mp = m_autoinfo_vod; return true;}
#endif
	int add_autoinfo(const hash_t& hash,const string& path);
	void remove_autoinfo(const hash_t& hash);
	void on_read_refer(const hash_t& hash);

	size64_t get_autocache_size();
	bool get_autocache_map(AutoMap& mp) const { mp = m_autoinfo; return true;}
	int auto_clear_cache(const hash_t& hashExclude); //return 0,可以继续写,1:不可以继续写
#ifdef SM_VOD
private:
	int delete_dlfile(const hash_t& hash, uint64 &rmsize); 
	int delete_url2file(const hash_t& excludehash,int filenum,uint64 &rmsize);
#endif

private:
	bool			m_binit;
	AutoMap			m_autoinfo;
	unsigned int	m_autoinfo_score;
	string			m_autoinfo_path;
#ifdef SM_VOD
	AutoMapvod			m_autoinfo_vod; /* only for vod */
	list<hash_t> m_prgvod; /* list record programe, delte vod file, will delete all file in one programe */
	bool m_autoclear;
#endif
};
typedef Singleton<FileAutoCache> FileAutoCacheSngl;
