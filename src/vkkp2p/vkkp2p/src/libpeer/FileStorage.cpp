#include "FileStorage.h"
#include "Setting.h"
#include "Util.h"
#include "Tracker.h"
#include "FileAutoCache.h"
#include "Timer.h"

#ifdef SM_DBG
#define FILESTORAGE_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "FileStorage",__LINE__, ##arg)
#else
#define FILESTORAGE_PRT(fmt, arg...)
#endif


FileStorage::FileStorage(void)
: m_binit(false)
//, m_autoinfo_score(0)
,m_readyinfo_mtime(0)
{
}

FileStorage::~FileStorage(void)
{
}
int FileStorage::init()
{
	if(m_binit)
		return 0;
	m_binit = true;
	m_downinfo_dir = SettingSngl::instance()->get_data_path() + "downinfo/";
	m_readyinfo_dir = SettingSngl::instance()->get_data_path() + "readyinfo/";
	m_downinfo_path = SettingSngl::instance()->get_data_path() + "downinfo.dat";
	m_readyinfo_path = SettingSngl::instance()->get_data_path() + "readyinfo.inf";
	Util::my_create_directory(m_downinfo_dir);
	//Util::my_create_directory(m_readyinfo_dir);
	load_downinfo();
	load_readyinfo();

	time_t ct=0,mt=0,at=0;
	Util::get_filetimes(m_readyinfo_path.c_str(),ct,mt,at);
	m_readyinfo_mtime = (int)mt;

	save_main_downinfo();
	save_readyinfo();
	if(SettingSngl::instance()->get_is_minor_version())
		TimerSngl::instance()->register_timer(this,1,3000);
	return 0;
}
void FileStorage::fini()
{
	if(!m_binit)
		return;
	m_binit = false;
	TimerSngl::instance()->unregister_all(this);
	save_downinfo();
	clear();
}
void FileStorage::on_timer(int e)
{
	switch(e)
	{
	case 1:
		check_reload_readyinfo();
		break;
	default:
		assert(false);
		break;
	}
}
int FileStorage::load_downinfo()
{
	//次版本不装载下载信息
	if(SettingSngl::instance()->get_is_minor_version())
		return 0;

	list<string> ls;
	if(!Util::get_stringlist_from_file(m_downinfo_path,ls))
		return -1;

	string str;
	for(list<string>::iterator it=ls.begin();it!=ls.end(); ++it)
	{
		str = *it;
		if(str.empty())
			continue;
		FileInfo *fi = new FileInfo();
		fi->info_path = str;
		if(0==this->read_downinfo_i(str,*fi))
		{
#ifdef SM_VOD
			/* will delete all uncompleted downloaded file */
			if(!SettingSngl::instance()->get_is_clear_url2_cache())
#else
			if(HT_URL2==fi->hash.hash_type() && SettingSngl::instance()->get_is_clear_url2_cache())
#endif /* end of SM_VOD */
			{
				File64::remove_file(fi->info_path.c_str());
				fi->close_file();
				File64::remove_file(fi->path.c_str());
				delete fi;
			}
			else
#ifdef SM_DBG
			{
				FILESTORAGE_PRT("load file %s successfully!\n", str.c_str());
#else
#endif
				this->m_downinfo[fi->hash] = fi;
#ifdef SM_DBG
			}
					
#else
#endif

		}
		else
		{
			//将信息文件也一齐删除
			File64::remove_file(fi->info_path.c_str());
			delete fi;
		}
	}
	return 0;
}
int FileStorage::load_readyinfo()
{
	list<string> ls;
	if(!Util::get_stringlist_from_file(m_readyinfo_path,ls))
		return -1;

	string str;
	for(list<string>::iterator it=ls.begin();it!=ls.end(); ++it)
	{
		str = *it;
		if(str.empty())
			continue;
		FileInfo *fi = new FileInfo();
		//tth|size|path|ftype|block_size|ctime|mtime
		fi->tth = Util::get_string_index(str,0,"|");
		fi->hash.set_string_hash(fi->tth.c_str());
		fi->size = Util::atoll(Util::get_string_index(str,1,"|").c_str());
		fi->path = Util::get_string_index(str,2,"|");
		fi->ftype = atoi(Util::get_string_index(str,3,"|").c_str());
		//fi->block_size = atoi(Util::get_string_index(str,4,"|").c_str());
		//if(fi->block_size<=0)
			fi->block_size = SettingSngl::instance()->get_block_size();
		fi->ctime = atoi(Util::get_string_index(str,5,"|").c_str());
		fi->mtime= atoi(Util::get_string_index(str,6,"|").c_str());
#ifdef SM_VOD
		/* get filetype */
		fi->filetype = atoi(Util::get_string_index(str,7,"|").c_str());
#endif /* end of SM_VOD */
		fi->flag = 0;

#ifdef SM_VOD
		if(PLAYTYPE_VOD!=fi->filetype) {
			FILESTORAGE_PRT("live type will not load old video........!\n");
			File64::remove_file(fi->path.c_str());
			delete fi;
			continue;
		}
#endif

		//check:
		if(SettingSngl::instance()->get_is_minor_version())
		{
			//次版本不检查重复性与存在性
			this->m_readyinfo[fi->hash] = fi;
		}
		else
		{
			if(HT_URL2==fi->hash.hash_type() && SettingSngl::instance()->get_is_clear_url2_cache())
			{
				FILESTORAGE_PRT("url2 type and clearurl2 will delte file %s\n", str.c_str());
				File64::remove_file(fi->path.c_str());
				delete fi;
				continue;
			}
			if(m_readyinfo.find(fi->hash)!=m_readyinfo.end())
			{
				delete fi;
				continue;
			}
			
			time_t ct=0,mt=0,at=0;
			Util::get_filetimes(fi->path.c_str(),ct,mt,at);
			int t = (int)mt;
			size64_t size = ERDBFile64::get_filesize(fi->path.c_str());
			if(t>0 && size>0)
			{
				//
				if(/*t!=fi->mtime || */size != fi->size)
				{
					//文件有改变，需要重新共享
					DEBUGMSG("#*** reshare file : %s \n",fi->path.c_str());
					//g_sharelist.push_back(fi->path);
					delete fi;
					continue;
				}
			}
			if(fi->open_file(F64_READ))
			{
				fi->close_file();//ready动态打开读
				this->m_readyinfo[fi->hash] = fi;
				FILESTORAGE_PRT("load ready file %s successfully!\n", str.c_str());
			}
			else
			{
				DEBUGMSG("#*** file open failed : %s \n",fi->path.c_str());
				delete fi;
			}
		}
		DEBUGMSG("\r load read size = %d\n",m_readyinfo.size());
	}
	DEBUGMSG("\n#FileStorage::load_readyinfo(): ready num : %d \n",m_readyinfo.size());
	return 0;
}
void FileStorage::check_reload_readyinfo()
{
	//todo:重新load要考虑重新共享文件的问题
	time_t ct=0,mt=0,at=0;
	Util::get_filetimes(m_readyinfo_path.c_str(),ct,mt,at);
	int t = (int)mt;
	if(t!=m_readyinfo_mtime)
	{
		m_readyinfo_mtime = t;
		FileMapIter it;
		for(it=m_readyinfo.begin();it!=m_readyinfo.end();++it)
			delete it->second; //todo:这里删除要考虑ShareService正在分享问题
		m_readyinfo.clear();
		load_readyinfo();
	}
}
int FileStorage::save_downinfo()
{
	if(SettingSngl::instance()->get_is_minor_version())
		return 0;
	for(FileMapIter it=m_downinfo.begin();it!=m_downinfo.end();++it)
	{
		write_downinfo_i(*it->second);
	}
	save_main_downinfo();
	return 0;
}
int FileStorage::save_readyinfo()
{
	if(SettingSngl::instance()->get_is_minor_version())
		return 0;
	list<string> ls;
	ls.clear();
	char buf[2048];
	FileInfo *fi = NULL;
	//tth|size|path|ftype|block_size|ctime|mtime
	for(FileMapIter it=m_readyinfo.begin();it!=m_readyinfo.end();++it)
	{
		fi = it->second;
#ifdef SM_VOD
		sprintf(buf,"%s|%lld|%s|%d|%d|%d|%d|%d",fi->tth.c_str(),fi->size,fi->path.c_str(),
			fi->ftype,fi->block_size,fi->mtime,fi->ctime, fi->filetype);
#else
		sprintf(buf,"%s|%lld|%s|%d|%d|%d|%d",fi->tth.c_str(),fi->size,fi->path.c_str(),
			fi->ftype,fi->block_size,fi->mtime,fi->ctime);
#endif /* end of SM_VOD */
		ls.push_back(buf);
	}
	Util::put_stringlist_to_file(m_readyinfo_path+".tmp",ls);
	Util::file_delete(m_readyinfo_path);
	Util::file_rename(m_readyinfo_path+".tmp",m_readyinfo_path);
	return 0;
}

