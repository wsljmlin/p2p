#include "DownloadManager.h"
#include "Timer.h"
#include "FileStorage.h"
#include "FileAutoCache.h"
#include "Util.h"
#include "Setting.h"
#include "Statistician.h"
#include "DownloadListManager.h"
#include "LocalM3u8.h"

#define GET_DOWNLOAD_RETURN(_name,_hash,ret) Download* _name=get_download(_hash); if(!_name) return ret;

#ifdef SM_DBG
#define DOWNLOADMANGER_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "DownloadManager", __LINE__, ##arg)
#else
#define DOWNLOADMANGER_PRT(fmt, arg...)
#endif


DownloadManager::DownloadManager(void)
: Speaker<DownloadManagerListener>(1)
, m_iCurrTick(0)
{
}

DownloadManager::~DownloadManager(void)
{
}

int DownloadManager::init()
{
	TrackerSngl::instance()->add_listener(this);
	TimerSngl::instance()->register_timer(this,1,1000);
	return 0;
}
int DownloadManager::fini()
{
	TrackerSngl::instance()->remove_listener(this);
	TimerSngl::instance()->unregister_all(this);
	clear_download_all();
	return 0;
}

int DownloadManager::create_download(const hash_t& hash,const string& path,const string& url/*=""*/,int http_max_num/*=-1*/,size64_t size/*=0*/,size64_t offset/*=0*/,
		unsigned int blockSize/*=DEFAULT_BLOCK_SIZE*/,int ftype/*=FTYPE_VOD*/,bool isStart/*=true*/,int rdbftype/*=RDBF_AUTO*/)
{
	if(path.empty()) 
		return -1;

	//update FileInfo
	FileInfo *fi = NULL;
#ifdef SM_DBG
	fi=FileStorageSngl::instance()->get_readyinfo(hash);
	if(NULL != fi) {
		DOWNLOADMANGER_PRT("file have existed!\n");
	}
#endif
	fi = FileStorageSngl::instance()->get_fileinfo(hash);
	if(NULL!=fi && FTYPE_SHAREONLY != fi->ftype)
	{
		if(FTYPE_VOD==fi->ftype && FTYPE_DOWNLOAD==ftype)
			FileStorageSngl::instance()->update_downinfo_path(hash,path);	
		if(FTYPE_DOWNLOAD==fi->ftype)
		{
			//下载类型不改变
			ftype = FTYPE_DOWNLOAD;
		}
		FileStorageSngl::instance()->update_downinfo_type(hash,ftype);
	}

	if(FTYPE_VOD==ftype)
	{
		FileAutoCacheSngl::instance()->auto_clear_cache(hash);
	}

	if(ExistDownload(hash,path))
	{
		DOWNLOADMANGER_PRT("current download existed!\n");
		Download *dl = get_download(hash);
		if(dl)
		{
			if(isStart)
				dl->start();
			else
				dl->stop();
		}
		return 1;
	}

	fi=FileStorageSngl::instance()->get_downinfo(hash);
	if(fi && fi->check_memcache_done())
		return 1;
	DOWNLOADMANGER_PRT("file is not cache done!\n");
	//create download
	Download *dl = new Download();
	if(!dl)
		return -1;
	dl->add_listener(this);
	bool isCreate = false;
	if(fi)
	{
		//前面已经更新过ftype
		if(dl->create_old_download(fi,url,http_max_num,isStart))
			isCreate = true;
	}
	else
	{
		if(dl->create_new_download(hash,path,url,http_max_num,size,offset,blockSize,ftype,isStart,rdbftype))
			isCreate = true;
	}
	if(isCreate)
	{
		m_downloads[hash] = dl;
		return 0;
	}
	else
	{
		dl->remove_listener(this);
		delete dl;
		return -1;
	}
}

