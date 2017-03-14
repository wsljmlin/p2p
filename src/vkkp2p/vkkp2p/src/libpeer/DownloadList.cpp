#include "DownloadList.h"
#include "DownloadManager.h"
#include "Setting.h"
#include "Util.h"
#include "MessagePusher.h"
#include "Statistician.h"
#ifdef SM_VOD
#include "FileStorage.h"
#endif

#ifdef SM_DBG
#define LOGFILE "log.out"
#define DOWNLOADLIST_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "DownloadList",__LINE__, ##arg)
#else
#define DOWNLOADLIST_PRT(fmt, arg...)
#endif

#ifdef SM_VOD
#define DOWNLOAD_ACTIVE_NUM_LIVE 2
#endif


DownloadList::DownloadList(void)
: m_token_i(0)
, m_isUseClientSNOnly(false)
, m_bopenbytracker(false)
, m_need_download_updateing(false)
, m_download_amount(0)
, m_http_max_num(0)
, m_begin_time(0)
, m_curr_tick(0)
, m_last_update_tick(0)
, m_last_search_tick(0)
, m_fini_num(0)
, m_need_ref(0)
, m_autoclose(false)
, m_last_need_end_tick(0)
, m_last_update_pl_time(0)
#ifdef SM_VOD
,m_playlist_updatecompleted(false)
//,m_main_download(true)
#endif
{
	m_export_logh = NULL;
	m_httpget.add_listener(this);
}

DownloadList::~DownloadList(void)
{

}
DownloadList* DownloadList::open(const hash_t& hash,const string& url,const string& name,bool autoclose,bool bopenbytracker)
{
	if(url.empty())
		return NULL;
	DownloadList* pl = new DownloadList();
	if(!pl) return NULL;
	pl->m_data.plname = name;
	pl->m_autoclose = autoclose;
	pl->m_bopenbytracker = bopenbytracker;
	pl->set_info(hash,url);
	pl->start();
	DOWNLOADLIST_PRT(" open!\n");
	return pl;
}

#ifdef SM_VOD
DownloadList* DownloadList::open(const hash_t& hash,const string& url,const string& name,bool autoclose,int playtype,bool bopenbytracker)
{
	if(url.empty())
		return NULL;
	DownloadList* pl = new DownloadList();
	if(!pl) return NULL;
	pl->m_data.plname = name;
	pl->m_autoclose = autoclose;
	pl->m_bopenbytracker = bopenbytracker;
	pl->m_playtype = playtype;
	pl->set_info(hash,url);
	pl->start();
	DOWNLOADLIST_PRT(" open, playtype=%d!\n", playtype);
	return pl;
}

DownloadList::FileNode* DownloadList::create_download(FileNode* fn,bool check_next,bool is_token,bool use_url, int playtype)
{
	assert(FN_IDLE == fn->state);
	char hashbuf[48];

	if(!check_memcache_ok_to_start(fn->hash))
		return NULL;
	fn->hash.to_string(hashbuf,48);
	//string str = hashbuf;
	//string path = SettingSngl::instance()->get_cache_path() + str.substr(2) + ".ts";
	DOWNLOADLIST_PRT("pre_filename=%s url=%s\n", m_data.pre_filename.c_str(), Util::url_get_name(fn->url).c_str());
	string path = SettingSngl::instance()->get_cache_path() + m_data.pre_filename + Util::url_get_name(fn->url);
	
	int ret=DownloadManagerSngl::instance()->create_download(fn->hash,playtype,path,use_url?fn->url:"",
		m_http_max_num,0,0,SettingSngl::instance()->get_block_size(),FTYPE_VOD,true,SettingSngl::instance()->get_cache_filetype());

	fn->local_path = path; //add for export playlist

	if(1==ret && check_next)
	{
		fn->state = FN_FINISHED;
		//find_next 从m_token_hash之后开始找,在此不要更新token,如果要更新也要取得原token的源列表
		return check_create_next_download(fn->hash,is_token,use_url);
	}
	else if(0==ret)
	{
		fn->read_ref++;
		DownloadManagerSngl::instance()->read_refer(fn->hash);
		fn->state = FN_DOWNING;
		m_download_amount++;
		//创建任务时即添加http和p2p源
		DownloadManagerSngl::instance()->add_source_list(fn->hash,m_p2p_sources);
		if(use_url)
			DownloadManagerSngl::instance()->http_add_source(fn->hash,fn->url,true);
		if(is_token||m_token_hash.empty())
		{
			set_token(fn);
		}
		return fn;
	}
	else
	{
		char logbuf[1024];
		string logpath = SettingSngl::instance()->get_log_path()+"error_dl.log";
		sprintf(logbuf,"***create_download() fail! (i:%d,hash:%s,path:%s, url:%s) ",fn->i,hashbuf,path.c_str(),fn->url.c_str());
		Util::write_log(logbuf,logpath.c_str());
	}
	return NULL;
}

int DownloadList::get_downloadcachetime(int& time) {
	/* should skip the begining of not cached time which may be deleted by autocache when storage is not enough*/
	FileNode* fn = NULL;
	for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it) {
		fn = &(*it);
		//if(FileStorageSngl::instance()->get_readyinfo(fn->hash))
		if( fn->state!=FN_DOWNING)
			time += (int)1000*fn->time;
		else break;
	}
	
	return 0;
}

int DownloadList::get_fileurl(const hash_t& hash, string& url) {
	FileNode* fn = find(hash);
	if(fn) url=fn->url;
	else url="";
	return 0;
}

int DownloadList::get_filepath(const hash_t& hash, string& path) {
	FileNode* fn = find(hash);
	if(fn) path=fn->local_path;
	else path="";
	return 0;
}

#endif
void DownloadList::close()
{
	stop();
	clear_cache();
	m_data.fn_list.clear();
	source_clear_all();
	m_httpget.remove_listener(this);
	m_httpget.close();
	delete this;
}
int DownloadList::get_downloadlist(list<string>& ls)
{
	//FileNode* fn = NULL;
	//for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	//{
	//	fn = &(*it);
	//	if(fn->is_line_no)
	//		ls.push_back(fn->local_url);
	//}
	ls = m_data.origi_downlist;
#if 0
	list<string>::iterator it=ls.begin();
	DOWNLOADLIST_PRT("##############\n");
	for(; it != ls.end(); it++) {
		DOWNLOADLIST_PRT("%s\n", (*it).c_str());
	}
#endif
	return 0;
}
int DownloadList::need_download(const hash_t& hash)
{
	m_need_ref++;
	FileNode* fn = find(hash);
	if(!fn)
		return -1;
	m_curr_need_hash = hash;
#ifdef SM_VOD
	if(PLAYTYPE_VOD!=m_playtype)
		check_http_resume();
#endif /* end of  SM_VOD */
	if(FN_IDLE==fn->state)
	{
		DOWNLOADLIST_PRT("need to download \n");
		//停止其它下载此文件
		m_need_download_updateing = true;
		for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
		{
			if(hash == (*it).hash)
				continue;
			if(FN_DOWNING == (*it).state)
			{
				read_release(&(*it));
			}
		}
		m_need_download_updateing = false;
		//assert(0==m_download_amount); //可能其它正在http请求中已至停止不了
		DEBUGMSG("#need_download update...(%d) !\n",fn->i);
#ifdef SM_VOD
		//assert(m_main_download==true);
		create_download(fn,true,true,true,m_playtype);
#else
		create_download(fn,true,true,true);
#endif
	}
	else
	{
		if(1!=SettingSngl::instance()->get_cache_flag())
			check_create_download();
	}
	//在此别考虑清掉旧内存缓冲
	//release_old_memcache();
	return 0;
}

