#include "LocalService.h"
#include "Interface.h"
#include "Setting.h"
#include "FileStorage.h"
#include "Util.h"
#include "sha1.h"
#include "Tracker.h"
#include "DownloadListManager.h"

LocalService::LocalService(void)
:m_brun(false)
,m_pll_mtime(0)
{
}

LocalService::~LocalService(void)
{
}
int LocalService::run()
{
	if(m_brun)
		return 0;
	m_brun = true;
	activate();
	TimerSngl::instance()->register_timer(this,1,10000);
	return 0;
}
void LocalService::end()
{
	if(!m_brun)
		return;
	m_brun = false;
	TimerSngl::instance()->unregister_all(this);
	wait();
}
int LocalService::work(int e)
{
	DEBUGMSG("#LocalService::work () \n");
	int n=5;
	while(m_brun)
	{
		Sleep(1000);
		if(++n>10)  //每10秒查一次
		{
			n=0;
			check();
		}
	}
	return 0;
}
void LocalService::on_timer(int e)
{
	string path = SettingSngl::instance()->get_preload_playlist();
	if ( path.empty() ) {
		path = SettingSngl::instance()->get_conf_path() + "playlist.inf";
	}
	else if (  path.find("/") != 0 ) {
		path = SettingSngl::instance()->get_conf_path() + path ;
	}

	//string path = SettingSngl::instance()->get_conf_path() + "playlist.inf";
	time_t ct=0,mt=0,at=0;
	Util::get_filetimes(path.c_str(),ct,mt,at);
	int t = (int)mt;
	if(t==m_pll_mtime)
	{
		if(!m_pre_pll.empty())
			check_share_playlist();
		return;
	}
	m_pre_pll.clear();

	Util::get_stringlist_from_file(SettingSngl::instance()->get_conf_path() + "playlist.inf",m_pre_pll);
	//去掉不是http://开头的
	for(list<string>::iterator it=m_pre_pll.begin();it!=m_pre_pll.end();)
	{
		//if((*it).at(0)=='#')
		if(0!=strncmp((*it).c_str(),"http://",7))
			m_pre_pll.erase(it++);
		else if((*it).empty())
			m_pre_pll.erase(it++);
		else {
			string m3u8end = SettingSngl::instance()->get_m3u8_tailer();
                        if( m3u8end.length() > 0 ) {
                                string url = *it;
                                size_t p1 = url.find( m3u8end );
                                if ( p1 != string::npos) {
                                        string clean_url = url.substr(0, p1 );
                                        DEBUGMSG("#*** playlist play knowned plurl (%s) \n", clean_url.c_str() );
                                        SettingSngl::instance()->m_plurls[ clean_url ] = url;
                                        *it = clean_url;
                                }
			}
			++it;
		}
	}
	m_pll_mtime = t;
	if(SettingSngl::instance()->get_is_minor_version())
	{
		check_share_playlist();
	}
	else
	{
		check_open_playlist();
	}
}
int LocalService::get_pll(list<string>& ls)
{
	ls = m_pll;
	return 0;
}
void LocalService::check_share_playlist()
{
	list<string>::iterator it,it2;
	bool bfind=false;
	hash_t hash;
	//如果是次版本，未共享的先共享
	if(SettingSngl::instance()->get_is_minor_version() && TrackerSngl::instance()->is_login())
	{
		//先将不存在的撤销共享
		for(it=m_pll.begin();it!=m_pll.end();)
		{
			bfind = false;
			for(it2=m_pre_pll.begin();it2!=m_pre_pll.end();++it2)
			{
				if((*it)==(*it2))
				{
					bfind = true;
					break;
				}
			}
			if(!bfind)
			{
				hash.set_urldl_string(Util::get_string_index((*it),0,"|").c_str());
#ifdef SM_VOD
				TrackerSngl::instance()->PTL_ReportRemoveFile(hash,FileStorageSngl::instance()->get_fileinfo(hash)->filetype);
#else
				TrackerSngl::instance()->PTL_ReportRemoveFile(hash);
#endif /* end of SM_VOD */
				m_pll.erase(it++);
			}
			else
				++it;
		}
		//将新的共享
		for(it=m_pre_pll.begin();it!=m_pre_pll.end();++it)
		{
			bfind = false;
			for(it2=m_pll.begin();it2!=m_pll.end();++it2)
			{
				if((*it)==(*it2))
				{
					bfind = true;
					break;
				}
			}
			if(!bfind)
			{
				hash.set_urldl_string(Util::get_string_index((*it),0,"|").c_str());
#ifdef SM_VOD
				TrackerSngl::instance()->PTL_ReportShareFile(hash,0,FileStorageSngl::instance()->get_fileinfo(hash)->filetype);
#else
				TrackerSngl::instance()->PTL_ReportShareFile(hash,0);
#endif /* end of SM_VOD */
				m_pll.push_back(*it);
			}	
		}
		m_pre_pll.clear();
	}
}
void LocalService::check_open_playlist()
{
	list<string>::iterator it,it2;
	bool bfind=false;
	//主版本，启动playlist下载
	if(!SettingSngl::instance()->get_is_minor_version())
	{
		//先将不存在的撤销共享
		for(it=m_pll.begin();it!=m_pll.end();)
		{
			bfind = false;
			for(it2=m_pre_pll.begin();it2!=m_pre_pll.end();++it2)
			{
				if((*it)==(*it2))
				{
					bfind = true;
					break;
				}
			}
			if(!bfind)
			{
				DownloadListManagerSngl::instance()->close_downloadlist(Util::get_string_index((*it),0,"|"));
				m_pll.erase(it++);
			}
			else
				++it;
		}
		//将新的共享
		for(it=m_pre_pll.begin();it!=m_pre_pll.end();++it)
		{
			bfind = false;
			for(it2=m_pll.begin();it2!=m_pll.end();++it2)
			{
				if((*it)==(*it2))
				{
					bfind = true;
					break;
				}
			}
			if(!bfind)
			{
				if(-1!=DownloadListManagerSngl::instance()->open_downloadlist(Util::get_string_index((*it),0,"|"),Util::get_string_index((*it),1,"|"),false))
					m_pll.push_back(*it);
			}	
		}
		m_pre_pll.clear();
	}
}