#ifdef SM_VOD
int DownloadManager::create_download(const hash_t& hash,int playtype, const string& path,const string& url/*=""*/,int http_max_num/*=-1*/,size64_t size/*=0*/,size64_t offset/*=0*/,
		unsigned int blockSize/*=DEFAULT_BLOCK_SIZE*/,int ftype/*=FTYPE_VOD*/,bool isStart/*=true*/,int rdbftype/*=RDBF_AUTO*/)
{
	if(path.empty()) 
		return -1;

	//update FileInfo
	FileInfo *fi = NULL;
#ifdef SM_DBG
	fi=FileStorageSngl::instance()->get_readyinfo(hash);
	if(NULL != fi) {
		DOWNLOADMANGER_PRT("file have existed!\n");
	}
#endif
	fi = FileStorageSngl::instance()->get_fileinfo(hash);
	if(NULL!=fi && FTYPE_SHAREONLY != fi->ftype)
	{
		if(FTYPE_VOD==fi->ftype && FTYPE_DOWNLOAD==ftype)
			FileStorageSngl::instance()->update_downinfo_path(hash,path);	
		if(FTYPE_DOWNLOAD==fi->ftype)
		{
			//下载类型不改变
			ftype = FTYPE_DOWNLOAD;
		}
		FileStorageSngl::instance()->update_downinfo_type(hash,ftype);
	}

	if(FTYPE_VOD==ftype)
	{
		FileAutoCacheSngl::instance()->auto_clear_cache(hash);
	}

	if(ExistDownload(hash,path))
	{
		DOWNLOADMANGER_PRT("current download existed!\n");
		Download *dl = get_download(hash);
		if(dl)
		{
			if(isStart)
				dl->start();
			else
				dl->stop();
		}
		return 1;
	}

	fi=FileStorageSngl::instance()->get_downinfo(hash);
	if(fi && fi->check_memcache_done())
		return 1;
	DOWNLOADMANGER_PRT("file is not cache done!\n");
	//create download
	Download *dl = new Download();
	if(!dl)
		return -1;
	dl->add_listener(this);
	dl->set_playtype(playtype);
	DOWNLOADMANGER_PRT("<<<<<<<playtype=%d type=%d\n", playtype, dl->get_playtype());
	bool isCreate = false;
	if(fi)
	{
		//前面已经更新过ftype
		if(dl->create_old_download(fi,url,http_max_num,isStart))
			isCreate = true;
	}
	else
	{
		if(dl->create_new_download(hash,path,url,http_max_num,size,offset,blockSize,ftype,isStart,rdbftype))
			isCreate = true;
	}
	if(isCreate)
	{
		m_downloads[hash] = dl;
		return 0;
	}
	else
	{
		dl->remove_listener(this);
		delete dl;
		return -1;
	}
}

int DownloadManager::on_file_done(const hash_t& hash,int playtype, const hash_t& newHash,char done_type)
{
	//有可能在这之前外部执行了release_i()，而导，此时会先调用了downloadlist::on_file_stop()
	//删除download时stop()会调用downloadlist::on_file_stop(),跟着执行release_i();
	//bool isFind = false;
	DownloadListManagerSngl::instance()->on_file_done(hash,done_type); //
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
	{
		assert(it->second->get_playtype()==playtype);
		//isFind = true;
		Download *dl = it->second;
		m_downloads.erase(it); //先释放再stop保证stop时执行的release_i()中找不到了
		dl->stop();
		dl->remove_listener(this); //stop之后再删除listener保证callback到on_stop
		delete dl;
	}
	if(1==done_type)
	{
		//memcache_done不执行以下动作
		FileStorageSngl::instance()->on_filedone(hash,newHash);
		//if(isFind)
		{
			fire(DownloadManagerListener::DLFileDone(),hash);
		}
		if(hash != newHash)
		{
			DOWNLOADMANGER_PRT("error download done!\n");
			TrackerSngl::instance()->PTL_ReportRemoveFile(hash, playtype);
			TrackerSngl::instance()->PTL_ReportDownloadWrong(hash,newHash);
			assert(0);
		}
		
		DOWNLOADMANGER_PRT("<<<<<<report to tracker, playtype=%d\n", playtype);
		TrackerSngl::instance()->PTL_ReportShareFile(newHash, (int)playtype);

#ifdef SM_DBG
		char buf[45];
		newHash.to_string(buf,45);
		DOWNLOADMANGER_PRT("report to tracker:%s\n", buf );
#endif

	}
	return 0;
}

int DownloadManager::delete_file(const hash_t& hash,bool isDelPhy, bool isclear)
{
	FileInfo *fi = FileStorageSngl::instance()->get_fileinfo(hash);
	if(fi && FTYPE_DOWNLOAD==fi->ftype && fi->ref>0)
	{
		//转为点播类型
		FileStorageSngl::instance()->update_downinfo_type(hash,FTYPE_VOD);
		return -1;
	}
	delete_download(hash);

	/* really clear will delte and report, or will only close download */
	if(true==isclear) {
		if(fi) {
			DOWNLOADMANGER_PRT("filetype=%d\n", fi->filetype);
			TrackerSngl::instance()->PTL_ReportRemoveFile(hash, fi->filetype);
		}
		FileStorageSngl::instance()->delete_fileinfo(hash,true);
	} else {
		FileStorageSngl::instance()->delete_fileinfo_noerase(hash,false);
	}
	
	DOWNLOADMANGER_PRT("delete file!!!!!\n");
	return 0;
}

