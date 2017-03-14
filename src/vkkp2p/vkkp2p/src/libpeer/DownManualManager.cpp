#include "DownManualManager.h"
#include "Util.h"
#include "Interface.h"
#include "Setting.h"
#include "FileStorage.h"
//#include "Timer.h"

DownManualManager::DownManualManager(void)
:m_bInit(false)
,m_bdowninit(false)
,m_isPausing(false)
,m_activeNum(0)
,m_maxActiveNum(2)
{
}

DownManualManager::~DownManualManager(void)
{
	assert(m_activeNum==0);
	clear();
}

int DownManualManager::run()
{
	if(m_bInit)
		return 0;
	m_bInit = true;
	m_maxActiveNum = SettingSngl::instance()->get_download_maxnum();
	InitPath();
	Load();
	activate();
	return 0;
}


void DownManualManager::end()
{
	if(!m_bInit)
		return;
	m_bInit = false;
	wait();
	Save();
	clear();
}
int DownManualManager::work(int e)
{
	Sleep(3000);
	DEBUGMSG("DownManualManager::Work() \n");
	DownInit();
	int n1 = 0;
	int n2 = 0;
	while(m_bInit)
	{
		Sleep(1000);
		n1++;
		n2++;
		if(n1>=3)
		{
			n1 = 0;
			DownUpdate();
		}
		if(n2>=30)
		{
			n2 = 0;
			Save();
		}
	}
	DEBUGMSG("DownManualManager::work() end \n");
	return 0;
}

int DownManualManager::add_down(const hash_t& hash,const string& filename,const string& path,const string& url,int state,int ftype)
{
	TLock<Mutex> l(m_mt);
	if(!m_bdowninit)
		return -1;
	char buf[64];
	hash.to_string(buf,64);
	string tth = buf;
	if(ExistDown(hash))
		return 1;
	DownManualInfo *inf = new DownManualInfo();
	if(!inf)
		return -1;
	inf->tth = tth;
	inf->hash = hash;
	inf->name = filename;
	inf->path = path;
	inf->url = url;
	inf->ftype = ftype;
	inf->state = DS_QUEUE;
	inf->createtime = (unsigned int)time(NULL);
	if(inf->name.empty())
		inf->name = tth;
	if(inf->path.empty())
		inf->path = m_path + inf->name + ".ic2";
	inf->last_state = inf->state;
	m_lsDown.push_back(inf);
	if(m_isPausing)
	{
		if(DS_START==state)
			state = DS_QUEUE;
		inf->last_state = state;
	}
	else
	{
		DownChangState(inf,state);
	}
	DownTryStart();
	return 0;
}

int DownManualManager::del_down(const hash_t& hash,bool isDelPhy)
{
	TLock<Mutex> l(m_mt);
	bool bdown=false,bready=false;
	for(FileIter it=m_lsFinished.begin();it!=m_lsFinished.end();)
	{
		if((*it)->hash == hash)
		{
			delete (*it);
			m_lsFinished.erase(it++);
			bready = true;
			break;
		}
		else
		{
			++it;
		}
	}
	for(FileIter it=m_lsDown.begin();it!=m_lsDown.end();)
	{
		if((*it)->hash == hash)
		{
			//停止任务，会尝试启动其它任务
			DownChangState(*it,DS_STOP);
			delete (*it);
			m_lsDown.erase(it++);
			bdown = true;
			break;
		}
		else
		{
			++it;
		}
	}
	if(bdown || bready)
		Save();
	PIF::instance()->delete_file(hash,isDelPhy);
	return 0;
}
int DownManualManager::set_state(const hash_t& hash,int state)
{
	TLock<Mutex> l(m_mt);
	FileIter it;
	for(it=m_lsDown.begin(); it!=m_lsDown.end(); ++it)
	{
		if((*it)->hash == hash)
			break;
	}
	if(it==m_lsDown.end())
		return -1;
	if(DownChangState(*it,state))
	{
		(*it)->last_state = DS_QUEUE; //pause过程中被设置过，不再自动resume
		return 0;
	}
	return -1;
}
int DownManualManager::pause()
{
	if(m_isPausing)
		return 0;
	DEBUGMSG("DownManualManager::pause() \n");
	m_isPausing = true;

	TLock<Mutex> l(m_mt);
	DownManualInfo *inf = NULL;
	FileIter it;
	for(it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		inf = *it;
		inf->last_state = inf->state;
		if(DS_START==inf->state)
		{
			DownChangState(inf,DS_QUEUE);
		}
	}

	return 0;
}
int DownManualManager::resume()
{
	if(!m_isPausing)
		return 0;
	DEBUGMSG("DownManualManager::resume() \n");
	m_isPausing = false;

	TLock<Mutex> l(m_mt);
	DownManualInfo *inf = NULL;
	FileIter it;
	for(it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		inf = *it;
		if(DS_START==inf->last_state)
		{
			DownChangState(inf,DS_START);
		}
	}
	DownTryStart();
	return 0;
}

