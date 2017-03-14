#pragma once
#include "commons.h"
#include "MessageQueue.h"

class HashManager : public TimerHandler
{
public:
	HashManager(void);
	~HashManager(void);
	typedef struct tagHashInfo
	{
		hash_t hash;
		hash_t newhash;
		string path;
		char done_type; //1:file done;2:file memcache done;
#ifdef SM_VOD
		int playtype;
#endif
	}HashInfo;
public:
	int run();
	void end();
	virtual void on_timer(int e);
#ifdef SM_VOD
	int check_filehash(const hash_t& hash,int playtype, const string& path,char done_type);
#endif
	int check_filehash(const hash_t& hash,const string& path,char done_type);
	static void _clear_msg_func(Message* msg);
private:
	MessageQueue m_tmqueue;
};
typedef Singleton<HashManager> HashManagerSngl;