#endif /* end of SM_VOD */

int DownloadManager::delete_download(const hash_t& hash)
{
	Download *dl = NULL;
	DownloadIter it = m_downloads.find(hash);
	if(it==m_downloads.end())
		return -1;
	dl = it->second;
	m_downloads.erase(it);
	dl->stop();
	dl->remove_listener(this);
	delete dl;
	return 0;
}
int DownloadManager::start_download(const hash_t& hash)
{
	DownloadIter it=m_downloads.find(hash);
	if(it!=m_downloads.end())
	{
		return it->second->start();
	}
	else
	{
		FileInfo *fi = FileStorageSngl::instance()->get_downinfo(hash);
		if(!fi)
			return -1;
		Download *dl = new Download();
		if(dl)
		{
			dl->add_listener(this);
			if(dl->create_old_download(fi,"",true))
			{
				m_downloads[fi->hash]=dl;
			}
			else
			{
				dl->remove_listener(this);
				delete dl;
			}
		}
	}
	return -1;
}

Download* DownloadManager::get_download(const hash_t& hash)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
		return it->second;
	return NULL;
}
int DownloadManager::get_downloadinfo(const hash_t& hash,DownloadInfo& inf)
{
	Download *dl = get_download(hash);
	if(dl)
	{
		inf = dl->get_downloadinfo();
		return 0;
	}
	FileInfo *fi = FileStorageSngl::instance()->get_readyinfo(hash);
	if(fi)
	{
		inf.size = fi->size;
		inf.reqOffset = fi->req_offset;
		inf.hash = hash;
		inf.blocks = 1;
		inf.down_blocks = 1;
		inf.bufferingSize = inf.size;
		inf.streamSize = inf.size;
		inf.speedB = 0;
		inf.connNum = 0;
		inf.srcNum = 0;
		inf.httpConnNum = 0;
		return 0;
	}
	fi = FileStorageSngl::instance()->get_downinfo(hash);
	if(fi)
	{
		inf.size = fi->size;
		inf.reqOffset = fi->req_offset;
		inf.hash = hash;
		inf.blocks = fi->blocks;
		inf.down_blocks = fi->down_blocks;
		inf.bufferingSize = 0;
		inf.streamSize = 0;
		inf.speedB = 0;
		inf.connNum = 0;
		inf.srcNum = 0;
		inf.httpConnNum = 0;
		return 0;
	}
	return -1;
}

int DownloadManager::stop_download_all()
{
	for(DownloadIter it=m_downloads.begin();it!=m_downloads.end();++it)
		it->second->stop();
	return 0;
}
int DownloadManager::clear_download_all()
{
	stop_download_all();
	for(DownloadIter it=m_downloads.begin();it!=m_downloads.end();++it)
	{
		it->second->remove_listener(this);
		delete it->second;
	}
	m_downloads.clear();
	return 0;
}

int DownloadManager::share_file(const hash_t& hash,const string& path)
{
	delete_download(hash);
	if(FileStorageSngl::instance()->create_readyinfo(hash,path,0,FTYPE_SHAREONLY))
	{
		DEBUGMSG("#:share file succeed (%s) \n",path.c_str());
		TrackerSngl::instance()->PTL_ReportShareFile(hash);
		return 0;
	}
	return -1;
}
int DownloadManager::delete_file(const hash_t& hash,bool isDelPhy)
{
	FileInfo *fi = FileStorageSngl::instance()->get_fileinfo(hash);
	if(fi && FTYPE_DOWNLOAD==fi->ftype && fi->ref>0)
	{
		//转为点播类型
		FileStorageSngl::instance()->update_downinfo_type(hash,FTYPE_VOD);
		return -1;
	}
	delete_download(hash);
	FileStorageSngl::instance()->delete_fileinfo(hash,isDelPhy);
	TrackerSngl::instance()->PTL_ReportRemoveFile(hash);
	DOWNLOADMANGER_PRT("delete file!!!!!\n");
	return 0;
}