void DownManualManager::InitPath()
{
	//
	string str,tmppath;
	tmppath = SettingSngl::instance()->get_download_path();
	if(!tmppath.empty())
	{
		//检查目录是否可用
		Util::my_create_directory(tmppath);
		//GetStatus()
		char c = tmppath.at(tmppath.length()-1);
		if('/' != c && '\\' != c)
			tmppath += "/";
		str = tmppath + "test.ic2!!";

		FILE *fp = fopen(str.c_str(),"wb+");
		if(fp)
		{
			fclose(fp);
			Util::file_delete(str);
		}
		else
		{
			tmppath = "";
		}
		if(!tmppath.empty())
			m_path = tmppath;

	}
	if(m_path.empty())
		m_path = SettingSngl::instance()->get_cache_path();
}

bool DownManualManager::StartDown(DownManualInfo* inf)
{
	DEBUGMSG("# DownManualManager::start: tth=%s \n",inf->tth.c_str());
	if(-1!=PIF::instance()->create_download(inf->hash,inf->path,inf->url,0,0,
		SettingSngl::instance()->get_block_size(),inf->ftype,true))
	{
		PIF::instance()->read_refer(inf->hash);
		inf->downtick = 0;
		inf->zerospeedtick = 0;
		m_activeNum++;
		inf->state = DS_START;
		DEBUGMSG("# DownManualManager::start ok!!!: tth=%s \n",inf->tth.c_str());
		return true;
	}
	else
	{
		assert(0);
		DEBUGMSG("# DownManualManager::start failed ! \n");
		return false;
	}
}
bool DownManualManager::StopDown(DownManualInfo* inf,int state)
{
	if(DS_START==state)
		return false;
	if(DS_START==inf->state)
	{
		PIF::instance()->read_release(inf->hash);
		m_activeNum--;
	}
	inf->speed = 0;
	inf->downtick = 0;
	inf->zerospeedtick = 0;
	inf->state = state;
	DEBUGMSG("#---stop or queue task : %s \n",inf->path.c_str());
	return true;
}

bool DownManualManager::ExistDown(const hash_t& hash)
{
	FileIter it;
	for(it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		if((*it)->hash == hash)
			return true;
	}
	for(it=m_lsFinished.begin();it!=m_lsFinished.end();++it)
	{
		if((*it)->hash == hash)
			return true;
	}
	return false;
}

bool DownManualManager::DownChangState(DownManualInfo* inf,int state)
{
	switch(state)
	{
	case DS_STOP:
	case DS_QUEUE:
		{
			if(DS_START==inf->state)
			{
				StopDown(inf,state);
				//注意：主动设置为queue时，不会尝试启动其它任务，这用于保证可以同时设置指定几个任务START
				if(DS_STOP == state)
					DownTryStart(&inf->hash);
			}
			else
			{
				//由DS_STOP|DS_QUEUE 到 DS_QUEUE，则会尝试启动
				if(DS_QUEUE==state && m_activeNum<m_maxActiveNum)
				{
					if(!StartDown(inf))
						inf->state = state;
				}
				else
					inf->state = state;
			}
		}
		break;
	case DS_START:
		{
			if(DS_START!=inf->state)
			{
				if(StartDown(inf))
					DownTryQueue(&inf->hash);
			}
		}
		break;
	default:
		assert(false);
		return false;
	}
	Save();
	return true;
}
void DownManualManager::DownTryStart(hash_t* exclude_hash/*=NULL*/)
{
	if(m_activeNum>=m_maxActiveNum)
		return;
	DownManualInfo *inf = NULL;
	for(FileIter it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		if(exclude_hash && (*it)->hash == *exclude_hash)
			continue;
		inf = *it;
		if(DS_QUEUE==inf->state)
			StartDown(inf);
		if(m_activeNum>=m_maxActiveNum)
			return;
	}
}

