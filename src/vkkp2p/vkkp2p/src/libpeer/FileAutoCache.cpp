#include "FileAutoCache.h"
#include "Setting.h"
#include "Util.h"
#include "FileStorage.h"
#include "DownloadManager.h"

#ifdef SM_DBG
#define FILEAUTOCACHE_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "FileAutoCache",__LINE__, ##arg)
#else
#define FILEAUTOCACHE_PRT(fmt, arg...)
#endif



FileAutoCache::FileAutoCache(void)
: m_binit(false)
, m_autoinfo_score(0)
#ifdef SM_VOD
,m_autoclear(false)
#endif
{
}

FileAutoCache::~FileAutoCache(void)
{
}

int FileAutoCache::init()
{
	if(m_binit)
		return 0;
	m_binit = true;
	m_autoinfo_path = SettingSngl::instance()->get_data_path() + "autoinfo.dat";
	sleep(5);
	load_autoinfo();
	save_autoinfo();
	return 0;
}
void FileAutoCache::fini()
{
	if(!m_binit)
		return;
	m_binit = false;
	load_autoinfo();
	save_autoinfo();
	clear();
}

int FileAutoCache::load_autoinfo()
{
	if(SettingSngl::instance()->get_is_minor_version())
		return -1;
	FILE *fp = fopen(m_autoinfo_path.c_str(),"rb");
	if(NULL == fp)
		return -1;
	uchar buf[64];
	char pathbuf[256];
	memset(buf,0,64);
	fread(buf,4,1,fp);
	if(0!=memcmp(buf,"ATIF",4))
	{
		fclose(fp);
		return -1;
	}
	//只支持版本1
	int ver = 0;
	fread(&ver,4,1,fp);
	if(1!=ver)
	{
		fclose(fp);
		return -1;
	}
	hash_t hash;
	AutoInfo *inf;
	unsigned int score = 0;
#ifdef SM_VOD
	int filetype = -1;
#endif
	unsigned int min_score = 10000;
	unsigned int max_score = 0;
	while(!feof(fp))
	{
		if(1!=fread(buf,HASHLEN,1,fp)) 
			break;
		hash.set_buffer(buf);
		fread(&score,4,1,fp);
#ifdef SM_VOD
		fread(&filetype,4,1,fp);
#endif
		memset(pathbuf,0,256);
		fread(pathbuf,256,1,fp);

		if(!FileStorageSngl::instance()->get_fileinfo(hash))
			continue;
		if (min_score > score)
			min_score = score;
		if (max_score < score)
			max_score =score;
		inf = new AutoInfo();
		if(inf)
		{
			inf->path = pathbuf;
			inf->score = score;
#ifdef SM_VOD
			inf->filetype = filetype;
			if(PLAYTYPE_VOD==filetype) {
				char bufurl2[45];
				hash_t hashdl;
				hash.to_string(bufurl2,45); 
				hashdl.set_urldl_string_hash(bufurl2+2);
				
				AutoMapItervod itvod=m_autoinfo_vod.find(hashdl);
				if(itvod!=m_autoinfo_vod.end()) {
					assert(itvod->second->find(hash)== itvod->second->end());
					(*(itvod->second))[hash] = inf;	//new a vod file
					FILEAUTOCACHE_PRT("load one vod..\n");
				} else {	
					AutoMap* mapvod = new AutoMap;
					(*mapvod)[hash] = inf;
					m_autoinfo_vod[hashdl] = mapvod; //new a programe and vod file
					m_prgvod.push_back(hashdl);
					FILEAUTOCACHE_PRT("load one programe....%s................!!!!\n", bufurl2);
				} 
			} else {
				m_autoinfo[hash] = inf;
			}
			
#else
			m_autoinfo[hash] = inf;
#endif /* end of SM_VOD */
		}
	}
	fclose(fp);
	if(10000 == min_score)
		min_score = 0;
	if(min_score>0)
	{
#ifdef SM_VOD
	/* because live will not save cache, will not load autocache, so this will not load in live mode */
	for(AutoMapItervod itvod=m_autoinfo_vod.begin();itvod!=m_autoinfo_vod.end();++itvod) 
		for(AutoMapIter itmap=itvod->second->begin();itmap!=itvod->second->end();++itmap) 
			itmap->second->score-=min_score;
	
	for(AutoMapIter it=m_autoinfo.begin();it!=m_autoinfo.end();++it)
			it->second->score -= min_score;
		max_score -= min_score;
#else
		for(AutoMapIter it=m_autoinfo.begin();it!=m_autoinfo.end();++it)
			it->second->score -= min_score;
		max_score -= min_score;
#endif /* end of SM_VOD */
	}

	FILEAUTOCACHE_PRT("listsize=%d\n",m_prgvod.size());
	m_autoinfo_score = max_score;
	return 0;
}
int FileAutoCache::save_autoinfo()
{
	if(SettingSngl::instance()->get_is_minor_version())
		return -1;
	FILE *fp = fopen(m_autoinfo_path.c_str(),"wb+");
	if(NULL == fp)
		return -1;
	list<string> ls;
	char buf[256];
	char tth[45];
	memset(tth,0,45);
	fwrite("ATIF",4,1,fp);
	int ver = 1;
	fwrite(&ver,4,1,fp);

#ifdef SM_VOD
	/* save live formation */
	for(AutoMapIter it=m_autoinfo.begin();it!=m_autoinfo.end();++it) {
		fwrite(it->first.buffer(),HASHLEN,1,fp);
		fwrite(&it->second->score,4,1,fp);
		fwrite(&it->second->filetype,4,1,fp);
		strcpy(buf,it->second->path.c_str());
		fwrite(buf,256,1,fp);
		it->first.to_string(tth,45);
		sprintf(buf,"%s|%s|%d|%d",tth,it->second->path.c_str(),it->second->score,it->second->filetype);
		ls.push_back(buf);
	}
	
	/* save vod information */
	for(AutoMapItervod itvod=m_autoinfo_vod.begin();itvod!=m_autoinfo_vod.end();++itvod) {
		for(AutoMapIter itmap=itvod->second->begin();itmap!=itvod->second->end();++itmap) {
			fwrite(itmap->first.buffer(),HASHLEN,1,fp);
			fwrite(&itmap->second->score,4,1,fp);
			fwrite(&itmap->second->filetype,4,1,fp);
			strcpy(buf,itmap->second->path.c_str());
			fwrite(buf,256,1,fp);

			itmap->first.to_string(tth,45);
			sprintf(buf,"%s|%s|%d|%d",tth,itmap->second->path.c_str(),itmap->second->score,itmap->second->filetype);
			ls.push_back(buf);
		}	
	}
#else
	for(AutoMapIter it=m_autoinfo.begin();it!=m_autoinfo.end();++it)
	{
		fwrite(it->first.buffer(),HASHLEN,1,fp);
		fwrite(&it->second->score,4,1,fp);
		strcpy(buf,it->second->path.c_str());
		fwrite(buf,256,1,fp);

		it->first.to_string(tth,45);
		sprintf(buf,"%s|%s|%d",tth,it->second->path.c_str(),it->second->score);
		ls.push_back(buf);
	}
#endif /* end of SM_VOD*/
	fclose(fp);
	//同时保存一份简单文本供阅读
	Util::put_stringlist_to_file(m_autoinfo_path+".txt",ls);
	return 0;
}
void FileAutoCache::clear()
{
	for(AutoMapIter it2=m_autoinfo.begin();it2!=m_autoinfo.end();++it2)
		delete it2->second;
	m_autoinfo.clear();

	/* clear vod info */
#ifdef SM_VOD
	AutoMapItervod itvod=m_autoinfo_vod.begin();
	while(itvod!=m_autoinfo_vod.end()) {
		AutoMapIter itmap=itvod->second->begin();
		while(itmap!=itvod->second->end()) {
			delete itmap->second; 
			itmap=itvod->second->begin();
		}
		itvod->second->clear();
		delete itvod->second;

		itvod=m_autoinfo_vod.begin();
	}
	m_autoinfo_vod.clear();
#endif
}