int DownloadManager::read_refer(const hash_t& hash)
{
	//DEBUGMSG("#:DownloadManager::read_refer()...\n");
	return FileStorageSngl::instance()->read_refer(hash);
}
int DownloadManager::read_release(const hash_t& hash)
{
	m_releases.push_back(ReleaseInfo(hash,GetTickCount()));
	return 0;
}
int DownloadManager::read_release_i(const hash_t& hash)
{
	//DEBUGMSG("#:DownloadManager::read_release()_i--!\n");
	FileStorageSngl::instance()->read_release(hash);
	FileInfo *fi = FileStorageSngl::instance()->get_fileinfo(hash);
	if(fi && fi->ref<=0)
	{
		//DEBUGMSG("#try delete_download(); fi->ref=%d\n",fi->ref);
		delete_download(hash);
	}
	return 0;
}
int DownloadManager::on_ptl_packet(Peer* peer,uint16 cmd,PTLStream& ss)
{
	Download *dl = NULL;
	switch(cmd)
	{
	case PTL_P2T_RESPONSE_FILE_SOURCE:
		{
			PTL_P2T_ResponseFileSource rsp;
			if(0==ss>>rsp)
			{
				m_hash.set_buffer((uchar*)rsp.fhash);
				dl = get_download(m_hash);
				if(!dl)
				{
					DOWNLOADMANGER_PRT("--no current download, when online speed is low may enter this!!!\n");
					DownloadListManagerSngl::instance()->on_response_source(m_hash,rsp);
					//downloadlist的源由downloadlist统一分配，不再在此进行特殊处理
					//hash_t token_hash;
					//DownloadListManagerSngl::instance()->get_token_hash(m_hash,token_hash);
					//if(!token_hash.empty())
					//	dl = get_download(token_hash);
				}
				if(dl)
					dl->ON_PTL_ResponseFileSource(peer,rsp);
			}
		}
		break;
	case PTL_P2P_RESPONSE_FILE_BLOCK_TABLE:
		{
			PTL_P2P_ResponseFileBlockTable rsp;
			if(0==ss>>rsp)
			{
				m_hash.set_buffer((uchar*)rsp.fhash);
				dl = get_download(m_hash);
				if(dl)
					dl->ON_PTL_ResponseFileBlockTable(peer,rsp);
			}
		}
		break;
	case PTL_P2P_RESPONSE_FILE_BLOCKS:
		{
			PTL_P2P_ResponseFileBlocks rsp;
			if(0==ss>>rsp)
			{
				m_hash.set_buffer((uchar*)rsp.fhash);
				dl = get_download(m_hash);
				if(dl)
					dl->ON_PTL_ResponseFileBlocks(peer,rsp);
			}
		}
		break;
	case PTL_P2P_RESPONSE_FILE_BLOCKS_DATA:
		{
			PTL_P2P_ResponseFileBlocksData rsp;
			if(0==ss>>rsp)
			{
				m_hash.set_buffer((uchar*)rsp.fhash);
				dl = get_download(m_hash);
				if(dl)
					dl->ON_PTL_ResponseFileBlocksData(peer,rsp);
			}
		}
		break;
	case PTL_P2P_RESPONSE_FILE_SUB_KEYS:
		{
			PTL_P2P_ResponseFileSubKeys rsp;
			if(0==ss>>rsp)
			{
				m_hash.set_buffer((uchar*)rsp.fhash);
				//todo:
			}
		}
		break;
	default:
		assert(0);
		break;
	}
	return 0;
}
int DownloadManager::on_peer_ready(Peer* peer,void* dref)
{
	for(DownloadIter it=m_downloads.begin();it!=m_downloads.end();++it)
	{
		if(it->second == (Download*)dref)
			return it->second->on_peer_ready(peer);
	}
	return 0;
}
void DownloadManager::on_peer_close(Peer* peer,void* dref)
{
	for(DownloadIter it=m_downloads.begin();it!=m_downloads.end();++it)
	{
		if(it->second == (Download*)dref)
			it->second->on_peer_close(peer);
	}
}
int DownloadManager::on_peer_turn(Peer* peer,uint32 sessionID)
{
	Download* dl = NULL;
	for (TurnIter it = m_turns.begin(); it != m_turns.end();)
	{
		if ((*it).sessionID == sessionID && (*it).type == 0)
		{
			dl = get_download((*it).hash);
			it = m_turns.erase(it);
			if(dl)
			{
				if (0==dl->on_peer_turn(peer,sessionID))
					return true;
				return false;
			}
		}
		else
		{
			++it;
		}
	}
	return false;
}