int DownloadList::need_download_end(const hash_t& hash)
{
	m_need_ref--;
	m_last_need_end_tick = m_curr_tick;
	return 0;
}
int DownloadList::set_info(const hash_t& hash,const string& url)
{
	//m_data.plhash.set_urldl_string(url.c_str());
	m_data.plhash = hash;
	m_data.plurl = url;

// check session based url

	map<string, string>::iterator it = SettingSngl::instance()->m_plurls.find(url);
	if ( it != SettingSngl::instance()->m_plurls.end() )
	{
		m_data.plurl_org  = it->second;
		m_data.plurl_real = it->second;
		DEBUGMSG("#*** set info set real plurl (%s) \n", m_data.plurl_real.c_str() );
	}
// end check

	char buf[128];
	m_data.plhash.to_string(buf,128);
	m_data.plstrhash = buf;
	m_data.pre_filename = buf;
	m_data.pre_filename += "_";
	char pll_name[256];
	sprintf(pll_name,"playlist_%s.txt",buf);
	m_data.plpath = SettingSngl::instance()->get_log_path() + pll_name;
	return 0;
}
int DownloadList::start()
{
	m_http_max_num = SettingSngl::instance()->get_downloadhttpi_cnns();

	m_token_hash.clear();
	if(1==GetPrivateProfileIntA("sn","snonly",0,SettingSngl::instance()->get_confini_path().c_str()))
		m_isUseClientSNOnly = true;
	m_begin_time = (unsigned int)time(NULL);
	MessagePusherSngl::instance()->on_downloadlist_start(m_data.plurl);
#ifdef SM_VOD
	if(true!=m_playlist_updatecompleted)
		update_downloadlist();
	//在第一次获取到playlist后再search，否则找不到token_hash赋搜索结果
	TrackerSngl::instance()->PTL_ReportShareFile(m_data.plhash, (int)m_playtype, 0);
	TrackerSngl::instance()->PTL_ReportStartDownloadList(m_data.plhash,m_playtype,m_data.plurl);
#else
	update_downloadlist();
	TrackerSngl::instance()->PTL_ReportShareFile(m_data.plhash,0);
	TrackerSngl::instance()->PTL_ReportStartDownloadList(m_data.plhash,m_data.plurl);
#endif /* end of SM_VOD */
	
#ifdef SM_DBG
	char buf[45];
	m_data.plhash.to_string(buf,45);
	DOWNLOADLIST_PRT("report to tracker:%s\n", buf );
#endif
	DOWNLOADLIST_PRT("download start!\n");
	return 0;
}
void DownloadList::stop()
{
	DOWNLOADLIST_PRT("stop..............!\n");
	StatisticianSngl::instance()->OnConnectionSpeed(1,(unsigned int)m_stat.speed.get_speed(),m_stat.speed.get_sec_amount(),0,0,true);
	m_need_download_updateing = true;
	for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
		read_release(&(*it));
	}
	m_need_download_updateing = false;
	//TrackerSngl::instance()->PTL_ReportStopDownloadFile(m_data.plhash);
	ptl_report_downladfileinfo(4);
#ifdef SM_VOD
	TrackerSngl::instance()->PTL_ReportRemoveFile(m_data.plhash, m_playtype);
#else
	TrackerSngl::instance()->PTL_ReportRemoveFile(m_data.plhash);
#endif /* end of SM_VOD */
	MessagePusherSngl::instance()->on_downloadlist_stop(m_data.plurl);
}
void DownloadList::clear_cache()
{
	for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
#ifdef SM_VOD
		/* live will delete file and report to tracker. But vod will not delete file, only delete download, but not report to tracker */
		if(PLAYTYPE_VOD!=m_playtype) {
			DownloadManagerSngl::instance()->delete_file((*it).hash,true, true);
		} else {
			DownloadManagerSngl::instance()->delete_file((*it).hash,false, false); 
		}
#else
		DownloadManagerSngl::instance()->delete_file((*it).hash,true);
#endif /* end of SM_VOD */
	}
}
DownloadList::FileNode* DownloadList::find(const hash_t& hash)
{
	if(hash.empty())
		return NULL;
	for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
		if(hash == (*it).hash)
			return &(*it);
	}
	return NULL;
}
DownloadList::FileNode* DownloadList::create_download(FileNode* fn,bool check_next,bool is_token,bool use_url)
{
	assert(FN_IDLE == fn->state);
	char hashbuf[48];

	if(!check_memcache_ok_to_start(fn->hash))
		return NULL;
	
	fn->hash.to_string(hashbuf,48);
	//string str = hashbuf;
	//string path = SettingSngl::instance()->get_cache_path() + str.substr(2) + ".ts";
	DOWNLOADLIST_PRT("pre_filename=%s url=%s\n", m_data.pre_filename.c_str(), Util::url_get_name(fn->url).c_str());
	string path = SettingSngl::instance()->get_cache_path() + m_data.pre_filename + Util::url_get_name(fn->url);
	int ret=DownloadManagerSngl::instance()->create_download(fn->hash,path,use_url?fn->url:"",
		m_http_max_num,0,0,SettingSngl::instance()->get_block_size(),FTYPE_VOD,true,SettingSngl::instance()->get_cache_filetype());

	fn->local_path = path; //add for export playlist

	if(1==ret && check_next)
	{
		fn->state = FN_FINISHED;
		//find_next 从m_token_hash之后开始找,在此不要更新token,如果要更新也要取得原token的源列表
		return check_create_next_download(fn->hash,is_token,use_url);
	}
	else if(0==ret)
	{
		fn->read_ref++;
		DownloadManagerSngl::instance()->read_refer(fn->hash);
		fn->state = FN_DOWNING;
		m_download_amount++;
		//创建任务时即添加http和p2p源
		DownloadManagerSngl::instance()->add_source_list(fn->hash,m_p2p_sources);
		if(use_url)
			DownloadManagerSngl::instance()->http_add_source(fn->hash,fn->url,true);
		if(is_token||m_token_hash.empty())
		{
			set_token(fn);
		}
		return fn;
	}
	else
	{
		char logbuf[1024];
		string logpath = SettingSngl::instance()->get_log_path()+"error_dl.log";
		sprintf(logbuf,"***create_download() fail! (i:%d,hash:%s,path:%s, url:%s) ",fn->i,hashbuf,path.c_str(),fn->url.c_str());
		Util::write_log(logbuf,logpath.c_str());
	}
	return NULL;
}
void DownloadList::read_release(FileNode* fn)
{
	if(fn->read_ref>0)
	{
		fn->read_ref--;
		DownloadManagerSngl::instance()->read_release_i(fn->hash);
		if(FN_MEMCACHE_FINISHED==fn->state)
			fn->state = FN_IDLE;
	}
}
int DownloadList::set_token(FileNode* fn)
{
	assert(m_token_hash != fn->hash);
	m_token_hash = fn->hash;
	m_token_i = fn->i;
	//token的时候一定尝试添加剂一次http源，重复会添加失败的
	//当多HTTP连接并发下载一个任务时，传递HTTP连接会导致某片段的HTTP连接数超量
	DownloadManagerSngl::instance()->http_add_source(fn->hash,fn->url,true); 
	return 0;
}
void DownloadList::check_create_download()
{
#ifdef SM_VOD
	int activedown_maxnum = (PLAYTYPE_LIVE==m_playtype)? 
			DOWNLOAD_ACTIVE_NUM_LIVE:(SettingSngl::instance()->get_downloadlist_active_partnum());
	/* will only download one file at the begining, may be will fast */
	if(m_data.fini_i_ls.size()<2) 
		activedown_maxnum = 1;
	
	if(m_download_amount>=activedown_maxnum|| m_data.fn_list.empty())
		return;
	int n = activedown_maxnum - m_download_amount;
	
#else
	if(m_download_amount>=SettingSngl::instance()->get_downloadlist_active_partnum() || m_data.fn_list.empty())
		return;
	int n = SettingSngl::instance()->get_downloadlist_active_partnum() - m_download_amount;
#endif /* end of SM_VOD*/

	for(int i=0;i<n;++i)
	{
		FileNode* fn = NULL;
		fn = find(m_curr_need_hash);
		if(NULL==fn)
		{
			FileIter it=m_data.fn_list.begin();
			//开启倒数第3个任务
			//unsigned int n = m_data.fn_list.size();
			//if(n>3) n-=3;
			//unsigned int i = 1;
			//for(;it!=m_data.fn_list.end() && i<n;++it,++i);
			////test:从第1个开始下载
			fn = &(*it);
			m_curr_need_hash = fn->hash;
		}

		if(fn->state!=FN_IDLE)
			fn = find_next(fn->hash,FN_IDLE,fn->hash);
		if(fn) {
#ifdef SM_VOD
			create_download(fn,true,false,true,m_playtype);
#else
			create_download(fn,true,false,true);
#endif
		}
	}
}
DownloadList::FileNode* DownloadList::check_create_next_download(const hash_t& hash,bool is_token,bool use_url)
{
	//创建下一个下载
	FileNode* fn = find_next(hash,FN_IDLE,hash);
	if(fn) {
#ifdef SM_VOD
		return create_download(fn,true,is_token,use_url,m_playtype);
#else
		return create_download(fn,true,is_token,use_url);
#endif
	}
	return NULL;
}