void DownManualManager::DownTryQueue(hash_t* exclude_hash/*=NULL*/)
{
	if(m_activeNum<=m_maxActiveNum)
		return;
	DownManualInfo *inf = NULL;
	for(FileIter it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		if(exclude_hash && (*it)->hash == *exclude_hash)
			continue;
		inf = *it;
		if(DS_START==inf->state)
		{
			StopDown(inf,DS_QUEUE);
			if(m_activeNum<=m_maxActiveNum)
				return;
		}
	}
}
void DownManualManager::Load()
{
	LoadList(m_lsDown,SettingSngl::instance()->get_data_path()+"downmanual_info.dat");
	LoadList(m_lsFinished,SettingSngl::instance()->get_data_path()+"downmanual_info_fini.dat");
	//修改路径：
	FileIter it;
	DownManualInfo *inf;
	FileInfo *fi;
	for(it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		inf = *it;
		if((fi=FileStorageSngl::instance()->get_downinfo(inf->hash)))
		{
			inf->path = fi->despath;
		}
		else
		{
			inf->path = m_path + inf->name + ".ic2";
		}
	}
	for(it=m_lsFinished.begin();it!=m_lsFinished.end();)
	{
		inf = *it;
		if((fi=FileStorageSngl::instance()->get_readyinfo(inf->hash)))
		{
			inf->path = fi->path;
			++it;
		}
		else
		{
			delete inf;
			m_lsFinished.erase(it++);
		}
	}
	Save();
}
void DownManualManager::LoadList(FileList& fls,const string& path)
{
	//tth|size|name|path|url|ftype|state|createtime|progress|speed|
	list<string> ls;
	string str;
	DownManualInfo *inf=NULL;
	Util::get_stringlist_from_file(path,ls);
	if(ls.empty())
		return;
	for(list<string>::iterator it=ls.begin();it!=ls.end();++it)
	{
		str = *it;
		if(str.empty())
			continue;
		inf = new DownManualInfo();
		inf->tth = Util::get_string_index(str,0,"|");
		if(0!=inf->hash.set_string_hash(inf->tth.c_str()))
		{
			delete inf;
			continue;
		}
		inf->size = Util::atoll(Util::get_string_index(str,1,"|").c_str());
		inf->name = Util::get_string_index(str,2,"|");
		inf->path = Util::get_string_index(str,3,"|");
		inf->url = Util::get_string_index(str,4,"|");
		inf->ftype = atoi(Util::get_string_index(str,5,"|").c_str());
		inf->state = atoi(Util::get_string_index(str,6,"|").c_str());
		inf->createtime = atoi(Util::get_string_index(str,7,"|").c_str());
		inf->progress = atoi(Util::get_string_index(str,8,"|").c_str());
		inf->speed = 0;
		fls.push_back(inf);
	}
}
void DownManualManager::Save()
{
	SaveList(m_lsDown,SettingSngl::instance()->get_data_path()+"downmanual_info.dat");
	SaveList(m_lsFinished,SettingSngl::instance()->get_data_path()+"downmanual_info_fini.dat");
	
}
void DownManualManager::SaveList(FileList& fls,const string& path)
{
	//tth|size|name|path|url|ftype|state|createtime|progress|speed|
	list<string> ls;
	string str;
	DownManualInfo *inf=NULL;
	char buf[2048];
	FileIter it;
	for(it=fls.begin();it!=fls.end();++it)
	{
		inf = *it;
		sprintf(buf,"%s|%lld|%s|%s|%s|%d|%d|%d|%d|%d",
			inf->tth.c_str(),inf->size,inf->name.c_str(),inf->path.c_str(),inf->url.c_str()
			,inf->ftype,inf->state,inf->createtime,inf->progress,inf->speed);
		ls.push_back(buf);
	}
	if(!ls.empty())
		Util::put_stringlist_to_file(path,ls);
	else
		Util::file_delete(path);
}

