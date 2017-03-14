#include "DownloadListManager.h"
#include "Setting.h"

#ifdef SM_DBG
#include "HttpClient.h"

#define DOWNLOADLISTMANGER_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "DownloadListManager", __LINE__, ##arg)
#else
#define DOWNLOADLISTMANGER_PRT(fmt, arg...) 
#endif


DownloadListManager::DownloadListManager(void)
{
}

DownloadListManager::~DownloadListManager(void)
{
}

int DownloadListManager::init()
{
	return 0;
}

void DownloadListManager::fini()
{
	closeall_downloadlist();
}

int DownloadListManager::open_downloadlist(const string& url,const string& name,bool closeother,bool autoclose/*=false*/,bool bopenbytracker/*=false*/)
{
	DOWNLOADLISTMANGER_PRT("url=%s\n", url.c_str());
	if(url.empty())
		return -1;
	hash_t hash;
	hash.set_urldl_string(url.c_str());
	if(closeother)
		closeall_downloadlist_exclude(hash);
	DLIter it = m_dlmap.find(hash);
	if(it!=m_dlmap.end())
		return 0;
	if((int)m_dlmap.size()>=SettingSngl::instance()->get_downloadlist_maxnum())
		return 2; //2表示目前队列满
	DownloadList* dl = DownloadList::open(hash,url,name,autoclose,bopenbytracker);
	if(!dl)
		return -1;
	m_dlmap[hash] = dl;
	m_dlmap2[hash] = url;
	DOWNLOADLISTMANGER_PRT("open download succesd!\n");
	return 0;
}

#ifdef SM_VOD
int DownloadListManager::open_downloadlist(const string& url,const string& name,bool closeother,int playtype,bool autoclose/*=false*/,bool bopenbytracker/*=false*/)
{
	DOWNLOADLISTMANGER_PRT("url=%s\n", url.c_str());
	if(url.empty())
		return -1;

	hash_t hash;
	hash.set_urldl_string(url.c_str());
	if(closeother)
		closeall_downloadlist_exclude(hash);
	DLIter it = m_dlmap.find(hash);
	if(it!=m_dlmap.end()) 
		return 0;
	
	if((int)m_dlmap.size()>=SettingSngl::instance()->get_downloadlist_maxnum())
		return 2; //2表示目前队列满
	DownloadList* dl = DownloadList::open(hash,url,name,autoclose,playtype,bopenbytracker);
	if(!dl)
		return -1;
	
	m_dlmap[hash] = dl;
	m_dlmap2[hash] = url;
	DOWNLOADLISTMANGER_PRT("open download succesd!\n");
	return 0;

}

int DownloadListManager::get_downloadcachetime(const string& url, int& time) {
	hash_t hash;
	hash.set_urldl_string(url.c_str());
	DLIter it = m_dlmap.find(hash);
	if(it!=m_dlmap.end()) {
		it->second->get_downloadcachetime(time);
	} else {
		time = 0;
	}
	return 0;
}

int DownloadListManager::get_fileurl(const hash_t& hash, string& url) {
	assert(HT_URL2==hash.hash_type());
	hash_t dlhash;
	hash.url2hash_to_urldlhash(dlhash);
	
	DLIter it = m_dlmap.find(dlhash);
	if(it!=m_dlmap.end()) {
		it->second->get_fileurl(hash, url);
		return 0;
	} 
	return -1;
}

int DownloadListManager::get_filepath(const hash_t& hash, string& path) {
	assert(HT_URL2==hash.hash_type());
	hash_t dlhash;
	hash.url2hash_to_urldlhash(dlhash);
	
	DLIter it = m_dlmap.find(dlhash);
	if(it!=m_dlmap.end()) {
		it->second->get_filepath(hash, path);
		return 0;
	} 
	return -1;
}
#endif