int FileStorage::save_main_downinfo()
{
	if(SettingSngl::instance()->get_is_minor_version())
		return 0;
	list<string> ls;
	for(FileMapIter it=m_downinfo.begin();it!=m_downinfo.end();++it)
		ls.push_back(it->second->info_path);
	Util::put_stringlist_to_file(m_downinfo_path,ls);
	return 0;
}
//int FileStorage::save_main_readyinfo()
//{
//	if(SettingSngl::instance()->get_is_minor_version())
//		return 0;
//	list<string> ls;
//	for(FileMapIter it=m_readyinfo.begin();it!=m_readyinfo.end();++it)
//		ls.push_back(it->second->info_path);
//	Util::put_stringlist_to_file(m_readyinfo_path,ls);
//
//	//同时保存一份简单文本供阅读
//	ls.clear();
//	char buf[2048];
//	FileInfo *fi = NULL;
//	//tth|size|path|block_size|ftype|mtime|ctime
//	for(FileMapIter it=m_readyinfo.begin();it!=m_readyinfo.end();++it)
//	{
//		fi = it->second;
//		sprintf(buf,"%s|%lld|%s|%d|%d|%d|%d",fi->tth.c_str(),fi->size,fi->path.c_str(),
//			fi->block_size,fi->ftype,fi->mtime,fi->ctime);
//		ls.push_back(buf);
//	}
//	Util::put_stringlist_to_file(m_readyinfo_path+".txt",ls);
//	return 0;
//}
void FileStorage::clear()
{
	FileMapIter it;
	for(it=m_downinfo.begin();it!=m_downinfo.end();++it)
		delete it->second;
	m_downinfo.clear();
	for(it=m_readyinfo.begin();it!=m_readyinfo.end();++it)
		delete it->second;
	m_readyinfo.clear();
}