DownloadList::FileNode* DownloadList::find_next(const hash_t& hash,int state,const hash_t& exclude_hash)
{
	FileNode* fn_first = NULL;
	FileNode* fn_next = NULL;
	FileNode* fn = NULL;
	FileIter it = m_data.fn_list.begin();
	if(!hash.empty())
	{
		for(;it!=m_data.fn_list.end();++it)
		{
			if((*it).hash == hash)
			{
				++it;
				break;
			}
		}
	}
	for(;it!=m_data.fn_list.end();++it)
	{
		fn = &(*it);
		//不是当前服务器列表的不要
		if(FN_IDLE==fn->state && !fn->is_line_no)
			continue;
		if(!fn_first && fn->state==state && fn->hash!=exclude_hash && fn->hash!=hash)
			fn_first = fn;
		if(fn->hash==hash)
		{
			++it;
			break;
		}
	}
	for(;it!=m_data.fn_list.end();++it)
	{
		fn = &(*it);
		//不是当前服务器列表的不要
		if(FN_IDLE==fn->state && !fn->is_line_no)
			continue;
		if(fn->state==state && fn->hash!=exclude_hash)
		{
			fn_next = fn;
			break;
		}
	}
	if(fn_next)
		return fn_next;
	else if(fn_first)
		return fn_first;
	else
		return NULL;
}
int DownloadList::get_idle_num()
{
	int n=0;
	FileIter it;
	//总是从token开始查找起
	for(it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
		if((*it).hash == m_token_hash)
			break;
	}
	for(;it!=m_data.fn_list.end();++it)
	{
		if((*it).state == FN_IDLE)
			++n;
	}
	return n;
}
void DownloadList::on(HttpGetListener::Url302,HttpGet* hg,const string& newurl)
{
	//20160126:
	m_data.plurl_real = newurl;
	DEBUGMSG("#*** http re location set real plurl (%s) \n", m_data.plurl_real.c_str() );
}
void DownloadList::on(HttpGetListener::Response,HttpGet* hg,int result,const string& save_path,const char* orig_rsp,int len)
{
	char * rsp = ( char *) orig_rsp;
	if ( SettingSngl::instance()->get_playlist_timeshift() != 0
		|| SettingSngl::instance()->get_playlist_delaynumber() != 0 ) {
		rsp = playlist_preprocess( orig_rsp );
	}

	list<string> ls;
	list<string>::iterator it;
	FileIter fit;
	bool bfind = false;
	if(-1==result)
	{
		////test:
		//ls.push_back("http://127.0.0.1:7081/vttv/33e9505d12942e8259a3c96fb6f88ed325b95797.rmvb");
		//ls.push_back("http://127.0.0.1:7081/vttv/bdac5c0e344778158017e063ac44ed89e492785f.rmvb");
		//ls.push_back("http://127.0.0.1:7081/vttv/75da2e7bee81a9efc720602e412e267a957bcbc5.mp4");
		Util::write_log("***DownloadList::on(HttpGetListener::Response) error !",(SettingSngl::instance()->get_log_path()+"error_dl.log").c_str());
		return;
	}
	string pre_url = m_data.plurl;
	string root_url = m_data.plurl;

	if ( m_data.plurl_real.length() != 0 )
        {
                pre_url = m_data.plurl_real;
                root_url= m_data.plurl_real;
        }

	string str;
	int pos = (int)pre_url.rfind('/');
	if(pos>0)
		pre_url.erase(pos+1);
	pos = (int)root_url.find("/",8);
	if(pos>0)
		root_url = root_url.substr(0,pos);

#ifdef SM_VOD
	m_playlist_updatecompleted = true;
#endif /* end if SM_VOD */

	string furl;
	////test:
	m_data.origi_downlist.clear();
	//Util::get_stringlist_from_file(save_path,m_data.origi_downlist);
	Util::get_stringlist_from_string(rsp,m_data.origi_downlist);
	for(it=m_data.origi_downlist.begin();it!=m_data.origi_downlist.end();++it)
	{
		furl=*it;
		if(!furl.empty() && furl.at(0)!='#')
		{
			if(furl.find("http://")!=0)
			{
				if(furl.length()>2 && furl.substr(0,2)=="./")
					furl = furl.substr(2);
				if(furl.at(0)=='/')
					str = root_url+furl;
				else
					str = pre_url + furl;
				ls.push_back(str);
				*it = str;
			}
			else
			{
				ls.push_back(furl);
				*it = furl;
			}
		}
	}
	if(ls.empty())
		return;
	
	if(m_data.fn_list.empty())
		source_search();

	m_data.m_playlist_first_hash.set_url2_string(m_data.plurl.c_str(),ls.front().c_str());

	unsigned int cache_num = 2*ls.size()+1;
	if(cache_num<(unsigned int)SettingSngl::instance()->get_diskcache_playlist_num())
		cache_num = (unsigned int)SettingSngl::instance()->get_diskcache_playlist_num();
	//清掉旧连接
	for(fit=m_data.fn_list.begin();fit!=m_data.fn_list.end();)
	{
		bfind = false;
		for(it=ls.begin();it!=ls.end();++it)
		{
			if(*it == (*fit).url) //1.考虑忽略?号后面
			{
				bfind = true;
				break;
			}
		}
		if(bfind)
		{
			//从ls中清除
			ls.erase(it);
			++fit;
		}
		else
		{
			(*fit).is_line_no = false;
			if(m_data.fn_list.size()>cache_num)
			{
				read_release(&(*fit));
#ifdef SM_VOD
				/* this will only live will called */
				if(0==DownloadManagerSngl::instance()->delete_file((*fit).hash,true, true)) 
#else
				if(0==DownloadManagerSngl::instance()->delete_file((*fit).hash,true)) //如果播放器正在获取数据，这里将删除不成功
#endif /* end of SM_VOD */
					fit = m_data.fn_list.erase(fit);
				else
					++fit;
			}
			else
				++fit;
		}
	}
	if(ls.empty())
	{
		DEBUGMSG("*** no new playlist file url!!! \n");
	}
	else
	{
		m_last_update_pl_time = time(NULL);
	}
	FileNode fn;
	char buf[1024];
	char strhash[48];
	string url_name;
	int http_port = (int)SettingSngl::instance()->get_http_port();
	for(it=ls.begin();it!=ls.end();++it)
	{
		fn.url = *it;
		url_name = Util::url_get_name(fn.url);
		fn.hash.set_url2_string(m_data.plurl.c_str(),url_name.c_str());  //this to adapt to format, such as letv, to keep the resource is the only one
		//fn.hash.set_url2_string(m_data.plurl.c_str(),fn.url.c_str()); //2.考虑忽略?号后面
		DOWNLOADLIST_PRT("--------new file %s:%s\n", m_data.plurl.c_str(), url_name.c_str());
		//local_urli
		fn.hash.to_string(strhash,48);
		fn.i = m_data.file_i++;
		//url_name = Util::url_get_name(fn.url);
		sprintf(buf,"http://127.0.0.1:%d/playlist/file/%s/%d__%s",http_port,strhash,fn.i,url_name.c_str());
		//sprintf(buf,"/playlist/file/%s/%d__%s",strhash,fn.i,url_name.c_str());
		fn.local_url = buf;
		fn.is_line_no = true;
		m_data.fn_list.push_back(fn);
#ifdef SM_DBG
		char buftmp[512];
		sprintf(buftmp, " [%s:%d] local_url: %s , url: %s", __FUNCTION__, __LINE__, fn.local_url.c_str(), fn.url.c_str());
		Util::write_log(buftmp, LOGFILE);
#endif
	}

	DOWNLOADLIST_PRT("origilistsize=%d fnlistsize=%d\n", m_data.origi_downlist.size(), m_data.fn_list.size());
	//save playlist T.B.D
	/*
	string logpath =  "port.txt";
	FILE *fp = fopen(logpath.c_str(),"w");
	if(!fp) {
		SM_PRT("open file failed\n");
	}
	fwrite(buftmp,strlen(buftmp),1,fp);
	fclose(fp);*/
	
	//更改源m3u8列表
	for(it=m_data.origi_downlist.begin();it!=m_data.origi_downlist.end();++it)
	{
		string& s1=*it;
		if(!s1.empty() && s1.at(0)!='#')
		{
			for(fit=m_data.fn_list.begin();fit!=m_data.fn_list.end();++fit)
			{
				if(s1==(*fit).url) //3.考虑忽略?问后面
				{
					s1 = (*fit).local_url;
					break;
				}
			}
		}
		
	}
	check_create_download();

#ifdef SM_VOD
	/* get duation time for every slice */
	for(it=m_data.origi_downlist.begin(),fit=m_data.fn_list.begin() ;it!=m_data.origi_downlist.end(),fit!=m_data.fn_list.end();++it) {
		string& s1=*it;
		if(s1.at(0)=='#' && s1.at(4)=='I') {//#EXTINF:7, 
			string t1=s1.substr(8,s1.find(",")-8);
			char *pEnd;
			(*fit).time = strtof(t1.c_str(), &pEnd);
			//DOWNLOADLIST_PRT("time=%s float=%f old=%s\n", t1.c_str(),(*fit).time, s1.c_str());
			fit++;
		}
	}
#endif /* end of SM_VOD */
}