#ifdef SM_VOD
int FileAutoCache::add_autoinfo(const hash_t& hash,int filetype,const string& path)
{
	assert(HT_URL2==hash.hash_type());
	if(PLAYTYPE_VOD!=filetype) {
		add_autoinfo(hash, path);
	} else {
		hash_t hashdl;
		hash.url2hash_to_urldlhash(hashdl);

		/* first find in programe */
		AutoMapItervod itvod = m_autoinfo_vod.find(hashdl);
		if(itvod!=m_autoinfo_vod.end()) {
			AutoMapIter itmap = itvod->second->find(hash);
			if(itmap!= itvod->second->end())  {
				itmap->second->path = path; //upddate path
			} else {
				AutoInfo *inf = new AutoInfo();
				inf->path = path;
				inf->score = m_autoinfo_score;
				inf->filetype = filetype;
				(*itvod->second)[hash] = inf; //new a vod file
			}	
		} else {	
			AutoMap* mapvod = new AutoMap;
			AutoInfo *inf = new AutoInfo();
			inf->path = path;
			inf->score = m_autoinfo_score;
			inf->filetype = filetype;
			(*mapvod)[hash] = inf;
			m_autoinfo_vod[hashdl] = mapvod; //new a programe and vod file
			m_prgvod.push_back(hashdl);
		}
	}

	save_autoinfo();
	
	return 0;
}