int FileStorage::write_downinfo_i(const FileInfo& fi)
{
	if(fi.is_memcache_only())
		return 0;
	File32 file;
	file.open(fi.info_path.c_str(),F32_RDWR|F32_TRUNC|F32_BINARY);
	if(!file.is_open())
		return -1;

	//flag:.DIF; version 1: hash=HASHLEN ; tth=64; path=256 ; despath=256 ; ftype=4 ,size=8 ;
	//block_size=4 ;blocks=4;block_offset=4;ctime=4;mtime=4; bt_finished; bdt
	unsigned int ver = 1;
	char buf[260] = {0};

	file.write(".DIF",4);
	file.write((const char*)&ver,4);
	file.write((const char*)fi.hash.buffer(),HASHLEN);
	strcpy(buf,fi.tth.c_str());
	file.write(buf,64);
	strcpy(buf,fi.path.c_str());
	file.write(buf,256);
	strcpy(buf,fi.despath.c_str());
	file.write(buf,256);

	file.write((const char*)&fi.ftype,sizeof(fi.ftype));
	file.write((const char*)&fi.size,sizeof(fi.size));
	file.write((const char*)&fi.block_size,sizeof(fi.block_size));
	file.write((const char*)&fi.blocks,sizeof(fi.blocks));
	file.write((const char*)&fi.block_offset,sizeof(fi.block_offset));
	file.write((const char*)&fi.ctime,sizeof(fi.ctime));
	file.write((const char*)&fi.mtime,sizeof(fi.mtime));
	file.write((const char*)&fi.flag,sizeof(fi.flag));
	
	fi.bt_finished.write_file(file);
	fi.bdt.write_file(file);
	file.close();
	return 0;
}
int FileStorage::read_downinfo_i(const string& path,FileInfo& fi)
{
	File32 file;
	file.open(fi.info_path.c_str(),F32_READ|F32_BINARY);
	if(!file.is_open())
		return -1;
	//flag:.DIF; version 1: hash=HASHLEN ; tth=64; path=256 ; despath=256 ; ftype=4 ,size=8 ;
	//block_size=4 ;blocks=4;block_offset=4;ctime=4;mtime=4; bt_finished; bdt
	char buf[260];
	int ver = 0;
	file.read(buf,4);
	file.read((char*)&ver,4);
	if(0!=memcmp(buf,".DIF",4)||1!=ver)
	{
		file.close();
		return -1;
	}
	file.read(buf,HASHLEN);
	fi.hash.set_buffer((uchar*)buf);
	file.read(buf,64);
	fi.tth = buf;
	file.read(buf,256);
	fi.path = buf;
	file.read(buf,256);
	fi.despath = buf;

	file.read((char*)&fi.ftype,sizeof(fi.ftype));
	file.read((char*)&fi.size,sizeof(fi.size));
	file.read((char*)&fi.block_size,sizeof(fi.block_size));
	file.read((char*)&fi.blocks,sizeof(fi.blocks));
	file.read((char*)&fi.block_offset,sizeof(fi.block_offset));
	file.read((char*)&fi.ctime,sizeof(fi.ctime));
	file.read((char*)&fi.mtime,sizeof(fi.mtime));
	file.read((char*)&fi.flag,sizeof(fi.flag));
	
	fi.bt_finished.read_file(file);
	fi.bdt.read_file(file);
	fi.bt_memfinished = fi.bt_finished;
	file.close();

	//检查数据合法
	if(fi.block_size==0)
		return -1;
	unsigned int n = 0;
	if(fi.bt_finished.get_block_num()!=fi.bdt.get_block_num() || fi.bdt.get_block_num()!=fi.blocks)
		return -1;
	if(fi.bt_finished.get_block_offset()!=fi.bdt.get_block_offset() || fi.bdt.get_block_offset()!=fi.block_offset)
		return -1;

	n = fi.bt_finished.get_block_num();
	for(fi.block_gap=fi.bt_finished.get_block_offset();fi.block_gap<n && fi.bt_finished[fi.block_gap];++fi.block_gap,++fi.down_blocks); //计算缺口
	for(unsigned int i= fi.block_gap+1;i<n;++i)
	{
		if(fi.bt_finished[i])
		{
			++fi.down_blocks;
		}
	}

	//try open file
	if(fi.open_file(F64_RDWR))
	{
		//预打开
		return 0;
	}
	return -1;
}
//int FileStorage::write_readyinfo_i(const FileInfo& fi)
//{
//	File32 file;
//	file.open(fi.info_path.c_str(),F32_RDWR|F32_TRUNC|F32_BINARY);
//	if(!file.is_open())
//		return -1;
//
//	//flag:.RDI; version 1: hash=HASHLEN ; tth=64; path=256 ; despath=256 ; ftype=4 ,size=8 ;
//	//block_size=4 ;ctime=4;mtime=4; bt_finished; bdt
//	unsigned int ver = 1;
//	char buf[260] = {0};
//
//	file.write(".RDI",4);
//	file.write((const char*)&ver,4);
//	file.write((const char*)fi.hash.buffer(),HASHLEN);
//	strcpy(buf,fi.tth.c_str());
//	file.write(buf,64);
//	strcpy(buf,fi.path.c_str());
//	file.write(buf,256);
//
//	file.write((const char*)&fi.ftype,sizeof(fi.ftype));
//	file.write((const char*)&fi.size,sizeof(fi.size));
//	file.write((const char*)&fi.block_size,sizeof(fi.block_size));
//	file.write((const char*)&fi.ctime,sizeof(fi.ctime));
//	file.write((const char*)&fi.mtime,sizeof(fi.mtime));
//	file.write((const char*)&fi.flag,sizeof(fi.flag));
//	
//	file.close();
//	return 0;
//}
//
//int FileStorage::read_readyinfo_i(const string& path,FileInfo& fi)
//{
//	File32 file;
//	file.open(fi.info_path.c_str(),F32_READ|F32_BINARY);
//	if(!file.is_open())
//		return -1;
//	//flag:.RDI; version 1: hash=HASHLEN ; tth=64; path=256 ; despath=256 ; ftype=4 ,size=8 ;
//	//block_size=4 ;ctime=4;mtime=4; bt_finished; bdt
//	char buf[260];
//	int ver = 0;
//	file.read(buf,4);
//	file.read((char*)&ver,4);
//	if(0!=memcmp(buf,".RDI",4)||1!=ver)
//	{
//		file.close();
//		return -1;
//	}
//	file.read(buf,HASHLEN);
//	fi.hash.set_buffer((uchar*)buf);
//	file.read(buf,64);
//	fi.tth = buf;
//	file.read(buf,256);
//	fi.path = buf;
//
//	file.read((char*)&fi.ftype,sizeof(fi.ftype));
//	file.read((char*)&fi.size,sizeof(fi.size));
//	file.read((char*)&fi.block_size,sizeof(fi.block_size));
//	file.read((char*)&fi.ctime,sizeof(fi.ctime));
//	file.read((char*)&fi.mtime,sizeof(fi.mtime));
//	file.read((char*)&fi.flag,sizeof(fi.flag));
//
//	file.close();
//	return 0;
//}

#ifdef SM_VOD
FileInfo* FileStorage::create_downinfo(const hash_t& hash,int filetype, const string& path,size64_t size/*=0*/,size64_t offset/*=0*/
	,unsigned int block_size/*=DEFAULT_BLOCK_SIZE*/,int ftype/*=FTYPE_VOD*/,int rdbftype/*=RDBF_AUTO*/) {
	//次版本不创建下载任务
	if(SettingSngl::instance()->get_is_minor_version())
		return NULL;
	TLock<Mutex> l(m_mt);
	if(0==block_size)
		return NULL;
	if(m_downinfo.find(hash)!=m_downinfo.end())
		return NULL;

	Util::create_directory_by_filepath(path);
	FileInfo *fi = new FileInfo();
	if(!fi)
		return NULL;
	
	char strhash[48];
	fi->hash = hash;
	fi->hash.to_string(strhash,48);
	fi->tth = strhash;
	fi->despath = path;
	fi->path = fi->despath + ".vkkd!";
	fi->size = size;
	fi->block_size = block_size;
	fi->ftype = ftype;
	fi->block_offset = (unsigned int)(offset/block_size);
	fi->filetype = filetype;
	
	if(!fi->open_file(F64_RDWR|F64_TRUN,rdbftype))
	{
		bool bopen = false;
		if(FTYPE_VOD==ftype && !fi->is_memcache_only())
		{
			SettingSngl::instance()->set_memcache_only();
			if(fi->open_file(F64_RDWR|F64_TRUN,rdbftype))
				bopen = true;
		}
		if(!bopen)
		{
			delete fi;
			return NULL;
		}
	}
	fi->info_path = m_downinfo_dir + fi->tth.substr(2) + ".inf";
	m_downinfo[fi->hash] = fi;
	if(fi->size==UINT64_INFINITE)
	{
		//todo_hcl:
	}
	else if(fi->size > 0)
	{
		fi->blocks = (unsigned int)((fi->size - 1)/fi->block_size + 1);
		if(fi->block_offset > fi->blocks) 
			fi->block_offset = fi->blocks; //此情况根本不会下载
		fi->blocks -= fi->block_offset;
		fi->bt_finished.resize(fi->blocks,fi->block_offset,false);
		fi->bt_memfinished.resize(fi->blocks,fi->block_offset,false);
		fi->bdt.resize(fi->blocks,fi->block_offset,false);
	}
	if(!fi->is_memcache_only())
	{
		write_downinfo_i(*fi);
		save_main_downinfo();
	}
	if(FTYPE_VOD==fi->ftype && !fi->is_memcache_only())
		FileAutoCacheSngl::instance()->add_autoinfo(hash,fi->filetype,fi->path);
	//DEBUGMSG("#FileStorage::create_downinfo() OK \n");
	return fi;
}