void DownloadList::source_clear_all()
{
	for(SourceIter it=m_p2p_sources.begin();it!=m_p2p_sources.end();++it)
		delete *it;
	m_p2p_sources.clear();
}
void DownloadList::source_search()
{
	if(m_isUseClientSNOnly)
		return;
	
#ifdef SM_VOD
	if(PLAYTYPE_VOD!=m_playtype) {
		TrackerSngl::instance()->PTL_RequestFileSource(m_data.plhash, m_playtype);
	}
#else
	TrackerSngl::instance()->PTL_RequestFileSource(m_data.plhash);
#endif
	m_last_search_tick = m_curr_tick;
}



void DownloadList::update_downloadlist()
{
	if(m_last_update_tick && m_last_update_tick+2>m_curr_tick)
		return;
	m_last_update_tick = m_curr_tick;

        if ( m_data.plurl_real.length() == 0  )
                m_httpget.set_url(m_data.plurl,m_data.plpath);
        else {
		DEBUGMSG("#*** update list from real url (%s) \n", m_data.plurl_real.c_str() );
                m_httpget.set_url(m_data.plurl_real,m_data.plpath);
        }
	DOWNLOADLIST_PRT("url=%s(%s)\n", m_data.plurl.c_str(), m_data.plurl_real.c_str());
        m_httpget.open_request();
}

#ifdef SM_VOD
void DownloadList::update_downloadlist_ex()
{
	
}
#endif /* end of SM_VOD */

int DownloadList::on_response_source(PTL_P2T_ResponseFileSource& rsp)
{
	////todo:返回源时再设置使用http连接数其实有问题
	//if(SettingSngl::instance()->get_downloadhttpi_cnns()<0)
	//	m_http_max_num = rsp.urlflag;

	//添加源到列表中
	list<SourceInfo *> ls_src;
	bool bfind = false;
	DOWNLOADLIST_PRT("receive p2psources:%d\n", rsp.num);
	for(uint32 i=0; i<rsp.num; i++)
	{
		bfind = false;
		for(SourceIter it=m_p2p_sources.begin();it!=m_p2p_sources.end();++it)
		{
			if((*it)->source.sessionID==rsp.peers[i].sessionID)
			{
				bfind = true;
				(*it)->source = rsp.peers[i]; //更新源,可能端口改变等
				break;
			}
		}
		if(!bfind)
		{
			SourceInfo *inf = new SourceInfo();
			inf->source = rsp.peers[i];
			m_p2p_sources.push_back(inf);
			ls_src.push_back(inf);
			DOWNLOADLIST_PRT("push source SessionID=%d\n", inf->source.sessionID);
		}
	}
	if(!ls_src.empty())
	{
		//每个活动任务都添加一次源
		for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
		{
			if(FN_DOWNING == (*it).state)
			{
				DOWNLOADLIST_PRT("will add source list in DownloadManger for file(%s)!!\n", (*it).local_url.c_str());
				DownloadManagerSngl::instance()->add_source_list((*it).hash,ls_src);
			}
		}
	}
	return 0;
}
void DownloadList::on_tracker_connected()
{
#ifdef SM_DBG	
		char buf[45];
		m_data.plhash.to_string(buf,45);
		DOWNLOADLIST_PRT("report to tracker:%s\n", buf );
#endif
#ifdef SM_VOD
	TrackerSngl::instance()->PTL_ReportShareFile(m_data.plhash,(int)m_playtype,0);
#else
	TrackerSngl::instance()->PTL_ReportShareFile(m_data.plhash,0);
#endif
	source_search();
}
void DownloadList::on_file_done(const hash_t& hash,char done_type)
{
	FileNode* fn = find(hash);
	if(fn)
	{
		m_data.fini_num++;
		m_data.fini_i_ls.push_back(fn->i);
		if(FN_DOWNING == fn->state)
		{
			//当在download执行hashmanager::check_filehash()后至downloadmamager::on_file_done()过程中没被release_i()过或者没被delete过时。
			//就会先到此再到on_file_stop()
			m_download_amount--; 
		}
		if(1==done_type)
			fn->state = FN_FINISHED;
		else
			fn->state = FN_MEMCACHE_FINISHED;
		m_fini_num++;
		//if(1==m_fini_num)
		//	TrackerSngl::instance()->PTL_ReportShareFile(m_data.plhash,0);
	}
	else
	{
		//唯一可能就是在hashmanager::未执行downloadmamager::on_file_done()时，就被调用了删除任务
		char hashbuf[48];
		hash.to_string(hashbuf,48);
		char logbuf[1024];
		string logpath = SettingSngl::instance()->get_log_path()+"error_dl.log";
		sprintf(logbuf,"***DownloadList::on_file_stop() error not find! (fns:%d,hash:%s) ",m_data.fn_list.size(),hashbuf);
		Util::write_log(logbuf,logpath.c_str());
		//assert(false);
	}
#ifdef SM_VOD
	if(PLAYTYPE_VOD!=m_playtype)
		check_http_pause();
#endif /* end of  SM_VOD */
}