int DownloadManager::on_file_done(const hash_t& hash,const hash_t& newHash,char done_type)
{
	//有可能在这之前外部执行了release_i()，而导，此时会先调用了downloadlist::on_file_stop()
	//删除download时stop()会调用downloadlist::on_file_stop(),跟着执行release_i();
	//bool isFind = false;
	DownloadListManagerSngl::instance()->on_file_done(hash,done_type); //
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
	{
		//isFind = true;
		Download *dl = it->second;
		m_downloads.erase(it); //先释放再stop保证stop时执行的release_i()中找不到了
		dl->stop();
		dl->remove_listener(this); //stop之后再删除listener保证callback到on_stop
		delete dl;
	}
	if(1==done_type)
	{
		//memcache_done不执行以下动作
		FileStorageSngl::instance()->on_filedone(hash,newHash);
		//if(isFind)
		{
			fire(DownloadManagerListener::DLFileDone(),hash);
		}
		if(hash != newHash)
		{
			TrackerSngl::instance()->PTL_ReportRemoveFile(hash);
			TrackerSngl::instance()->PTL_ReportDownloadWrong(hash,newHash);
			assert(0);
		}
		TrackerSngl::instance()->PTL_ReportShareFile(newHash);

#ifdef SM_DBG
		char buf[45];
		newHash.to_string(buf,45);
		DOWNLOADMANGER_PRT("report to tracker:%s\n", buf );
#endif

	}
	return 0;
}

bool DownloadManager::create_turn_channel(uint32 trackID,uint32 sessionID,const hash_t& hash)
{
	if(TrackerSngl::instance()->PTL_RequestConnTurn(trackID,sessionID) == 0)
	{
		m_turns.push_back(TurnInfo(hash,sessionID,0,m_iCurrTick));
		return true;
	}
	return false;
}

void DownloadManager::on_timer(int e)
{
	switch(e)
	{
	case 1:
		{
			m_iCurrTick++;
			TimerDownloadHeartbeat();
			TimerClearTimeoutTurn();
			TimerRelease();
			DownloadListManagerSngl::instance()->on_second();
			//DEBUGMSG("#tick = %d \n",GetTickCount());
		}
		break;
	default:
		assert(0);
		break;
	}
}