FileInfo* FileStorage::create_readyinfo(const hash_t& hash,int filetype, const string& path,size64_t size,int ftype/*=FTYPE_SHAREONLY*/)
{
	TLock<Mutex> l(m_mt);
	if(0==size)
		size = ERDBFile64::get_filesize(path.c_str());
	if(size==0)
		return NULL;
	FileMapIter it;
	//检查删除下载列表中的节目
	it = m_downinfo.find(hash);
	if(it!=m_downinfo.end())
	{
		this->delete_downinfo(hash,true);
	}

	it = m_readyinfo.find(hash);
	if(it!=m_readyinfo.end())
	{
		//更新内容
		FileInfo *fi = it->second;
		if(fi->ftype != FTYPE_SHAREONLY)
			this->delete_readyinfo(hash,true);
		else if(fi->path!= path)
		{
			{
				time_t ct=0,mt=0,at=0;
				Util::get_filetimes(path.c_str(),ct,mt,at);
				fi->ctime = (int)ct;
				fi->mtime = (int)mt;
			}
			fi->path = path;
			fi->size = size;
			//write_readyinfo_i(*fi);
			save_readyinfo();
			return fi;
		}
	}
	FileInfo *fi = new FileInfo();
	if(!fi)
		return NULL;
	char strhash[48];
	fi->hash = hash;
	fi->hash.to_string(strhash,48);
	fi->tth = strhash;
	fi->path = path;
	fi->size = size;
	fi->ftype = ftype;
	fi->filetype = filetype;
	{
		time_t ct=0,mt=0,at=0;
		Util::get_filetimes(path.c_str(),ct,mt,at);
		fi->ctime = (int)ct;
		fi->mtime = (int)mt;
	}

	if(!fi->open_file(F64_READ))
	{
		delete fi;
		return NULL;
	}
	fi->close_file();
	fi->info_path = m_readyinfo_dir + fi->tth.substr(2) + ".inf";
	m_readyinfo[fi->hash] = fi;
	//write_readyinfo_i(*fi);
	//save_main_readyinfo();
	save_readyinfo();
	DEBUGMSG("#FileStorage::create_readyinfo() OK \n");
	return fi;
}

int FileStorage::delete_fileinfo_noerase(const hash_t& hash,bool delfile)
{
	if(0!=delete_downinfo_noerase(hash,delfile))
		delete_readyinfo_noerase(hash,delfile);
	return 0;
}

int FileStorage::delete_downinfo_noerase(const hash_t& hash,bool delfile)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it=m_downinfo.find(hash);
	if(it==m_downinfo.end())
		return -1;
	FileInfo *fi = it->second;
	fi->close_file();
	if(delfile)
	{
		File64::remove_file(fi->path.c_str());
	}
	save_main_downinfo();
	return 0;
}
int FileStorage::delete_readyinfo_noerase(const hash_t& hash,bool delfile)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it=m_readyinfo.find(hash);
	if(it==m_readyinfo.end())
		return -1;
	FileInfo *fi = it->second;
	fi->close_file();
	//File64::remove_file(fi->info_path.c_str());
	if(delfile)
	{
		File64::remove_file(fi->path.c_str());
	}
	//save_main_readyinfo();
	save_readyinfo();
	return 0;
}

#endif /* end of SM_VOD */