void FileAutoCache::remove_autoinfo(const hash_t& hash, int filetype)
{
	assert(HT_URL2==hash.hash_type());

	/* remove */
	if(PLAYTYPE_VOD!=filetype)  {
		remove_autoinfo(hash);
	} else {
		hash_t hashdl;
		hash.url2hash_to_urldlhash(hashdl);

		/* find the programe */
		AutoMapItervod itvod = m_autoinfo_vod.find(hashdl);
		if(itvod!=m_autoinfo_vod.end()){
			AutoMapIter itmap = itvod->second->find(hash);
			/* find file */
			if(itmap!=itvod->second->end()) {
				FILEAUTOCACHE_PRT("size=%d\n",itvod->second->size());
				delete itmap->second;
				itvod->second->erase(itmap);
				if(!itvod->second->size()) {
					itvod->second->clear();
					delete itvod->second; 
					m_autoinfo_vod.erase(itvod);
					save_autoinfo();
				}	
			} 
		}
	}
}

void FileAutoCache::on_read_refer(const hash_t& hash, int filetype)
{
	if(PLAYTYPE_VOD!=filetype) {
		on_read_refer(hash);
	} else {
		hash_t hashdl;
		hash.url2hash_to_urldlhash(hashdl);

		/* find the programe */
		AutoMapItervod itvod = m_autoinfo_vod.find(hashdl);
		if(itvod!=m_autoinfo_vod.end()){
			AutoMapIter itmap = itvod->second->find(hash);
			/* find file */
			if(itmap!=itvod->second->end()){
				itmap->second->score=++m_autoinfo_score;
			} 
		}
	}
}

int FileAutoCache::delete_dlfile(const hash_t& hash, uint64 &rmsize)
{
	assert(HT_URLDL==hash.hash_type());
	hash_t hashdl;
	FileInfo *fi=NULL;

	m_autoclear = true;

	/* find the programe */
	AutoMapItervod itvod = m_autoinfo_vod.find(hash);
	if(itvod!=m_autoinfo_vod.end()) {
		AutoMapIter itmap = itvod->second->begin();
		bool b_last = false;
		while(1) {
			fi = FileStorageSngl::instance()->get_fileinfo(itmap->first);
			if(!fi || fi->ref) {
				FILEAUTOCACHE_PRT("----ref-----%d\n",fi->ref);
				FileStorageSngl::instance()->read_release(itmap->first); //somefile may not readrelease ok, again
				continue;
			}
			rmsize += File64::get_file_size(fi->path.c_str());
			char bufurl2[45];
			itmap->first.to_string(bufurl2,45); 
			FILEAUTOCACHE_PRT("delete file.............size=%d.........type=%d..........bufurl=%s.........\n", itvod->second->size(),itmap->first.hash_type(), bufurl2);
			DownloadManagerSngl::instance()->delete_file(itmap->first,true,true);
			if(b_last==false) {
				if(itvod->second->size()==1) {
					FILEAUTOCACHE_PRT("last file----------!\n");
					b_last = true;
				}
				itmap = itvod->second->begin(); 
			} else {
				FILEAUTOCACHE_PRT("delete all file successfully!\n");
				m_autoclear = false;
				return 0;
			}
		}
	} else {
		m_autoclear = false;
		return 1;
	}
	return 0;
}