void DownloadList::on_file_stop(const hash_t& hash,void* dl)
{
	//先stop再到finish
#ifdef SM_VOD
	/* only live mode need to update playlist timely */
	if(PLAYTYPE_VOD!=m_playtype)
		if(get_idle_num()<=3)
			update_downloadlist();
#else
	if(get_idle_num()<=3)
		update_downloadlist();
#endif /* end of SM_VOD */
	FileNode* fn = find(hash);
	if(fn)
	{
		//可能已经on_file_done
		if(FN_DOWNING == fn->state)
		{
			m_download_amount--;
			fn->state = FN_IDLE;
		}
		//如果是内存cache，延后release
		if(fn->state != FN_MEMCACHE_FINISHED)
		{
			//如果是DownloadList::主动执行的release(),会在这里重入read_release
			read_release(fn);
		}

		if(!m_need_download_updateing)
		{
#ifdef SM_VOD
			int activedown_maxnum=(PLAYTYPE_LIVE==m_playtype)? 
				DOWNLOAD_ACTIVE_NUM_LIVE:(SettingSngl::instance()->get_downloadlist_active_partnum());
			if(m_download_amount<activedown_maxnum)
#else
			if(m_download_amount<SettingSngl::instance()->get_downloadlist_active_partnum())
#endif 
				check_create_next_download(hash,m_token_hash==hash,true); 
			else if(m_token_hash==hash && m_download_amount>0)
			{
				//如果不用新建下载，找一个下载传递token
				fn = find_next(hash,FN_DOWNING,hash);
				if(fn)
					set_token(fn); 
			}
		}
	}
	else
	{
		char hashbuf[48];
		hash.to_string(hashbuf,48);
		char logbuf[1024];
		string logpath = SettingSngl::instance()->get_log_path()+"error_dl.log";
		sprintf(logbuf,"***DownloadList::on_file_stop() error not find! (fns:%d,hash:%s) ",m_data.fn_list.size(),hashbuf);
		Util::write_log(logbuf,logpath.c_str());
		//assert(false);
	}
	if(0==m_download_amount)
		m_token_hash.clear();
	
}
int DownloadList::on_peer_reattach(const hash_t& src_hash,Peer* peer,int ctype)
{
	//目前由于reattach前先创建任务，并且创建任务时可能已经开始创建所有源的P2P连接，所以此连接reattach后极可能产生重复连接，所以暂时先取消支持
	if(1) return -1;

	if(m_need_download_updateing)
		return -1;
	FileNode* fn = NULL;
#ifdef SM_VOD
	int activedown_maxnum=(PLAYTYPE_LIVE==m_playtype)? 
				DOWNLOAD_ACTIVE_NUM_LIVE:(SettingSngl::instance()->get_downloadlist_active_partnum());
	if(m_download_amount>activedown_maxnum)
#else
	if(m_download_amount>SettingSngl::instance()->get_downloadlist_active_partnum())
#endif /* end of SM_VOD */
	{
		fn = find_next(src_hash,FN_DOWNING,src_hash);
		if(fn)
		{
			if(0==DownloadManagerSngl::instance()->on_attach_peer(fn->hash,peer,ctype))
				return 0;
		}
	}
	else
	{
		fn = check_create_next_download(src_hash,false,true); //允许创建时开始HTTP连接
		if(fn)
		{
			if(0==DownloadManagerSngl::instance()->on_attach_peer(fn->hash,peer,ctype))
				return 0;
		}
	}
	return -1;
}
int DownloadList::on_http_peer_reattach(const hash_t& src_hash,HttpPeer* peer)
{
	//如果支持多并发，在此不使用reattach
	if(m_need_download_updateing || SettingSngl::instance()->get_downloadlist_active_partnum()>1)
		return -1;
	FileNode* fn = NULL;
#ifdef SM_VOD
	int activedown_maxnum=(PLAYTYPE_LIVE==m_playtype)? 
				DOWNLOAD_ACTIVE_NUM_LIVE:(SettingSngl::instance()->get_downloadlist_active_partnum());
	if(m_download_amount>activedown_maxnum)
#else

	if(m_download_amount>SettingSngl::instance()->get_downloadlist_active_partnum())
#endif /* end of SM_VOD */
	{
		fn = find_next(src_hash,FN_DOWNING,src_hash);
		if(fn)
		{
			if(0==DownloadManagerSngl::instance()->on_attach_http_peer(fn->hash,peer,fn->url))
				return 0;
		}
		else
		{
			DEBUGMSG("#*** attach_http_peer failed! no next download....(%d) \n",m_download_amount);
		}
			
	}
	else
	{
		fn = check_create_next_download(src_hash,false,false);
		if(fn)
		{
			if(0==DownloadManagerSngl::instance()->on_attach_http_peer(fn->hash,peer,fn->url))
				return 0;
			else
			{
				DEBUGMSG("#*** create_download() and attach_http_peer failed!\n");
			}
		}
		else
		{
			DEBUGMSG("#*** attach_http_peer failed! playlist end!....\n");
		}
	}
	return -1;
}