FileInfo* FileStorage::create_downinfo(const hash_t& hash,const string& path,size64_t size/*=0*/,size64_t offset/*=0*/
	,unsigned int block_size/*=DEFAULT_BLOCK_SIZE*/,int ftype/*=FTYPE_VOD*/,int rdbftype/*=RDBF_AUTO*/)
{
	//次版本不创建下载任务
	if(SettingSngl::instance()->get_is_minor_version())
		return NULL;
	TLock<Mutex> l(m_mt);
	if(0==block_size)
		return NULL;
	if(m_downinfo.find(hash)!=m_downinfo.end())
		return NULL;

	Util::create_directory_by_filepath(path);
	FileInfo *fi = new FileInfo();
	if(!fi)
		return NULL;
	
	char strhash[48];
	fi->hash = hash;
	fi->hash.to_string(strhash,48);
	fi->tth = strhash;
	fi->despath = path;
	fi->path = fi->despath + ".vkkd!";
	fi->size = size;
	fi->block_size = block_size;
	fi->ftype = ftype;
	fi->block_offset = (unsigned int)(offset/block_size);

	if(!fi->open_file(F64_RDWR|F64_TRUN,rdbftype))
	{
		bool bopen = false;
		if(FTYPE_VOD==ftype && !fi->is_memcache_only())
		{
			SettingSngl::instance()->set_memcache_only();
			if(fi->open_file(F64_RDWR|F64_TRUN,rdbftype))
				bopen = true;
		}
		if(!bopen)
		{
			delete fi;
			return NULL;
		}
	}
	fi->info_path = m_downinfo_dir + fi->tth.substr(2) + ".inf";
	m_downinfo[fi->hash] = fi;
	if(fi->size==UINT64_INFINITE)
	{
		//todo_hcl:
	}
	else if(fi->size > 0)
	{
		fi->blocks = (unsigned int)((fi->size - 1)/fi->block_size + 1);
		if(fi->block_offset > fi->blocks) 
			fi->block_offset = fi->blocks; //此情况根本不会下载
		fi->blocks -= fi->block_offset;
		fi->bt_finished.resize(fi->blocks,fi->block_offset,false);
		fi->bt_memfinished.resize(fi->blocks,fi->block_offset,false);
		fi->bdt.resize(fi->blocks,fi->block_offset,false);
	}
	if(!fi->is_memcache_only())
	{
		write_downinfo_i(*fi);
		save_main_downinfo();
	}
	if(FTYPE_VOD==fi->ftype && !fi->is_memcache_only())
		FileAutoCacheSngl::instance()->add_autoinfo(hash,fi->path);
	//DEBUGMSG("#FileStorage::create_downinfo() OK \n");
	return fi;
}
FileInfo* FileStorage::create_readyinfo(const hash_t& hash,const string& path,size64_t size,int ftype/*=FTYPE_SHAREONLY*/)
{
	TLock<Mutex> l(m_mt);
	if(0==size)
		size = ERDBFile64::get_filesize(path.c_str());
	if(size==0)
		return NULL;
	FileMapIter it;
	//检查删除下载列表中的节目
	it = m_downinfo.find(hash);
	if(it!=m_downinfo.end())
	{
		this->delete_downinfo(hash,true);
	}

	it = m_readyinfo.find(hash);
	if(it!=m_readyinfo.end())
	{
		//更新内容
		FileInfo *fi = it->second;
		if(fi->ftype != FTYPE_SHAREONLY)
			this->delete_readyinfo(hash,true);
		else if(fi->path!= path)
		{
			{
				time_t ct=0,mt=0,at=0;
				Util::get_filetimes(path.c_str(),ct,mt,at);
				fi->ctime = (int)ct;
				fi->mtime = (int)mt;
			}
			fi->path = path;
			fi->size = size;
			//write_readyinfo_i(*fi);
			save_readyinfo();
			return fi;
		}
	}
	FileInfo *fi = new FileInfo();
	if(!fi)
		return NULL;
	char strhash[48];
	fi->hash = hash;
	fi->hash.to_string(strhash,48);
	fi->tth = strhash;
	fi->path = path;
	fi->size = size;
	fi->ftype = ftype;
	{
		time_t ct=0,mt=0,at=0;
		Util::get_filetimes(path.c_str(),ct,mt,at);
		fi->ctime = (int)ct;
		fi->mtime = (int)mt;
	}

	if(!fi->open_file(F64_READ))
	{
		delete fi;
		return NULL;
	}
	fi->close_file();
	fi->info_path = m_readyinfo_dir + fi->tth.substr(2) + ".inf";
	m_readyinfo[fi->hash] = fi;
	//write_readyinfo_i(*fi);
	//save_main_readyinfo();
	save_readyinfo();
	DEBUGMSG("#FileStorage::create_readyinfo() OK \n");
	return fi;
}
int FileStorage::update_downinfo_size(const hash_t& hash,size64_t size)
{
	TLock<Mutex> l(m_mt);
	if(size==0)
		return -1;
	FileMapIter it = m_downinfo.find(hash);
	if(it == m_downinfo.end())
		return -1;
	if(it->second->size>0)
		return -1;
	FileInfo *fi = it->second;
	fi->size = size;
	
	if(fi->size==UINT64_INFINITE)
	{
		//todo_hcl:
	}
	else if(fi->size > 0)
	{
		fi->blocks = (unsigned int)((fi->size - 1)/fi->block_size + 1);
		if(fi->block_offset > fi->blocks) 
			fi->block_offset = fi->blocks; //此情况根本不会下载
		fi->blocks -= fi->block_offset;
		fi->bt_finished.resize(fi->blocks,fi->block_offset,false);
		fi->bt_memfinished.resize(fi->blocks,fi->block_offset,false);
		fi->bdt.resize(fi->blocks,fi->block_offset,false);
	}
	write_downinfo_i(*fi);
	return 0;
}
int FileStorage::update_downinfo_path(const hash_t& hash,const string& path)
{
	TLock<Mutex> l(m_mt);
	if(path.empty())
		return -1;
	FileMapIter it = m_downinfo.find(hash);
	if(it != m_downinfo.end())
	{
		FILESTORAGE_PRT("update path form %s to %s\n", it->second->despath.c_str(), path.c_str());
		it->second->despath = path;
		write_downinfo_i(*it->second);
		return 0;
	}
	else
	{
		it = m_readyinfo.find(hash);
		if(it==m_readyinfo.end())
			return -1;
		FileInfo *fi = it->second;
		//注意：目前如果只是纯共享文件，不要改变
		if(FTYPE_SHAREONLY==fi->ftype || fi->path == path)
			return 0;
		fi->close_file();
		if(0==File64::rename_file(fi->path.c_str(),path.c_str()))
		{
			fi->path = path;
			//write_readyinfo_i(*fi);
			save_readyinfo();
			return 0;
		}
		return -1;
	}
}
int FileStorage::update_downinfo_type(const hash_t& hash,int type)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it = m_downinfo.find(hash);
	if(it != m_downinfo.end())
	{
		if(type != it->second->ftype)
		{
			it->second->ftype = type;
			write_downinfo_i(*it->second);
#ifdef SM_VOD
			if(FTYPE_VOD==it->second->ftype ) 
				FileAutoCacheSngl::instance()->add_autoinfo(hash,it->second->filetype,it->second->path);
			 else 
				FileAutoCacheSngl::instance()->remove_autoinfo(hash, it->second->filetype);
#else
			if(FTYPE_VOD==it->second->ftype)
				FileAutoCacheSngl::instance()->add_autoinfo(hash,it->second->path);
			else
				FileAutoCacheSngl::instance()->remove_autoinfo(hash);
#endif /* end of SM_VOD */
		}
		return 0;
	}
	else
	{
		it = m_readyinfo.find(hash);
		if(it==m_readyinfo.end())
			return -1;
		FileInfo *fi = it->second;
		//注意：目前如果只是纯共享文件，不要改变
		if(FTYPE_SHAREONLY==fi->ftype || fi->ftype == type)
			return 0;
		fi->ftype = type;
		//write_readyinfo_i(*fi);
		save_readyinfo();
#ifdef SM_VOD
		if(FTYPE_VOD==fi->ftype)
			FileAutoCacheSngl::instance()->add_autoinfo(hash,fi->filetype,fi->path);
		else
			FileAutoCacheSngl::instance()->remove_autoinfo(hash,fi->filetype); 
#else
		if(FTYPE_VOD==fi->ftype)
			FileAutoCacheSngl::instance()->add_autoinfo(hash,fi->path);
		else
			FileAutoCacheSngl::instance()->remove_autoinfo(hash);
#endif /* end of SM_VOD */
		return 0;
	}
}

int FileStorage::delete_downinfo(const hash_t& hash,bool delfile)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it=m_downinfo.find(hash);
	if(it==m_downinfo.end())
		return -1;
#ifdef SM_VOD
	FileAutoCacheSngl::instance()->remove_autoinfo(hash, it->second->filetype);
#else
	FileAutoCacheSngl::instance()->remove_autoinfo(hash);
#endif /* end of SM_VOD */
	FileInfo *fi = it->second;
	fi->close_file();
	File64::remove_file(fi->info_path.c_str());
	if(delfile)
	{
		File64::remove_file(fi->path.c_str());
	}
	delete fi;
	m_downinfo.erase(it);
	save_main_downinfo();
	return 0;
}
int FileStorage::delete_readyinfo(const hash_t& hash,bool delfile)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it=m_readyinfo.find(hash);
	if(it==m_readyinfo.end())
		return -1;
#ifdef SM_VOD
	FileAutoCacheSngl::instance()->remove_autoinfo(hash, it->second->filetype);
#else
	FileAutoCacheSngl::instance()->remove_autoinfo(hash);
#endif /* end of SM_VOD */
	FileInfo *fi = it->second;
	fi->close_file();
	//File64::remove_file(fi->info_path.c_str());
	if(delfile)
	{
		File64::remove_file(fi->path.c_str());
	}
	delete fi;
	m_readyinfo.erase(it);
	//save_main_readyinfo();
	save_readyinfo();
	return 0;
}

int FileStorage::delete_fileinfo(const hash_t& hash,bool delfile)
{
	if(0!=delete_downinfo(hash,delfile))
		delete_readyinfo(hash,delfile);
	return 0;
}


FileInfo* FileStorage::get_downinfo(const hash_t& hash)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it=m_downinfo.find(hash);
	if(it!=m_downinfo.end())
		return it->second;
	return NULL;
}
FileInfo* FileStorage::get_readyinfo(const hash_t& hash)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it=m_readyinfo.find(hash);
	if(it!=m_readyinfo.end())
		return it->second;
	return NULL;
}
FileInfo* FileStorage::get_fileinfo(const hash_t& hash)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it=m_downinfo.find(hash);
	if(it!=m_downinfo.end())
		return it->second;
	it=m_readyinfo.find(hash);
	if(it!=m_readyinfo.end())
		return it->second;
	return NULL;
}
size64_t FileStorage::get_file_size(const hash_t& hash)
{
	TLock<Mutex> l(m_mt);
	FileInfo* fi = get_fileinfo(hash);
	if(fi)
		return fi->size;
	return 0;
}