int FileAutoCache::delete_url2file(const hash_t& excludehash,int filenum,uint64 &rmsize) {
	hash_t hashculdl;
	excludehash.url2hash_to_urldlhash(hashculdl);
	AutoMapIter it;
	hash_t hash;
	FileInfo *fi=NULL;
	int filecnt = 0;
	m_autoclear = true;

	unsigned int minScore = 10000;
	int isFind = false;
	AutoMapIter min_it;
	
	it=m_autoinfo_vod[hashculdl]->begin();
	while(filecnt < filenum) {
		if(it==m_autoinfo_vod[hashculdl]->end())
			break;
		
		minScore = 10000;
		
		FILEAUTOCACHE_PRT("listsize=%d\n",m_autoinfo_vod[hashculdl]->size());
		while(it!=m_autoinfo_vod[hashculdl]->end()) {
			/* skip the current file */
			if(it->first == excludehash) {
				it++;
				continue;
			}
			/* find the minum score */
			if(minScore > it->second->score) {
				minScore = it->second->score;
				min_it = it;
				hash = min_it->first;
				fi = FileStorageSngl::instance()->get_fileinfo(hash);
				if(fi && (!fi->ref))  {
					isFind = true;
				}
			}
			it++;
		}
		isFind = isFind;
		assert(isFind==true);
		FILEAUTOCACHE_PRT("will delete %s\n", fi->path.c_str());
		rmsize += File64::get_file_size(fi->path.c_str()); 
		DownloadManagerSngl::instance()->delete_file(hash,true, true);
		filecnt++;
		it=m_autoinfo_vod[hashculdl]->begin();
	}

	
	/*it=m_autoinfo_vod[hashculdl]->begin();
	while(filecnt < filenum) {
		if(it==m_autoinfo_vod[hashculdl]->end())
			break;
		if(excludehash==it->first) {
			it++;
			continue;
		}
		hash = it->first;
		m_autoinfo_vod[hashculdl]->erase(it);
		fi = FileStorageSngl::instance()->get_fileinfo(hash);
		if(!fi || fi->ref)  {
			it++;
			continue;
		}
		FILEAUTOCACHE_PRT("XXXXXXXXXXXXXXXXXX will delete %s\n", fi->path.c_str());
		rmsize += File64::get_file_size(fi->path.c_str()); 
		DownloadManagerSngl::instance()->delete_file(hash,true, true);
		filecnt++;
		it=m_autoinfo_vod[hashculdl]->begin();
	}*/
	m_autoclear = false;
	return 0;
}


#endif /* end of SM_VOD */

int FileAutoCache::add_autoinfo(const hash_t& hash,const string& path)
{
#ifdef SM_VOD
	if(0==SettingSngl::instance()->get_cache_flag())
		return 0;
#endif /* end of SM_VOD */
	//如果存在，替换路径
	AutoMapIter it = m_autoinfo.find(hash);
	if(it!=m_autoinfo.end())
	{
		it->second->path = path;
		save_autoinfo();
		return 0;
	}
	AutoInfo *inf = new AutoInfo();
	inf->path = path;
	inf->score = m_autoinfo_score;//ready refer的时候递增
	m_autoinfo[hash] = inf; 
	save_autoinfo();
	return 0;
}
void FileAutoCache::remove_autoinfo(const hash_t& hash)
{
#ifdef SM_VOD
	if(0==SettingSngl::instance()->get_cache_flag())
		return;
#endif /* end of SM_VOD */

	AutoMapIter it = m_autoinfo.find(hash);
	if(it!=m_autoinfo.end())
	{
		delete it->second;
		m_autoinfo.erase(it);
		save_autoinfo();
	}
}
void FileAutoCache::on_read_refer(const hash_t& hash)
{
#ifdef SM_VOD
	if(0==SettingSngl::instance()->get_cache_flag())
		return;
#endif /* end of SM_VOD */

	AutoMapIter it=m_autoinfo.find(hash);
	if(it!=m_autoinfo.end())
		it->second->score = ++m_autoinfo_score;
	save_autoinfo();
}