int DownloadList::on_peer_connect_failed(const hash_t& hash,unsigned int sessionID) //连接失败计数，失败多的丢弃源
{
	for(SourceIter it=m_p2p_sources.begin();it!=m_p2p_sources.end();++it)
	{
		if((*it)->source.sessionID == sessionID)
		{
			SourceInfo* inf = *it;
			inf->connectFailTimes++;
			if(inf->connectFailTimes>20) //连断20次连接失败，放弃源
			{
				delete inf;
				m_p2p_sources.erase(it);
			}
			break;
		}
	}
	return 0;
}
int DownloadList::on_peer_req_nodata(const hash_t& hash,unsigned int sessionID) //一次连接完成，从连接中获取不到任何数据
{
	for(SourceIter it=m_p2p_sources.begin();it!=m_p2p_sources.end();++it)
	{
		if((*it)->source.sessionID == sessionID)
		{
			SourceInfo* inf = *it;
			inf->noDataTimes++;
			if(inf->noDataTimes>10) //连断10次连接都取不到任务数据，放弃源
			{
				delete inf;
				m_p2p_sources.erase(it);
			}
			break;
		}
	}
	return 0;
}
int DownloadList::on_peer_req_havedata(const hash_t& hash,unsigned int sessionID) //一次连接完成，从连接中成功获取到过数据
{
	for(SourceIter it=m_p2p_sources.begin();it!=m_p2p_sources.end();++it)
	{
		if((*it)->source.sessionID == sessionID)
		{
			SourceInfo* inf = *it;
			inf->noDataTimes=0;
			inf->connectFailTimes=0; //表示有连接成功
			break;
		}
	}
	return 0;
}
void DownloadList::release_old_memcache()
{
	//释放前面的memcache
	for(FileIter it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
		if(m_curr_need_hash == (*it).hash)
			break;
		if(FN_MEMCACHE_FINISHED==(*it).state)
			read_release(&(*it));
	}
}
bool DownloadList::check_memcache_ok_to_start(const hash_t& hash)
{
	if(1==SettingSngl::instance()->get_cache_flag())
		return true;
	//只要有memcache类型的，都判断，判断hash是在当前下载窗口内的才允许下载
	int win_size = SettingSngl::instance()->get_memcache_playlist_win();
	int i=0;
	FileIter it=m_data.fn_list.begin();
	for(;it!=m_data.fn_list.end();++it)
	{
		if(m_curr_need_hash == (*it).hash)
			break;
	}
	for(;it!=m_data.fn_list.end()&&i<win_size;++it,++i)
	{
		if(hash == (*it).hash)
			return true;
	}
	DEBUGMSG("# -------------------memcache playlist pause------------------------------------ !\n");
	return false;
}
int DownloadList::on_download_bytes(const hash_t& hash,int size,int iptype,int utype)
{
	m_stat.downBytesPerIPT_B[iptype-IPT_TCP] += size;
	m_stat.downBytesPerUserT_B[utype-UT_CLIENT] += size;
	m_stat.speed.add(size);
	return 0;
}
int DownloadList::on_connection(const hash_t& hash,int ctype,bool succeed)
{
	if(LOCAL_TCP == ctype) ctype = TCP_CONN;
	if(LOCAL_UDP == ctype) ctype = UDP_CONN;
	if(succeed)
	{
		m_stat.connSucceedPerNetT[ctype-TCP_CONN] += 1;
	}
	else
	{
		m_stat.connFailedPerNetT[ctype-TCP_CONN] += 1;
	}
	return 0;
}
int DownloadList::on_share_bytes(const hash_t& hash,int size,int iptype)
{
	m_stat.shareBytesPerIPT_B[iptype-IPT_TCP] += size;
	m_stat.shareSpeed.add(size);
	return 0;
}
void DownloadList::on_second()
{
	m_curr_tick++;

	//统计处理
	m_stat.speed.on_second();
	m_stat.shareSpeed.on_second();
	if(0==m_curr_tick%3)
	{
		DEBUGMSG("#DownloadList:: downn=%d speed = %d KB \n",m_download_amount,(unsigned int)((m_stat.speed.get_speed(5)+512)>>10));
	}
	//每20分钟报一次下载信息
	//test:
	//if(0==m_curr_tick%12)
#ifdef SM_VOD
	if(0==m_curr_tick%300)
#else
	if(0==m_curr_tick%1200)
#endif /* end of SM_VOD */
		ptl_report_downladfileinfo(3);

#ifdef SM_VOD	
	/* every 4 minutes to check download, to avoid download count becomes 1,but nerver add again */
	if(0==m_curr_tick%60)
		if(PLAYTYPE_VOD==m_playtype)
			check_create_download();
#endif

	//1。周期更新playlist表
	//2。检测速度为0一段时间后，中止本下载开始下一个下载
	//3。周期搜索源

	if(m_curr_tick%60==0)
		ontimer_check_download_state();
	if(m_curr_tick%5==0)
	{
#ifdef SM_VOD
		/* only live mode need to update playlist timely */
		if(PLAYTYPE_VOD!=m_playtype)
			if(get_idle_num()==0 || m_last_update_tick + 10< m_curr_tick)
				update_downloadlist();
#else
		//10秒钟必须更新一次
		if(get_idle_num()==0 || m_last_update_tick + 10< m_curr_tick)
			update_downloadlist();
#endif /* end of SM_VOD */
		export_playlist();
	}
	static unsigned int iSearchCycle = 180;
	if(m_curr_tick%5==0)
	{
		iSearchCycle = 180;
		Download* dl = DownloadManagerSngl::instance()->get_download(m_token_hash);
		if(dl)
		{
			//int speed = (int)dl->get_download_speed(5)>>10;
			//DEBUGMSG("speed_KB=%d\n",speed);
			//if(speed < 70)
			//	iSearchCycle = 120;  //0~70 KB 的2分钟搜索一次
			//else if(speed < 120)
			//	iSearchCycle = 300;  //70~120 KB 的5分钟搜索一次
			//else if(speed < 200)
			//	iSearchCycle = 600;  //120~200 KB 的10分钟搜索一次
			//else
			//	iSearchCycle = 1200; //0~70 KB 的20分钟搜索一次

			int num = dl->get_source_num();
			if(num==dl->get_source_num())
				iSearchCycle = 60; //无源1分钟搜索1次
			else if(num>20)
				iSearchCycle = 360;
			//30秒没速度
			if(dl->get_zerospeed_count()>10)
			{
				char hashbuf[48];
				m_token_hash.to_string(hashbuf,48);
				char logbuf[1024];
				string logpath = SettingSngl::instance()->get_log_path()+"error_dl.log";

				FileNode *fn = find(m_token_hash);
				if(fn)
				{
					sprintf(logbuf,"***DownloadList::second() download zerospeed  error (i:%d,hash:%s,url:%s) ",fn->i,hashbuf,fn->url.c_str());
					read_release(fn);
				}
				else
				{
					sprintf(logbuf,"***DownloadList::second() download zerospeed  error (not find token! hash:%s) ",hashbuf);
				}
				Util::write_log(logbuf,logpath.c_str());
			}
		}
		if(m_last_search_tick + iSearchCycle < m_curr_tick)
		{
			source_search();
		}
	}

	//如果fini_i_ls过长，只保留最近100个
#ifdef SM_VOD
	if((PLAYTYPE_LIVE==m_playtype) && (m_data.fini_i_ls.size()>110)) {
		while(m_data.fini_i_ls.size()>100) {
			m_data.fini_i_ls.erase(m_data.fini_i_ls.begin());
		}
	}
#else
	if(m_data.fini_i_ls.size()>110)
	{
		while(m_data.fini_i_ls.size()>100)
		{
			m_data.fini_i_ls.erase(m_data.fini_i_ls.begin());
		}
	}
#endif /* end of SM_VOD */
}
int DownloadList::get_msg_downloadlist_info(MsgDownloadlistInfo_t *inf)
{
#ifdef SM_VOD
	static DLStat_t stat_prev;
	static unsigned int tick_prev = m_curr_tick;
		
	inf->hash = m_data.plhash;
	inf->begintime = m_begin_time;
	inf->downing_num = m_download_amount;
	inf->downSpeed_KB = (unsigned int)((m_stat.speed.get_speed(3)+512)>>10);
	inf->shareSpeed_KB = (unsigned int)((m_stat.shareSpeed.get_speed(3)+512)>>10);
	inf->file_fini_num = m_data.fini_num;
	inf->file_all_num = m_data.file_i;
	inf->fini_i_ls = m_data.fini_i_ls;
	inf->p2psrc_num = m_p2p_sources.size();
	get_connect_num(inf->p2pconn_num,inf->httpconn_num);

	if(m_curr_tick > tick_prev) {
		inf->seconds = m_curr_tick - tick_prev;
		inf->downSizeB = m_stat.speed.get_total() - stat_prev.speed.get_total();
		inf->shareSizeB = m_stat.shareSpeed.get_total() - stat_prev.shareSpeed.get_total(); 
		for(int i=0;i<3;++i) inf->downSizeB_Con[i] = m_stat.downBytesPerIPT_B[i] -  stat_prev.downBytesPerIPT_B[i];
		for(int i=0;i<6;++i) inf->downSizeB_User[i] = m_stat.downBytesPerUserT_B[i]  - stat_prev.downBytesPerUserT_B[i] ;
	} else {
		inf->seconds = m_curr_tick;
		inf->downSizeB = m_stat.speed.get_total();
		inf->shareSizeB = m_stat.shareSpeed.get_total(); 
		for(int i=0;i<3;++i) inf->downSizeB_Con[i] = m_stat.downBytesPerIPT_B[i] ;
		for(int i=0;i<6;++i) inf->downSizeB_User[i] = m_stat.downBytesPerUserT_B[i] ;
	}
	stat_prev = m_stat;
	tick_prev = m_curr_tick;
#else
	inf->hash = m_data.plhash;
	inf->begintime = m_begin_time;
	inf->seconds = m_curr_tick;
	inf->downing_num = m_download_amount;
	inf->downSizeB = m_stat.speed.get_total();
	inf->shareSizeB = m_stat.shareSpeed.get_total();
	inf->downSpeed_KB = (unsigned int)((m_stat.speed.get_speed(3)+512)>>10);
	inf->shareSpeed_KB = (unsigned int)((m_stat.shareSpeed.get_speed(3)+512)>>10);
	inf->file_fini_num = m_data.fini_num;
	inf->file_all_num = m_data.file_i;
	inf->fini_i_ls = m_data.fini_i_ls;
	inf->p2psrc_num = m_p2p_sources.size();
	get_connect_num(inf->p2pconn_num,inf->httpconn_num);
	
	for(int i=0;i<3;++i) inf->downSizeB_Con[i] = m_stat.downBytesPerIPT_B[i] ;
	for(int i=0;i<6;++i) inf->downSizeB_User[i] = m_stat.downBytesPerUserT_B[i] ;
#endif /* end of SM_VOD */
	return 0;
}
int DownloadList::get_msg_downloadlist_info2(MsgDownloadlistInfo2_t *inf)
{
	inf->url = m_data.plurl;
	inf->name = m_data.plname;
	inf->strhash = m_data.plstrhash;
	inf->downSpeed_KB = (unsigned int)((m_stat.speed.get_speed(3)+512)>>10);
	inf->shareSpeed_KB = (unsigned int)((m_stat.shareSpeed.get_speed(3)+512)>>10);
	inf->file_fini_num = m_data.fini_num;
	inf->max_i = m_data.file_i-1;
	inf->token_i = m_token_i;
	inf->list_num = m_data.origi_downlist.size();
	inf->last_update_pl_time = m_last_update_pl_time;
	return 0;
}
void DownloadList::ptl_report_downladfileinfo(int flag)
{
	PTL_P2T_ReportDownloadFileInfo inf;
	memset(&inf,0,sizeof(inf));
	//上报速度记录
#ifdef SM_VOD
	static DLStat_t stat_prev;
	static unsigned int ui_cachetime=0;
	inf.flag = flag;
	memcpy(inf.fhash,m_data.plhash.buffer(),HASHLEN);
	if(ui_cachetime < m_stat.speed.get_sec_amount()) {
		inf.size = m_stat.speed.get_total() - stat_prev.speed.get_total();
		inf.speed_KB = (unsigned int)((m_stat.speed.get_speed()+512)>>10);
		inf.downSeconds = m_stat.speed.get_sec_amount() - stat_prev.speed.get_sec_amount();
		int i=0;
		for(i=0;i<5;++i)
		{
			inf.connSucceedPerNetT[i] = m_stat.connSucceedPerNetT[i] - stat_prev.connSucceedPerNetT[i];
			inf.connFailedPerNetT[i] = m_stat.connFailedPerNetT[i] - stat_prev.connSucceedPerNetT[i];
		}
		for(i=0;i<2;++i) {
				inf.shareBytesPerIPT_KB[i] =  (uint32)((m_stat.shareBytesPerIPT_B[i]+512)>>10) - (uint32)((stat_prev.shareBytesPerIPT_B[i]+512)>>10); 
				if(inf.shareBytesPerIPT_KB[i] <  0) inf.shareBytesPerIPT_KB[i] = 0;
		}
		for(i=0;i<3;++i) {
			inf.downBytesPerIPT_KB[i] =  (uint32)((m_stat.downBytesPerIPT_B[i]+512)>>10) - (uint32)((stat_prev.downBytesPerIPT_B[i]+512)>>10); 
			if(inf.downBytesPerIPT_KB[i] < 0) inf.downBytesPerIPT_KB[i] = 0;
		}
		for(i=0;i<6;++i) {
			inf.downBytesPerUserT_KB[i] =  (uint32)((m_stat.downBytesPerUserT_B[i]+512)>>10) - (uint32)((stat_prev.downBytesPerUserT_B[i]+512)>>10); 
			if(inf.downBytesPerUserT_KB[i] < 0) inf.downBytesPerIPT_KB[i]  = 0;
		}
	} else {
		inf.size = m_stat.speed.get_total();
		inf.speed_KB = (unsigned int)((m_stat.speed.get_speed()+512)>>10);
		inf.downSeconds = m_stat.speed.get_sec_amount();
		int i=0;
		for(i=0;i<5;++i)
		{
			inf.connSucceedPerNetT[i] = m_stat.connSucceedPerNetT[i];
			inf.connFailedPerNetT[i] = m_stat.connFailedPerNetT[i];
		}
		for(i=0;i<2;++i)
			inf.shareBytesPerIPT_KB[i] =  (uint32)((m_stat.shareBytesPerIPT_B[i]+512)>>10);
		for(i=0;i<3;++i)
			inf.downBytesPerIPT_KB[i] =  (uint32)((m_stat.downBytesPerIPT_B[i]+512)>>10);
		for(i=0;i<6;++i)
			inf.downBytesPerUserT_KB[i] =  (uint32)((m_stat.downBytesPerUserT_B[i]+512)>>10);
	}
	stat_prev = m_stat;
	ui_cachetime = m_stat.speed.get_sec_amount();
#else
	inf.flag = flag;
	memcpy(inf.fhash,m_data.plhash.buffer(),HASHLEN);
	inf.size = m_stat.speed.get_total();
	inf.speed_KB = (unsigned int)((m_stat.speed.get_speed()+512)>>10);
	inf.downSeconds = m_stat.speed.get_sec_amount();
	int i=0;
	for(i=0;i<5;++i)
	{
		inf.connSucceedPerNetT[i] = m_stat.connSucceedPerNetT[i];
		inf.connFailedPerNetT[i] = m_stat.connFailedPerNetT[i];
	}
	for(i=0;i<2;++i)
		inf.shareBytesPerIPT_KB[i] =  (uint32)((m_stat.shareBytesPerIPT_B[i]+512)>>10);
	for(i=0;i<3;++i)
		inf.downBytesPerIPT_KB[i] =  (uint32)((m_stat.downBytesPerIPT_B[i]+512)>>10);
	for(i=0;i<6;++i)
		inf.downBytesPerUserT_KB[i] =  (uint32)((m_stat.downBytesPerUserT_B[i]+512)>>10);
#endif /* end of SM_VOD */
	
	TrackerSngl::instance()->PTL_ReportDownloadFileInfo(inf);
}