bool FileStorage::check_exist_filepath(const string& path,bool check_state)
{
	TLock<Mutex> l(m_mt);
	FileInfo *fi=NULL;
	bool bready = false;
	FileMapIter it;
	for(it=m_readyinfo.begin();it!=m_readyinfo.end();++it)
	{
#ifdef _WIN32
		//忽略大小写比较：
		if(0==stricmp(it->second->path.c_str(),path.c_str()))
#else
		if(0==strcmp(it->second->path.c_str(),path.c_str()))
#endif
		{
			bready = true;
			fi = it->second;
			break;
		}
	}

	if(!fi)
	{
		for(it=m_downinfo.begin();it!=m_downinfo.end();++it)
		{
#ifdef _WIN32
			//忽略大小写比较：
			if(0==stricmp(it->second->path.c_str(),path.c_str()))
#else
			if(0==strcmp(it->second->path.c_str(),path.c_str()))
#endif
			{
				fi = it->second;
				break;
			}
		}
	}
	if(!fi)
		return false;
	if(bready && check_state)
	{
		time_t ct=0,mt=0,at=0;
		Util::get_filetimes(fi->path.c_str(),ct,mt,at);
		int t = (int)mt;
		size64_t size = ERDBFile64::get_filesize(fi->path.c_str());
		if((t>0&&t!=fi->mtime) || size!=fi->size)
		{
			hash_t hash = fi->hash;
			delete_readyinfo(hash,false);
			FILESTORAGE_PRT("remove file............!\n");
#ifdef SM_VOD
			TrackerSngl::instance()->PTL_ReportRemoveFile(hash, fi->filetype);
#else
			TrackerSngl::instance()->PTL_ReportRemoveFile(hash);
#endif /* end of SM_VOD */
			return false;
		}
	}
	return true;
}


FileBlock* FileStorage::get_fileblock(const hash_t& hash,unsigned int index,unsigned int index_pos,unsigned int len,unsigned int block_size)
{
	TLock<Mutex> l(m_mt);
	FileBlock *block = FileBlockPoolSngl::instance()->get_fileblock(hash,index,block_size);
	if(!block)
		return NULL;
	unsigned int begin = block->begin;
	unsigned int end = block->end;
	int iread = 0;
	if(begin>index_pos)
	{
		iread = begin-index_pos;
		if(iread==read_filedata(hash,block->buf+index_pos,iread,block_size*(size64_t)index + index_pos))
			block->begin = index_pos;
		else
		{
			FileBlockPoolSngl::instance()->put_fileblock(block);
			return NULL;
		}
	}
	if(end<(index_pos+len))
	{
		iread = index_pos+len-end;
		if(iread==read_filedata(hash,block->buf+end,iread,block_size*(size64_t)index + end))
			block->end = index_pos+len;
		else
		{
			FileBlockPoolSngl::instance()->put_fileblock(block);
			return NULL;
		}
	}
	return block;
}

//供shareservice使用，尝试从memcache中获取，memcache中没有的话内存获取，获取后不会放入memcache中
FileBlock* FileStorage::get_fileblock_trymemcache(const hash_t& hash,unsigned int index,unsigned int index_pos,unsigned int len,unsigned int block_size)
{
	TLock<Mutex> l(m_mt);FileInfo *fi = NULL;
	bool bReady = true;
	FileBlock *block = NULL;
	FileInfo::FileBlockMapIter fb_it;

	FileMapIter it = m_readyinfo.find(hash);
	if(it==m_readyinfo.end())
	{
		bReady = false;
		it=m_downinfo.find(hash);
		if(it==m_downinfo.end())
			return NULL;
	}
	fi = it->second;
	if(!bReady && !fi->bt_memfinished[index])
	{
		return NULL;
	}
	fb_it = fi->fbmp_mem.find(index);
	if(fb_it!=fi->fbmp_mem.end())
	{
		block = fb_it->second;
		//可能块数据不完整
		if(block->begin > index_pos || block->end<(index_pos+len))
		{
			FileBlock *block2 = get_fileblock(hash,index,index_pos,len,block_size);
			if(block2)
			{
				//因为get_fileblock()一定会获取到同一块memcache_block，所以这里获取一下就释放仅作为读取完整block
				assert(block2==block);
				put_fileblock(block2); 
			}
		}
		if(block->begin > index_pos || block->end<(index_pos+len))
			block = NULL;
		else
			block->ref++;
	}
	else
	{
		block = get_fileblock(hash,index,index_pos,len,block_size);
		//DEBUGMSG("#read file block trymemache(%d) \n",index);
	}
	return block;
}
void FileStorage::put_fileblock(FileBlock* b)
{
	TLock<Mutex> l(m_mt);
	FileBlockPoolSngl::instance()->put_fileblock(b);
}