size64_t FileAutoCache::get_autocache_size()
{
	size64_t size = 0,itmp=0;
	for(AutoMapIter it=m_autoinfo.begin();it!=m_autoinfo.end(); ++it)
	{
		itmp = File64::get_file_size(it->second->path.c_str());
		//只cache内存的时候不会有大小
		if(itmp>0) 
			size += itmp;
	}
	
#ifdef SM_VOD
	for(AutoMapItervod itvod=m_autoinfo_vod.begin();itvod!=m_autoinfo_vod.end(); ++itvod) {
		for(AutoMapIter itmap=itvod->second->begin();itmap!=itvod->second->end(); ++itmap) {
			itmp = File64::get_file_size(itmap->second->path.c_str());
			if(itmp>0) 
				size += itmp;
		}
	}
#endif /* end of SM_VOD */
	return size;
}
int FileAutoCache::auto_clear_cache(const hash_t& hashExclude)
{
#ifdef SM_VOD
	if(0==SettingSngl::instance()->get_cache_flag())
		return 0;
#endif /* end of SM_VOD */
		
	/* if not clear ok, will not clear again */
	if(true==m_autoclear) 
		return 0;
	
	ULONGLONG total=0,used=0,free=0,minFree=0;
	Util::get_volume_size(SettingSngl::instance()->get_cache_path(),total,used,free);
	minFree = SettingSngl::instance()->get_disk_min_free_spaceMB();
	minFree = minFree << 20; //默认最少保证该盘1G的空间
	uint64 cacheSize = get_autocache_size();
	uint64 maxSizeinMB = SettingSngl::instance()->get_cache_sizeMB();
	uint64 maxSize = maxSizeinMB << 20;
	uint64 sizetmp=0, sizetmp2=0;
	int ret = 0;

	FILEAUTOCACHE_PRT("cache file size=%d, maxsize=%d\n", (int)cacheSize>>20, (int)maxSizeinMB);
	if(cacheSize  > maxSize || (cacheSize>0 && total!=0 && free<minFree))
	{
#ifdef SM_VOD
		hash_t hashculdl, hashpopdl;
		hashExclude.url2hash_to_urldlhash(hashculdl);

		/* will leave 1/16 free, so will  delete (1/16) */
		sizetmp = maxSize>>4;
		while((cacheSize + sizetmp) > maxSize || (total!=0 && free<minFree)) {
			FILEAUTOCACHE_PRT("will listsize=%d.. delete size:%d\n", m_prgvod.size(), (int)sizetmp2 >> 20);
			hashpopdl = m_prgvod.front();
			if((hashculdl==hashpopdl) && (1==m_prgvod.size())) {
				/* only have one programe, not so many disk space, will not  delet old file current file*/
				FILEAUTOCACHE_PRT("---------current have one program, may be will delete file in current programe\n");
				delete_url2file(hashExclude, 4, sizetmp2);
				cacheSize -= sizetmp2;
				free+=sizetmp2;
			} else {
				if(hashculdl==hashpopdl) {
					FILEAUTOCACHE_PRT("------current programe is in using, find next\n");
					m_prgvod.pop_front();
					m_prgvod.push_back(hashpopdl);
					continue;
				} else if(1==m_prgvod.size()){
					FILEAUTOCACHE_PRT("------this should never enter, if enter, please check\n");
					return 1;
				}
				FILEAUTOCACHE_PRT("will delete %d.. type=%d\n", m_prgvod.size(), hashpopdl.hash_type());
				ret = delete_dlfile(hashpopdl, sizetmp2);
				m_prgvod.pop_front();
				cacheSize -= sizetmp2;
				free += sizetmp2;
			}
		}	
		ret = 0; 
#else
		ret = 1;  //不可以继续写
		AutoMap mp;
		AutoMapIter it,min_it;
		get_autocache_map(mp);

		unsigned int minScore = 10000;
		bool isFind = false;
		hash_t hash;
		FileInfo *fi=NULL;
		////每次删除留400M空间或者1/8 中的大者
		//sizetmp = maxSize/8;  //
		//if( sizetmp< (400<<20) ) 
		sizetmp = 400<<20;
		while((cacheSize + sizetmp) > maxSize || (total!=0 && free<minFree))
		{
			isFind = false;
			minScore = 10000;
			for(it=mp.begin();it!=mp.end();++it)
			{
				if(it->first == hashExclude)
					continue;
				if(minScore>it->second->score)
				{
					minScore = it->second->score;
					min_it = it;
					isFind = true;
				}
			}
			if(!isFind)
				break;
			hash = min_it->first;
			mp.erase(min_it);
			fi = FileStorageSngl::instance()->get_fileinfo(hash);
			if(!fi || fi->ref)
				continue;
			sizetmp2 = File64::get_file_size(fi->path.c_str());
			cacheSize -= sizetmp2;
			DownloadManagerSngl::instance()->delete_file(hash,true);
			//异步删除不能实时取
			//Util::get_volume_size(SettingSngl::instance()->get_cache_path(),total,used,free);
			if(total!=0) free+=sizetmp2;
			ret = 0;  //可以继续写
		}
#endif /* end of SM_VOD */
	}
	return ret;
}