void DownloadList::ontimer_check_download_state()
{
	if(m_data.fn_list.empty())
		return;
	if( m_token_i + m_data.fn_list.size()*2/3 < (m_data.file_i-1))
	{
		char buf[1024];
		sprintf(buf,"#** playlist slow - t/m: %d/%d | %d (%s) ",m_token_i,m_data.file_i-1,m_data.fini_num,m_data.plurl.c_str());
		LOG_playliststate(buf);
	}
	int ts = (int)( time(NULL)-m_last_update_pl_time);
	if(ts>50)
	{
		char buf[1024];
		sprintf(buf,"#** playlist un_update:%d sec (%s) ",ts,m_data.plurl.c_str());
		LOG_playliststate(buf);
	}
		
}

void DownloadList::export_log( const char * msg )
{
	if( m_export_logh == NULL ) {
		if( SettingSngl::instance()->get_export_logfile() == "" )
			m_export_logh = (FILE *) -1;
		else
			m_export_logh = fopen( SettingSngl::instance()->get_export_logfile().c_str(), "w+" );
	}

	printf("EXPORT LOG: %s\n", msg);
	if ( m_export_logh != (FILE *)-1 ) {
		fprintf( m_export_logh, "%s\n", msg );
		fflush( m_export_logh );
	}
}

static int trim_endspace( char * str ) 
{
	int count = 0;
	char * end = str + strlen(str) - 1;
	while( end > str && isspace(*end) ) {
		*end='\0';
		end --;
		count++;
	}
	return count;
}

static int trim_lastline( char * str ) 
{
	int c_space1 = trim_endspace(str);
	char * lastline = strrchr(str, '\n');
	if ( lastline == NULL )
		return c_space1;
	int c_line = (int)strlen(lastline);
	*lastline = '\0';
	return c_space1+c_line + trim_endspace(str);
}
static int trim_lastcomment( char * str ) 
{
//printf("\n---------------------------------\n");
	int c_space1 = trim_endspace(str);
	char * lastline = strrchr(str, '\n');
//printf("remove space %d, and last line is %s\n", c_space1, lastline );
	if ( lastline == NULL || *(lastline+1) != '#' )
		return c_space1;
	int c_line = (int)strlen(lastline);
	*lastline = '\0';
	int c_total1 = c_space1+c_line + trim_lastcomment(str);
	return c_total1 + trim_endspace(str);
}
static char * get_lastline( char * str )
{
	char * lastline = strrchr(str, '\n');
	if ( lastline == NULL )
		return NULL;
	return lastline + 1;
}