int FileStorage::write_filedata(const hash_t& hash,const char *buf,unsigned int buflen,unsigned int index,unsigned int index_pos)
{
	TLock<Mutex> l(m_mt);
	FileInfo *fi = NULL;
	FileMapIter it = m_downinfo.find(hash);
	if(it == m_downinfo.end())
		return -1;
	fi = it->second;

	if(fi->bt_finished[index])
		return -1;
	BlockDataTable::BlockData_t& bd = fi->bdt[index];
	FileBlock *block = (FileBlock*)bd.data;
	if(!block)
	{
		block = FileBlockPoolSngl::instance()->get_fileblock(hash,index,fi->block_size);
		if(!block)
			return -1;
		//如果原来已经将数据读了入来的话，这里也可能被清掉
		block->begin = block->end = bd.size;
		bd.data = (void*)block;
	}
	return block->write(buf,buflen,index_pos); //写到缓冲中
}
void FileStorage::flush_file_all(FileInfo* fi)
{
	TLock<Mutex> l(m_mt);
	fi->flush_file_all();
}
int FileStorage::read_filedata(const hash_t& hash,char *buf,unsigned int buflen,size64_t offset)
{
	TLock<Mutex> l(m_mt);
	FileInfo *fi = NULL;
	bool bReady = true;
	int index1=0;
	int index2=0;
	int iread_fromfile=0;
	int iread_fromcache=0;
	int ret = -1;
	FileMapIter it = m_readyinfo.find(hash);
	if(it==m_readyinfo.end())
	{
		bReady = false;
		it=m_downinfo.find(hash);
		if(it==m_downinfo.end())
			return -1;
	}
	fi = it->second;
	if(fi->size<=offset)
		return -2;
	fi->req_offset = offset;
	if(buflen==0)
		return 0; //用于seek
	iread_fromfile = (int)((fi->size - offset) > (size64_t)buflen ? buflen : (fi->size - offset));
	if(!bReady)
	{
		index1 = (int)(offset / fi->block_size);
		index2 = (int)((offset + iread_fromfile - 1) / fi->block_size);
		if( !fi->bt_finished[index1] )
		{
			unsigned int index_pos = (unsigned int)(offset % fi->block_size);
			BlockDataTable::BlockData_t& bd = fi->bdt[index1];
			FileBlock* block = (FileBlock*)bd.data;
			if(index_pos >= bd.size)
			{
				//磁盘里面没有需要的数据,但cache里面可能有
				if (block && block->begin<=index_pos && block->end > index_pos )
				{
					iread_fromcache = block->end - index_pos;
					if (iread_fromcache > iread_fromfile)
						iread_fromcache = iread_fromfile;
				}
				else
				{
						//还是没数据,返回
						return 0;
				}
				iread_fromfile = 0;
			}
			else
			{
				//磁盘有数据
				int iLen = bd.size - index_pos;
				if (iread_fromfile > iLen)//磁盘里面的数据比需要读的少,但cache里面可能有
				{			  
					if(block && block->end > bd.size)
					{
						assert(block->begin <= bd.size); //可能共享时有从硬盘读到一些数据进来
						iread_fromcache = block->end - bd.size;
						if (iread_fromcache > iread_fromfile - iLen)
							iread_fromcache = iread_fromfile - iLen;
					}
					iread_fromfile = iLen;
				}
			}
		}
		else if (index2 > index1)
		{
			int i= index1+1;
			while (i< index2 && fi->bt_finished[i])
				i++;
			//不再读cache里面的数据
			int iLen = (int)(i*(size64_t)fi->block_size + fi->bdt[i].size - offset);
			if(iread_fromfile>iLen)
				iread_fromfile = iLen;
		}
		//else 所要数据在同一块里面，而且已经下载完整，直接就是iread_fromfile
	}
	bool bget=true;
	int mode = F64_READ;
	if(!bReady) mode |=  F64_WRITE;
	if(fi->open_file(mode))
	{
		ret = 0;
		if(iread_fromfile > 0)
		{
			fi->_file.seek(offset,SEEK_SET);
			ret = fi->_file.read(buf,iread_fromfile);
			if(ret<0)
			{
				assert(0);
				return -1;
			}
			else if(ret<iread_fromfile)
				assert(0);
		}
		if(ret==iread_fromfile && iread_fromcache>0)
		{
			//这里假设index1不完整才有可能读iread_fromcache,小心上面的改动
			char *ptr = ((FileBlock*)fi->bdt[index1].data)->buf+(int)(offset%fi->block_size)+ret;
			memcpy(buf+ret,ptr,iread_fromcache);
			ret += iread_fromcache;
		}
		if(bReady&&fi->ref<=0)
			fi->close_file();
	}
	else
	{
		bget = false;
		if(fi->is_memcache_only())
			ret = 0;
	}

	if(!bget && bReady)
	{
		DEBUGMSG("*** ready file not exist!: %s\n",fi->path.c_str());
		//上报删除文件
		delete_readyinfo(hash,false);
		FILESTORAGE_PRT("delete file........!\n");
#ifdef SM_VOD
		TrackerSngl::instance()->PTL_ReportRemoveFile(hash, fi->filetype);
#else
		TrackerSngl::instance()->PTL_ReportRemoveFile(hash);
#endif /* end of SM_VOD */
	}
	if(ret>0)
	{
		fi->last_req_offset = offset+ret;
		fi->req_offset = offset+ret;
		//Download通过判断req_offset!=last_req_offset时为跳动读数据
	}
	return ret;
}
int FileStorage::read_filedata_trymemcache(const hash_t& hash,char *buf,unsigned int buflen,size64_t offset)
{
	TLock<Mutex> l(m_mt);
	FileInfo *fi = NULL;
	bool bReady = true;
	unsigned int index=0;
	unsigned int index_pos=0;
	unsigned int block_size=0;
	unsigned int iRead=0;
	FileBlock *block = NULL;
	FileInfo::FileBlockMapIter fb_it;

	FileMapIter it = m_readyinfo.find(hash);
	if(it==m_readyinfo.end())
	{
		bReady = false;
		it=m_downinfo.find(hash);
		if(it==m_downinfo.end())
			return -1;
	}
	fi = it->second;
	if(fi->size<=offset)
		return -2;
	fi->req_offset = offset;
	if(buflen<=0)
		return 0; //用于seek
	index = (unsigned int)(offset / fi->block_size);
	index_pos = (unsigned int)(offset % fi->block_size);
	block_size = fi->get_block_size(index);
	iRead = (block_size - index_pos) > buflen ? buflen : (block_size - index_pos);
	if(!bReady && !fi->bt_memfinished[index])
	{
		////其实cache里可能有数据,从cache中读
		//return read_filedata(hash,buf,buflen,offset);
		return 0;
	}
	fb_it = fi->fbmp_mem.find(index);
	if(fb_it!=fi->fbmp_mem.end())
	{
		bReady = true;
		block = fb_it->second;
		//可能块数据不完整
		if(block->begin!=0 || block->end!=block_size)
		{
			block = get_fileblock(hash,index,0,block_size,fi->block_size);
			if(block) 
				put_fileblock(block);
		}
	}
	else
	{
		bReady = false;
		block = get_fileblock(hash,index,0,block_size,fi->block_size);
		//DEBUGMSG("#read file block(%d) \n",index);
	}
	if(!block)
		return 0;
	//assert(block->begin==0 && block->end==block_size);
	memcpy(buf,block->buf+index_pos,iRead);
	if(!bReady)
	{
		fi->try_add_memcache(index,block);
		put_fileblock(block);
	}
	if(iRead>0)
	{
		fi->last_req_offset = offset+iRead;
		fi->req_offset = offset+iRead;
		//Download通过判断req_offset!=last_req_offset时为跳动读数据
	}
	return iRead;
}