void DownManualManager::clear()
{
	FileIter it;
	for(it=m_lsDown.begin();it!=m_lsDown.end();++it)
		delete (DownManualInfo*)(*it);
	for(it=m_lsFinished.begin();it!=m_lsFinished.end();++it)
		delete (DownManualInfo*)(*it);
	m_lsDown.clear();
	m_lsFinished.clear();
}
void DownManualManager::DownUpdate()
{
	//
	TLock<Mutex> l(m_mt);
	DownManualInfo *fi=NULL;
	list<hash_t> ls_df;
	for(FileIter it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		if((*it)->state==DS_START)
		{
			fi = *it;
			if(0>=fi->size)
			{
				uint64 size = PIF::instance()->get_filesize(fi->hash);
				if(size>0)
					fi->size = size;
			}
			else
			{
				DownloadInfo inf;
				if(0==PIF::instance()->get_downloadinfo(fi->hash,inf))
				{
					fi->speed = inf.speedB;
					if(inf.blocks!=0)
						fi->progress = inf.down_blocks * 1000 / inf.blocks;
					if(fi->progress>=1000)
					{
						//下载完成
						ls_df.push_back(fi->hash);
					}
				}
			}
		}
	}
	for(list<hash_t>::iterator it=ls_df.begin();it!=ls_df.end();++it)
	{
		DownFinished(*it);
	}
	if(!ls_df.empty())
	{
		Save();
	}
}

void DownManualManager::DownFinished(const hash_t& hash)
{
	for(FileIter it=m_lsDown.begin();it!=m_lsDown.end();)
	{
		if((*it)->hash == hash)
		{
			DEBUGMSG("download finished:%s \n",(*it)->name.c_str());
			DownManualInfo *inf = *it;
			assert(inf->progress==1000);
			DownChangState(inf,DS_STOP);
			m_lsFinished.push_back(inf);
			m_lsDown.erase(it++);
			return;
		}
		else
		{
			++it;
		}
	}
}

void DownManualManager::DownInit()
{
	TLock<Mutex> l(m_mt);
	m_bdowninit = true;
	for(FileIter it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		if((*it)->state==DS_START)
		{
			if(m_activeNum>=m_maxActiveNum)
			{
				(*it)->state = DS_QUEUE;
			}
			else
			{
				if(!StartDown(*it))
					(*it)->state = DS_QUEUE;
			}
		}
	}
	DownTryStart(NULL);
}
 //会改变状态，所以先Save()
void DownManualManager::DownFini()
{
	//注意：此处停止，但不改变任务状态
	for(FileIter it=m_lsDown.begin();it!=m_lsDown.end();++it)
	{
		if((*it)->state==DS_START)
			StopDown(*it,DS_QUEUE);
	}
}

