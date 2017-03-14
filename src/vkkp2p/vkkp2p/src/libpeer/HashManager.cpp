#include "HashManager.h"
#include "DownloadManager.h"
#include "sha1.h"

HashManager::HashManager(void)
{
}

HashManager::~HashManager(void)
{
	m_tmqueue.ClearMessage(_clear_msg_func);
}

int HashManager::run()
{
	TimerSngl::instance()->register_timer(this,1,30);
	return 0;
}
void HashManager::end()
{
	TimerSngl::instance()->unregister_all(this);
}
void HashManager::on_timer(int e)
{
	Message* msg = NULL;
	HashInfo *inf = NULL;
	while((msg=m_tmqueue.GetMessage(0)))
	{
		inf = (HashInfo*)msg->data;
#ifdef SM_VOD
		DownloadManagerSngl::instance()->on_file_done(inf->hash,inf->playtype,inf->newhash,inf->done_type);
#else
		DownloadManagerSngl::instance()->on_file_done(inf->hash,inf->newhash,inf->done_type);
#endif /* end of SM_VOD */
		_clear_msg_func(msg);
		msg = NULL;
	}
}
int HashManager::check_filehash(const hash_t& hash,const string& path,char done_type)
{
	HashInfo *inf = new HashInfo();
	inf->hash = hash;
	inf->newhash = hash;
	inf->path = path;
	inf->done_type = done_type;
	m_tmqueue.AddMessage(new Message(1,inf,0));
	return 0;
}

#ifdef SM_VOD
int HashManager::check_filehash(const hash_t& hash,int playtype, const string& path,char done_type)
{
	HashInfo *inf = new HashInfo();
	inf->hash = hash;
	inf->newhash = hash;
	inf->path = path;
	inf->done_type = done_type;
	inf->playtype=playtype;
	m_tmqueue.AddMessage(new Message(1,inf,0));
	return 0;
}

#endif /* end of SM_VOD */
void HashManager::_clear_msg_func(Message* msg)
{
	delete (HashInfo*)msg->data;
	delete msg;
}