int FileStorage::on_blockdone(const hash_t& hash,unsigned int index)
{
	TLock<Mutex> l(m_mt);
	FileInfo *fi = get_downinfo(hash);
	if(!fi)
		return -1;
	if(fi->bt_finished[index])
		return 0;
	if(fi->get_block_downing_size(index)!=fi->get_block_size(index))
	{
		assert(false);
		return -1;
	}
	BlockDataTable::BlockData_t& bd = fi->bdt[index];
	fi->try_add_memcache(index,(FileBlock*)bd.data); //里面会设置bt_memfinished

	//////////////////////////////////////
	//考虑写硬盘时影响读数据，在此写硬盘不使用锁，不过这样操作线程不安全,如果读数据时同样使用此指针的话可能会写乱
	fi->flush_file(index);
	//不保存到硬盘的时候
	if(bd.size < fi->get_block_size(index))
		return -1;
	///////////////////////////////////////

	fi->bt_finished.set_block(index);
	fi->bt_memfinished.set_block(index);
	fi->down_blocks++;
	if(fi->down_blocks % 10 == 0)
		write_downinfo_i(*fi);
	for( ;fi->block_gap<fi->bt_finished.get_block_num() && fi->bt_finished[fi->block_gap]; ++fi->block_gap);
	//检查空间是否可以继续写
	if(FTYPE_VOD==fi->ftype && !fi->is_memcache_only() && (0==index%10))
	{
		if(1==FileAutoCacheSngl::instance()->auto_clear_cache(hash))
			SettingSngl::instance()->set_memcache_only();
	}
	return 0;
}
int FileStorage::on_filedone(const hash_t& hash,const hash_t& new_hash)
{
	TLock<Mutex> l(m_mt);
	FileMapIter it = m_downinfo.find(hash);
	if(it==m_downinfo.end())
		return -1;
	delete_readyinfo(hash,false);
	FileInfo *fi = it->second;
	fi->close_file();
	assert(0==fi->block_offset && fi->down_blocks==fi->blocks);
	if(fi->despath != fi->path)
	{
		File64::remove_file(fi->despath.c_str());
		Util::create_directory_by_filepath(fi->despath);
		FILESTORAGE_PRT(" rename from %s to %s\n", fi->path.c_str(), fi->despath.c_str());
		if(0!=File64::rename_file(fi->path.c_str(),fi->despath.c_str()))
		{
			return -1;
		}
	}
	m_downinfo.erase(it);
	File64::remove_file(fi->info_path.c_str());
	fi->path = fi->despath;
	{
		time_t ct=0,mt=0,at=0;
		Util::get_filetimes(fi->path.c_str(),ct,mt,at);
		fi->mtime = (int)mt;
		fi->ctime = (int)ct;
	}
	string old_tth = fi->tth;
	if(hash!=new_hash)
	{
		fi->hash = new_hash;
		char buf[45];
		fi->hash.to_string(buf,45);
		fi->tth = buf;
	}
#ifdef SM_VOD
	if(FTYPE_VOD==fi->ftype)
		FileAutoCacheSngl::instance()->add_autoinfo(fi->hash,fi->filetype,fi->path); 
#else
	if(FTYPE_VOD==fi->ftype)
		FileAutoCacheSngl::instance()->add_autoinfo(fi->hash,fi->path); //更新路径
#endif /* end of SM_VOD */
	fi->bdt.resize(0,0,false);
	fi->bt_finished.resize(0,0,false);
	fi->bt_memfinished.resize(0,0,false);
	fi->info_path = m_readyinfo_dir + fi->tth.substr(2) + ".inf";

	m_readyinfo[fi->hash] = fi;  //直接不删除fi转给readyinfo，保证引用计数正确
	save_main_downinfo();
	//write_readyinfo_i(*fi);
	//save_main_readyinfo();
	save_readyinfo();
	//DEBUGMSG("#:on file done (old hash=%s ,new hash=%s) \n",old_tth.c_str(),fi->tth.c_str());
	return 0;
}

int FileStorage::read_refer(const hash_t& hash)
{
	TLock<Mutex> l(m_mt);
	bool bready=false;
	FileInfo *fi = NULL;
	FileMapIter it = m_downinfo.find(hash);
	if(it==m_downinfo.end())
	{
		it = m_readyinfo.find(hash);
		if(it==m_readyinfo.end())
			return -1;
		bready = true;
	}
	fi = it->second;
	fi->ref++;
	if(1==fi->ref)
	{
		if(bready)
			fi->open_file(F64_READ);
		//else
		//	fi->open_file(F64_RDWR);
	}
#ifdef SM_VOD
	FileAutoCacheSngl::instance()->on_read_refer(hash,fi->filetype);
#else
	FileAutoCacheSngl::instance()->on_read_refer(hash);
#endif /* end of SM_VOD */
	//AutoMapIter it2=m_autoinfo.find(hash);
	//if(it2!=m_autoinfo.end())
	//	it2->second->score = ++m_autoinfo_score;
	//save_autoinfo();
	return 0;
}
int FileStorage::read_release(const hash_t& hash)
{
	TLock<Mutex> l(m_mt);
	//bool bready=false;
	FileInfo *fi = NULL;
	FileMapIter it = m_downinfo.find(hash);
	if(it==m_downinfo.end())
	{
		it = m_readyinfo.find(hash);
		if(it==m_readyinfo.end())
			return -1;
		//bready = true;
	}
	fi = it->second;
	//read_begin时此任务可能是downloadinfo(在下载队列),在read_end时此任务已经是readyinfo(就绪队列)，注意引用数值传递
	if(fi->ref>0)
		fi->ref--;
	if(0==fi->ref)
	{
		fi->close_file();
		fi->free_memcache_all();
	}
	return 0;
}
int FileStorage::get_sharefile(const hash_t& hash,PTL_P2T_FileInfo& inf)
{
	TLock<Mutex> l(m_mt);
	FileInfo* fi = get_fileinfo(hash);
	if (fi == NULL)
		return -1;
	memcpy(inf.fhash,fi->hash.buffer(),HASHLEN);
	inf.fsize = fi->size;
	return 0;
}
int FileStorage::get_sharefile_all(list<PTL_P2T_FileInfo*>& ls)
{
	TLock<Mutex> l(m_mt);
	//注意:不清空原来的ls
	FileMapIter it;
	FileInfo* fi = NULL;
	PTL_P2T_FileInfo *inf = NULL;
	for(it=m_readyinfo.begin(); it!=m_readyinfo.end(); ++it)
	{
		fi = it->second;
#ifdef SM_VOD
		/* vod will report */
		if(PLAYTYPE_VOD!=fi->filetype) {
			if(HT_URL2==fi->hash.hash_type())
				continue;
		}
#else
		//url2类型不共享
		if(HT_URL2==fi->hash.hash_type())
			continue;
#endif /* end of SM_VOD */
		inf = new PTL_P2T_FileInfo();
		memcpy(inf->fhash,fi->hash.buffer(),HASHLEN);
		inf->fsize = fi->size;
#ifdef SM_VOD
		inf->filetype = fi->filetype;
#endif
		ls.push_back(inf);
	}
	for(it=m_downinfo.begin(); it!=m_downinfo.end(); ++it)
	{
		fi = it->second;
		if(fi->size==0)
			continue;

#ifdef SM_VOD
		if((HT_URL2==fi->hash.hash_type()) && (PLAYTYPE_VOD!=fi->filetype)) {
			continue; 
		}
		else if(UINT64_INFINITE==fi->size) {
			if(fi->ref<=0)
				continue;
		} else {
			//有20M数据开始共享
			if(fi->down_blocks < MIN_SHARE_BLOCKS)
				continue;
		}
#else

		//url2类型不共享
		if(HT_URL2==fi->hash.hash_type())
			continue;
		else if(UINT64_INFINITE==fi->size)
		{
			if(fi->ref<=0)
				continue;
		}
		else
		{
			//有20M数据开始共享
			if(fi->down_blocks < MIN_SHARE_BLOCKS)
				continue;
		}
#endif /* end of SM_VOD */

		inf = new PTL_P2T_FileInfo();
		if(!inf) continue;
		memcpy(inf->fhash,fi->hash.buffer(),HASHLEN);
		inf->fsize = fi->size;
		ls.push_back(inf);
	}
	return 0;
}