void DownloadManager::TimerDownloadHeartbeat()
{
	for(DownloadIter it=m_downloads.begin();it!=m_downloads.end();++it)
		it->second->on_second();
}
void DownloadManager::TimerClearTimeoutTurn()
{
	if(m_iCurrTick % 10!=0)
		return;
	TurnList ls;
	TurnIter it;
	for(it=m_turns.begin();it!=m_turns.end();)
	{
		if(((*it).beginTick + 45) < m_iCurrTick)
		{
			ls.push_back(*it);
			m_turns.erase(it++);
		}
		else
			++it;
	}
	for(it=ls.begin();it!=ls.end();++it)
	{
		Download *dl = get_download((*it).hash);
		if(dl)
		{
			if(0==(*it).type)
			{
				//turn failed
				dl->on_peer_turn(NULL,(*it).sessionID);
			}
			else
				assert(0);
		}
	}
	ls.clear();
}
void DownloadManager::TimerRelease()
{
	ReleaseIter it;
	DWORD tick = GetTickCount();
	for(it=m_releases.begin();it!=m_releases.end();)
	{
		if((*it).beginTick + 2000 < tick)
		{
			read_release_i((*it).hash);
			m_releases.erase(it++);
		}
		else
			break;
	}
}
bool DownloadManager::ExistDownload(const hash_t& hash,const string& path)
{
	if(m_downloads.find(hash)!=m_downloads.end())
		return true;

	//本地测试下载
	if(!SettingSngl::instance()->get_force_download() && FileStorageSngl::instance()->get_readyinfo(hash))
		return true;
	return false;
}
//int DownloadManager::auto_clear_cache(const hash_t& hashExclude)
//{
//	ULONGLONG total=0,used=0,free=0,minFree=0;
//	Util::get_volume_size(SettingSngl::instance()->get_cache_path(),total,used,free);
//	minFree = SettingSngl::instance()->get_disk_min_free_spaceMB();
//	minFree = minFree << 20; //默认最少保证该盘1G的空间
//	uint64 cacheSize = FileStorageSngl::instance()->get_autocache_size();
//	uint64 maxSize = SettingSngl::instance()->get_cache_sizeMB();
//	maxSize = maxSize << 20;
//	uint64 sizetmp=0, sizetmp2=0;
//	if(cacheSize > maxSize || (cacheSize>0 && total!=0 && free<minFree))
//	{
//		FileStorage::AutoMap mp;
//		FileStorage::AutoMapIter it,min_it;
//		FileStorageSngl::instance()->get_autocache_map(mp);
//
//		unsigned int minScore = 10000;
//		bool isFind = false;
//		hash_t hash;
//		FileInfo *fi=NULL;
//		////每次删除留400M空间或者1/8 中的大者
//		//sizetmp = maxSize/8;  //
//		//if( sizetmp< (400<<20) ) 
//		sizetmp = 400<<20;
//		while((cacheSize + sizetmp) > maxSize || (total!=0 && free<minFree))
//		{
//			isFind = false;
//			minScore = 10000;
//			for(it=mp.begin();it!=mp.end();++it)
//			{
//				if(it->first == hashExclude)
//					continue;
//				if(minScore>it->second->score)
//				{
//					minScore = it->second->score;
//					min_it = it;
//					isFind = true;
//				}
//			}
//			if(!isFind)
//				break;
//			hash = min_it->first;
//			mp.erase(min_it);
//			fi = FileStorageSngl::instance()->get_fileinfo(hash);
//			if(!fi || fi->ref)
//				continue;
//			sizetmp2 = File64::get_file_size(fi->path.c_str());
//			cacheSize -= sizetmp2;
//			delete_file(hash,true);
//			//异步删除不能实时取
//			//Util::get_volume_size(SettingSngl::instance()->get_cache_path(),total,used,free);
//			if(total!=0) free+=sizetmp2;
//		}
//	}
//	return 0;
//}
int DownloadManager::get_source_list(const hash_t& hash,list<SourceInfo*>& ls)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
		return it->second->get_source_list(ls);
	return -1;
}
int DownloadManager::add_source_list(const hash_t& hash,list<SourceInfo*>& ls)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
		return it->second->add_source_list(ls);
	return -1;
}
int DownloadManager::http_add_source(const hash_t& hash,const string& url,bool trycreate)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
	{
		if(it->second->http_add_source(url,0,trycreate))
		{
			it->second->http_create_all_channel();
			return 0;
		}
	}
	return -1;
}
int DownloadManager::on_attach_peer(const hash_t& hash,Peer* peer,int ctype)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
	{
		if(0==it->second->attach_peer(peer,true,ctype))
		{
			it->second->on_peer_ready(peer);
			return 0;
		}
	}
	return -1;
}
int DownloadManager::on_attach_http_peer(const hash_t& hash,HttpPeer* peer,const string& url)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
		return it->second->on_attach_http_peer(peer,url);
	return -1;
}
int DownloadManager::update_http_max_num(const hash_t& hash,int n)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
		return it->second->update_http_max_num(n);
	return -1;
}
int DownloadManager::get_connect_num(const hash_t& hash,int& p2pconn_num,int& httpconn_num)
{
	DownloadIter it = m_downloads.find(hash);
	if(it!=m_downloads.end())
	{
		p2pconn_num = it->second->get_p2pconnect_amount();
		httpconn_num = it->second->get_httpconnect_amount();
		return 0;
	}
	return -1;
}

void DownloadManager::on(TrackerListener::Connected)
{
	for(DownloadIter it=m_downloads.begin();it!=m_downloads.end();++it)
		it->second->on_tracker_connected();
	DownloadListManagerSngl::instance()->on_tracker_connected();
	LocalM3u8MgrSngl::instance()->on_tracker_connected();
}

void DownloadManager::on(DownloadListener::DLCreate,const DownloadInfo& inf)
{
	fire(DownloadManagerListener::DLCreate(),inf);
}

void DownloadManager::on(DownloadListener::DLStart,const DownloadInfo& inf)
{
	fire(DownloadManagerListener::DLStart(),inf);
}
void DownloadManager::on(DownloadListener::DLStop,void* dl,const DownloadInfo& inf)
{
	fire(DownloadManagerListener::DLStop(),inf);
	DownloadListManagerSngl::instance()->on_file_stop(inf.hash,dl);
}
void DownloadManager::on(DownloadListener::DLDownloading,const DownloadInfo& inf)
{
	fire(DownloadManagerListener::DLDownloading(),inf);
}
void DownloadManager::on(DownloadListener::DLBlockTable,const hash_t& hash,const BlockTable& inf)
{
	fire(DownloadManagerListener::DLBlockTable(),hash,inf);
}
void DownloadManager::on(DownloadListener::DLBlockDone,const hash_t& hash,int index)
{
	fire(DownloadManagerListener::DLBlockDone(),hash,index);
}