//*********************************************************************************************
//供C接口使用
unsigned int DownManualManager::down_file_get_count()
{
	return m_lsDown.size();
}
int DownManualManager::down_file_get_info(unsigned int index,unsigned int n,FileList& ls)
{
	TLock<Mutex> l(m_mt);
	if(index>=m_lsDown.size())
		return 0;
	unsigned int i=0;
	unsigned int count=0;
	FileIter it;
	for(it=m_lsDown.begin();it!=m_lsDown.end() && i<index;++it,i++);
	for(;it!=m_lsDown.end()&&count<n;++it)
	{
		DownManualInfo *inf = new DownManualInfo();
		if(!inf)
			break;
		*inf = *(*it);
		ls.push_back(inf);
		count++;
	}
	return count;
}
int DownManualManager::down_file_set_status(unsigned int index,int state) //DS_STOP=0,DS_START=1,DS_QUEUE=2
{
	TLock<Mutex> l(m_mt);
	if(index >=m_lsDown.size())
		return -1;
	unsigned int i=0;
	FileIter it;
	for(it=m_lsDown.begin(); it!=m_lsDown.end() && i<index; ++it,i++);
	if(DownChangState(*it,state))
	{
		(*it)->last_state = DS_QUEUE; //pause过程中被设置过，不再自动resume
		return 0;
	}
	return -1;
}
int DownManualManager::down_file_set_priority(unsigned int oldIndex,unsigned int newIndex) //将它从原索引排到新索引位置
{
	TLock<Mutex> l(m_mt);
	if(oldIndex>=m_lsDown.size() || newIndex>=m_lsDown.size())
		return -1;
	if(oldIndex==newIndex)
		return 0;
	
	unsigned int i=0;
	DownManualInfo *inf = NULL;
	FileIter it;
	for(it=m_lsDown.begin(),i=0;it!=m_lsDown.end() && i<oldIndex;++it,++i);
	inf = *it;
	m_lsDown.erase(it);
	for(it=m_lsDown.begin(),i=0;it!=m_lsDown.end() && i<newIndex;++it,++i);
	m_lsDown.insert(it,inf);
	return 0;
}
int DownManualManager::down_file_stop_all()
{
	TLock<Mutex> l(m_mt);
	DownManualInfo *inf = NULL;
	FileIter it;
	for(it=m_lsDown.begin(); it!=m_lsDown.end(); ++it)
	{
		inf = *it;
		if(DS_STOP!=inf->state)
			StopDown(inf,DS_STOP);
		inf->last_state = DS_STOP;
	}
	m_isPausing = false;
	return 0;
}
int DownManualManager::down_file_queue_all_from_stop()
{
	TLock<Mutex> l(m_mt);
	DownManualInfo *inf = NULL;
	FileIter it;
	for(it=m_lsDown.begin(); it!=m_lsDown.end(); ++it)
	{
		inf = *it;
		//只要不是start的，都改为queue,然后会尝试激活
		if(DS_START!=inf->state)
		{
			DownChangState(inf,DS_QUEUE);
			inf->last_state = DS_QUEUE;
		}
	}
	m_isPausing = false;
	return 0;
}

int DownManualManager::down_file_delete(unsigned int index)
{
	TLock<Mutex> l(m_mt);
	if(index >=m_lsDown.size())
		return -1;
	unsigned int i=0;
	FileIter it;
	for(it=m_lsDown.begin(); it!=m_lsDown.end() && i<index; ++it,i++);
	hash_t hash = (*it)->hash;
	return del_down(hash,true);
}
//****************************************************
unsigned int DownManualManager::downfini_file_get_count()
{
	return m_lsFinished.size();
}
int DownManualManager::downfini_file_get_info(unsigned int index,unsigned int n,FileList& ls)
{
	TLock<Mutex> l(m_mt);
	if(index>=m_lsFinished.size())
		return 0;
	unsigned int i=0;
	unsigned int count=0;
	FileIter it;
	for(it=m_lsFinished.begin();it!=m_lsFinished.end() && i<index;++it,i++);
	for(;it!=m_lsFinished.end()&&count<n;++it)
	{
		DownManualInfo *inf = new DownManualInfo();
		if(!inf)
			break;
		*inf = *(*it);
		ls.push_back(inf);
		count++;
	}
	return count;
}
int DownManualManager::downfini_file_delete(unsigned int index)
{
	TLock<Mutex> l(m_mt);
	if(index >=m_lsFinished.size())
		return -1;
	unsigned int i=0;
	hash_t hash;
	FileIter it;
	for(it=m_lsFinished.begin(); it!=m_lsFinished.end() && i<index; ++it,i++);
	hash = (*it)->hash;
	delete (*it);
	m_lsFinished.erase(it);
	Save();
	PIF::instance()->delete_file(hash,true);
	return 0;
}