int DownloadListManager::close_downloadlist(const string& url,bool bopenbytracker/*=false*/)
{
	hash_t hash;
	hash.set_urldl_string(url.c_str());
	return close_downloadlist(hash,bopenbytracker);
}
int DownloadListManager::close_downloadlist(const hash_t& hash,bool bopenbytracker/*=false*/)
{
	DLIter it = m_dlmap.find(hash);
	if(it!=m_dlmap.end())
	{
		//如果是tracker通知停止，则只停止由tracker开启的节目
		if(bopenbytracker && !it->second->is_openbytracker())
			return 0;
		it->second->close();
		m_dlmap2.remove(it->first);
		m_dlmap.erase(it);
	}
	return 0;
}
int DownloadListManager::closeall_downloadlist()
{
	for(DLIter it=m_dlmap.begin();it!=m_dlmap.end();++it)
	{
		it->second->close();
	}
	m_dlmap.clear();
	m_dlmap2.clear();
	return 0;
}
int DownloadListManager::closeall_downloadlist_exclude(const hash_t& hash)
{
	for(DLIter it=m_dlmap.begin();it!=m_dlmap.end();)
	{
		if(it->first == hash)
		{
			++it;
		}
		else
		{
			it->second->close();
			m_dlmap2.remove(it->first);
			m_dlmap.erase(it++);
		}
	}
	return 0;
}
int DownloadListManager::get_downloadlist(const string& url,list<string>& ls)
{
	hash_t hash;
	hash.set_urldl_string(url.c_str());
	DLIter it = m_dlmap.find(hash);
	if(it!=m_dlmap.end())
		return it->second->get_downloadlist(ls);
	return -1;
}
int DownloadListManager::get_downloadlist_info(const string& url,MsgDownloadlistInfo_t *inf)
{
	hash_t hash;
	hash.set_urldl_string(url.c_str());
	DLIter it = m_dlmap.find(hash);
	if(it!=m_dlmap.end())
		return it->second->get_msg_downloadlist_info(inf);
	return -1;
}
int DownloadListManager::get_all_downloadlist_info(list<MsgDownloadlistInfo2_t>& ls)
{
	MsgDownloadlistInfo2_t inf;
	for(DLIter it=m_dlmap.begin();it!=m_dlmap.end();++it)
	{
		if(0==it->second->get_msg_downloadlist_info2(&inf))
			ls.push_back(inf);
	}
	return 0;
}
int DownloadListManager::need_download(const char* strhash)
{
	//http://127.0.0.1:7080/playlist/file/strhash.ts
	//strhash: 4_xxx...xxx
	hash_t plhash;
	hash_t fhash;
	plhash.set_urldl_string_hash(strhash+2);
	fhash.set_url2_string_hash(strhash+2);
#ifdef SM_DBG
	char bufdl[45];
	char buf2[45];
	plhash.to_string(bufdl,45);
	fhash.to_string(buf2,45);
	DOWNLOADLISTMANGER_PRT("need downloadfilehash=%s urdl=%s url2=%s\n", strhash, bufdl, buf2);
#endif
	DLIter it=m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->need_download(fhash);
	return -1;
}
int DownloadListManager::need_download_end(const char* strhash)
{
	//http://127.0.0.1:7080/playlist/file/strhash.ts
	//strhash: 4_xxx...xxx
	hash_t plhash;
	hash_t fhash;
	plhash.set_urldl_string_hash(strhash+2);
	fhash.set_url2_string_hash(strhash+2);
	DLIter it=m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->need_download_end(fhash);
	return -1;
}
int DownloadListManager::get_all_downloadlists(list<string>& ls)
{
	for(DLIter2 it=m_dlmap2.begin();it!=m_dlmap2.end();++it)
	{
		ls.push_back(it->second);
	}
	return 0;
}
int DownloadListManager::on_response_source(const hash_t& hash,PTL_P2T_ResponseFileSource& rsp)
{
	DLIter it=m_dlmap.find(hash);
	if(it!=m_dlmap.end())
	{
		return it->second->on_response_source(rsp);
	}
	return -1;
}
int DownloadListManager::get_token_hash(const hash_t& hash,hash_t& token_hash)
{
	DLIter it=m_dlmap.find(hash);
	if(it!=m_dlmap.end())
	{
		token_hash = it->second->get_token_hash();
		return 0;
	}
	return -1;
}
void DownloadListManager::on_tracker_connected()
{
	for(DLIter it=m_dlmap.begin();it!=m_dlmap.end();++it)
	{
		it->second->on_tracker_connected();
	}
}
int DownloadListManager::on_file_done(const hash_t& hash,char done_type)
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		it->second->on_file_done(hash,done_type);
	return 0;
}
int DownloadListManager::on_file_stop(const hash_t& hash,void* dl)
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		it->second->on_file_stop(hash,dl);
	return 0;
}
int DownloadListManager::on_peer_reattach(const hash_t& hash,Peer* peer,int ctype)
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_peer_reattach(hash,peer,ctype);
	return -1;
}
int DownloadListManager::on_http_peer_reattach(const hash_t& hash,HttpPeer* peer)
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_http_peer_reattach(hash,peer);
	return -1;
}

int DownloadListManager::on_peer_connect_failed(const hash_t& hash,unsigned int sessionID) //连接失败计数，失败多的丢弃源
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_peer_connect_failed(hash,sessionID);
	return -1;
}
int DownloadListManager::on_peer_req_nodata(const hash_t& hash,unsigned int sessionID) //一次连接完成，从连接中获取不到任何数据
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_peer_req_nodata(hash,sessionID);
	return -1;
}
int DownloadListManager::on_peer_req_havedata(const hash_t& hash,unsigned int sessionID) //一次连接完成，从连接中成功获取到过数据
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_peer_req_havedata(hash,sessionID);
	return -1;
}
int DownloadListManager::on_download_bytes(const hash_t& hash,int size,int iptype,int utype)
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_download_bytes(hash,size,iptype,utype);
	return -1;
}
int DownloadListManager::on_connection(const hash_t& hash,int ctype,bool succeed)
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_connection(hash,ctype,succeed);
	return -1;
}
int DownloadListManager::on_share_bytes(const hash_t& hash,int size,int iptype)
{
	hash_t plhash;
	if(!hash.url2hash_to_urldlhash(plhash))
		return -1;
	DLIter it = m_dlmap.find(plhash);
	if(it!=m_dlmap.end())
		return it->second->on_share_bytes(hash,size,iptype);
	return -1;
}

void DownloadListManager::on_second()
{
	list<hash_t> ls_del;
	for(DLIter it=m_dlmap.begin();it!=m_dlmap.end();++it)
	{
		it->second->on_second();
		if(it->second->check_auto_close())
			ls_del.push_back(it->first);
	}
	for(list<hash_t>::iterator it2=ls_del.begin();it2!=ls_del.end();++it2)
	{
		this->close_downloadlist(*it2);
	}
}
//void DownloadList::on(DownloadManagerListener::DLCreate,const DownloadInfo& inf)
//{
//}
//void DownloadList::on(DownloadManagerListener::DLStart,const DownloadInfo& inf)
//{
//}
//void DownloadList::on(DownloadManagerListener::DLStop,const DownloadInfo& inf)
//{
//}
//void DownloadList::on(DownloadManagerListener::DLDownloading,const DownloadInfo& inf)
//{
//}
//void DownloadList::on(DownloadManagerListener::DLBlockTable,const hash_t& hash,const BlockTable& inf)
//{
//}
//void DownloadList::on(DownloadManagerListener::DLBlockDone,const hash_t& hash,int index)
//{
//}
//void DownloadList::on(DownloadManagerListener::DLFileDone,const hash_t& hash)
//{
//}