void LocalService::check()
{
	//DEBUGMSG("#LocalService::check()... \n");
	if(SettingSngl::instance()->get_is_minor_version())
		return;
	string path;
	//share
	path = SettingSngl::instance()->get_conf_path() + "sharefile.txt.tmp";
	if(Util::file_exist(path))
	{
		if(0==share_files(path))
			Util::file_delete(path);
	}
	Util::file_rename(SettingSngl::instance()->get_conf_path() + "sharefile.txt",path);
	if(Util::file_exist(path))
	{
		if(0==share_files(path))
			Util::file_delete(path);
	}

	//delete
	path = SettingSngl::instance()->get_conf_path() + "deletefile.txt.tmp";
	if(Util::file_exist(path))
	{
		delete_files(path);
		Util::file_delete(path);
	}
	Util::file_rename(SettingSngl::instance()->get_conf_path() + "deletefile.txt",path);
	if(Util::file_exist(path))
	{
		delete_files(path);
		Util::file_delete(path);
	}

	//download
	path = SettingSngl::instance()->get_conf_path() + "downloadfile.txt.tmp";
	if(Util::file_exist(path))
	{
		download_files(path);
		Util::file_delete(path);
	}
	Util::file_rename(SettingSngl::instance()->get_conf_path() + "downloadfile.txt",path);
	if(Util::file_exist(path))
	{
		download_files(path);
		Util::file_delete(path);
	}
	path = SettingSngl::instance()->get_conf_path() + "downloadfile_test.txt";
	if(Util::file_exist(path))
	{
		download_files(path);
	}
}
int  LocalService::share_files(const string& path)
{
	list<string> ls;
	Util::get_stringlist_from_file(path,ls);
	return share_files_list(ls);
}
void  LocalService::delete_files(const string& path)
{
	list<string> ls;
	list<string>::iterator it;
	Util::get_stringlist_from_file(path,ls);
	string sha1;
	int flag;
	hash_t hash;
	for(it=ls.begin();it!=ls.end();++it)
	{
		sha1 = Util::get_string_index(*it,0,"|");
		Util::string_trim(sha1,' ');
		if(sha1.length()!=40)
			continue;
		flag = atoi(Util::get_string_index(*it,1,"|").c_str());
		hash.set_string_hash(sha1.c_str());
		PIF::instance()->delete_file(hash,flag?true:false);
	}
}
void  LocalService::download_files(const string& path)
{
	//DEBUGMSG("#LocalService::download_files().. \n");
	list<string> ls;
	list<string>::iterator it;
	Util::get_stringlist_from_file(path,ls);
	string sha1;
	string fpath;
	hash_t hash;
	string url;
	//支持完整的hash或者http url
	for(it=ls.begin();it!=ls.end();++it)
	{
		//DEBUGMSG("# --> download: %s  \n",(*it).c_str());
		sha1 = Util::get_string_index(*it,0,"|");
		fpath = Util::get_string_index(*it,1,"|");
		Util::string_trim(fpath,' ');
		if(sha1.length()<8)
		{
			DEBUGMSG("# **** short sha1 ****\n");
			continue;
		}
		if(0==strncmp(sha1.c_str(),"http://",7)||0==strncmp(sha1.c_str(),"HTTP://",7))
		{
			hash.set_url_string(sha1.c_str());
			url = sha1;
		}
		else
		{
			if(0!=hash.set_string_hash(sha1.c_str()))
			{
				DEBUGMSG("# **** set sha1 wrong ****\n");
				continue;
			}
			url = "";
		}
		hash.set_string_hash(sha1.c_str());
		PIF::instance()->create_download(hash,fpath,url,0,0,SettingSngl::instance()->get_block_size(),FTYPE_DOWNLOAD);
	}
}
int LocalService::share_files_list(list<string>& ls)
{
	list<string>::iterator it;
	string tth;
	string path;
	string str;
	hash_t hash;
	char hashbuf[64];
	for(it=ls.begin();it!=ls.end();++it)
	{
		if(!m_brun)
			return -1;
		path = *it;
		Util::string_trim(path);
		if(path.empty())
		{
			continue;
		}
		if(!Util::file_exist(path))
		{
			printf("#:file not exist : path = %s \n",path.c_str());
			continue;
		}
		if(FileStorageSngl::instance()->check_exist_filepath(path,true))
		{
			printf("#:file aready share : path = %s \n",path.c_str());
			continue;
		}
		
		if(0!=::Sha1_BuildFile(path.c_str(),NULL,hashbuf,0))
		{
			printf("#:file hash failed : path = %s \n",path.c_str());
			continue;
		}
		hash.set_sha1_buffer((uchar*)hashbuf);
		PIF::instance()->share_file(hash,path);
	}
	return 0;
}