char * DownloadList::playlist_preprocess( const char * rsp )
{
	int tf_count = SettingSngl::instance()->get_playlist_timeshift() ;
	int delay    = SettingSngl::instance()->get_playlist_delaynumber();

	string istr = rsp;
	if ( delay > 0 ) {
		char * start = (char *) istr.c_str();

		while( delay > 0 ) {
			trim_lastcomment(start);
			if ( trim_lastline(start) > 0 ) delay--;
			else break;
		}
		trim_lastcomment(start);
		istr.erase( strlen(start) );
	}

	//printf("Playlist Preporcess, istr is -----------------------\n%s\n after trunc\n",istr.c_str());

	if ( tf_count > 0 ) {
		if ( m_timeshift_playlists.size() == 0 )  {
			m_timeshift_playlists.push_back( istr );
		}
		else if ( m_timeshift_playlists.back() != istr ) {
			m_timeshift_playlists.push_back( istr );
		}

		if (  m_timeshift_playlists.size() > (unsigned int) tf_count ) {
			m_timeshift_playlists.pop_front();
		}

		list<string>::iterator it = m_timeshift_playlists.begin();
		istr = *it;
		while( (++it) != m_timeshift_playlists.end() ) {
			char * start = (char *) istr.c_str();
			trim_lastcomment(start);
			istr.erase( strlen(start) );
			char * lastline = get_lastline( start );
			if ( lastline == NULL || *lastline == '\0' )
				continue;
			size_t pos = (*it).find( lastline );
			if ( pos == string::npos )
				continue;
			pos = (*it).find( "\n", pos );
			if ( pos == string::npos || pos < 10 || pos >= (*it).length() -1 ) 
				continue;
			if ( (*it).at( pos - 1 ) == '\r' )
				istr += (*it).substr( pos - 1 );
			else
				istr += (*it).substr( pos );
		}
	}
	//printf("Playlist Preporcess, istr is -----------------------\n%s\n after merged\n",istr.c_str());

	m_playlist = istr;
	return (char *) m_playlist.c_str();
}


void DownloadList::export_playlist()
{
	if(1!=SettingSngl::instance()->get_export_list())
		return;
	int playlist_size = SettingSngl::instance()->get_playlist_size();

	string export_prefix = SettingSngl::instance()->get_export_prefix();
	string export_filename = SettingSngl::instance()->get_export_filename();

	int cache_path_length = (int)SettingSngl::instance()->get_cache_path().length();
	string exportstr ;
	int list_ok_count = 0;
	int down_ok_count = 0;
	int down_count = 0;
	int no_count = 0;
	string cstatus = "-";
	string status;

	list<string>::iterator it;
	FileIter fit;
	string seqline = "";
	FileNode *fn;

	//打印列表前的状态:
	char buf[1024];
	sprintf(buf,"files(%d)--",m_data.fn_list.size());
	status += buf;
	for(fit=m_data.fn_list.begin();fit!=m_data.fn_list.end();++fit)
	{
		if((*fit).hash == m_data.m_playlist_first_hash)
			break;
		fn = &(*fit);
		if(FN_MEMCACHE_FINISHED==fn->state || FN_FINISHED==fn->state)
			cstatus = "F";
		else if(FN_DOWNING==fn->state)
			cstatus = "D";
		else
			cstatus = "N";
		if(fn->hash == m_curr_need_hash)
			cstatus += "* ";
		status += cstatus;

	}
	status += "--";
	for(it=m_data.origi_downlist.begin();it!=m_data.origi_downlist.end();++it)
	{
		string& s1=*it;
		if(!s1.empty() && s1.find("#EXT-X-MEDIA-SEQUENCE:") == 0  ) 
		{
			seqline = s1;
		}
		if(!s1.empty() && s1.at(0)!='#')
		{
			string local_path;
			for(fit=m_data.fn_list.begin();fit!=m_data.fn_list.end();++fit)
			{
				if(s1==(*fit).local_url)
				{
					fn = &(*fit);
					local_path = (*fit).local_path;
					if(FN_MEMCACHE_FINISHED==fn->state || FN_FINISHED==fn->state)
						cstatus = "F";
					else if(FN_DOWNING==fn->state)
						cstatus = "D";
					else
						cstatus = "N";
					if(fn->hash == m_curr_need_hash)
						cstatus += "* ";

					break;
				}
			}
			if (!local_path.empty()) {
#ifndef _WIN32
				struct stat statbuf;
				if ( stat( local_path.c_str() , &statbuf ) == 0 ) {
					cstatus = "O";
					if( down_count == 0 && no_count == 0 ) {
						if ( playlist_size <= 0  || list_ok_count < playlist_size ) {
							exportstr += export_prefix;
							exportstr += local_path.substr(cache_path_length) ;
							exportstr += "\r\n";
						}
						list_ok_count++;
					}
					else {
						down_ok_count++;
					}
				}
				else 
#endif
				{
					down_count++;
					//cstatus = "D";
				}
			}
			else {
				no_count ++;
				//cstatus = "N";
			}
			status += cstatus;
		}
		else if( down_count == 0 && no_count == 0 ) {
			if ( playlist_size <= 0  || list_ok_count < playlist_size ) {
				exportstr += s1;
				exportstr += "\r\n";
			}
		}
	}
	char * ss = (char *) seqline.c_str();
	trim_endspace( ss );
	string logstr = ss;
	logstr += "---";
	logstr += status;
	export_log( logstr.c_str() );
	//export_log( exportstr.c_str() );

	char * str = (char *) exportstr.c_str();
	trim_lastcomment( str );
	if ( m_exportlist.empty() == false && strcmp( str, m_exportlist.c_str() ) == 0 )  {
		printf("export not update\n");
		return;
	}
	//export_log( str );

	m_exportlist = str;

	if ( export_filename.empty() == true ) 
		export_filename = m_data.pre_filename + "pl.m3u8";

	if ( export_filename.find("/") != 0 ) 
		export_filename = SettingSngl::instance()->get_cache_path() + export_filename ;

	//export_log( export_filename.c_str() );

	FILE * efh = fopen( export_filename.c_str(), "w" ) ;
	if ( efh != NULL ) {
		fprintf(efh, "%s", str );
		fclose( efh );
	}

}

//当>=cache_playlist_http_win_up 满时执行httpi,让P2P分享更多数据
//当<cache_playlist_http_win_down 时执行0
void DownloadList::update_http_max_num(int n) 
{
	FileNode * fn = NULL;
	FileIter it;
	for(it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
		fn = &(*it);
		if(fn->state == FN_DOWNING)
		{
			DownloadManagerSngl::instance()->update_http_max_num(fn->hash,n);
		}
	}
}

int DownloadList::get_afterneed_finished_num()
{
	FileNode * fn = NULL;
	int n = 0;
	FileIter it;
	bool bfind = false;
	for(it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
		fn = &(*it);
		if(fn->hash == m_curr_need_hash)
		{
			bfind = true;
		}
		if(bfind && (fn->state == FN_FINISHED|| fn->state ==FN_MEMCACHE_FINISHED) )
		{
			n++;
		}
	}
	return n;
}

void DownloadList::check_http_pause()
{
	if(SettingSngl::instance()->get_playlist_http_pause_win_up()>0)
	{
		if(get_afterneed_finished_num()>=SettingSngl::instance()->get_playlist_http_pause_win_up())
		{
			m_http_max_num = 0;
			update_http_max_num(m_http_max_num);
		}
	}
}
void DownloadList::check_http_resume()
{
	if(SettingSngl::instance()->get_playlist_http_pause_win_up()>0)
	{
		int n = SettingSngl::instance()->get_playlist_http_pause_win_down();
		if(n<1) n=1;
		if(get_afterneed_finished_num()<=SettingSngl::instance()->get_playlist_http_pause_win_down())
		{
			m_http_max_num = SettingSngl::instance()->get_downloadhttpi_cnns();
			update_http_max_num(m_http_max_num);
		}
	}
}
void DownloadList::get_connect_num(int& p2pconn,int& httpconn)
{
	//get_connect_num
	int n1,n2;
	p2pconn = httpconn = 0;
	FileNode * fn = NULL;
	FileIter it;
	for(it=m_data.fn_list.begin();it!=m_data.fn_list.end();++it)
	{
		fn = &(*it);
		if(fn->state == FN_DOWNING)
		{
			if(0==DownloadManagerSngl::instance()->get_connect_num(fn->hash,n1,n2))
			{
				p2pconn += n1;
				httpconn += n2;
			}
		}
	}
}


