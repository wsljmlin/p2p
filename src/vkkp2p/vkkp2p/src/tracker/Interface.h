#pragma once
#include "TrackerMessage.h"
#include "MessageQueue.h"

//tracker interface ->TIF
class Scheduler;
class TIF
{
	friend class Scheduler;
public:
	TIF(void);
	virtual ~TIF(void);
	static TIF* __instance;
	static TIF* instance() { if(!__instance) __instance=new TIF();return __instance;}
	static void destroy() { if(__instance){delete __instance;__instance=NULL;}}
public:
	int init();
	void fini();

	int get_msg_tracker_info(MsgTrackerInfo& msg);
	int get_msg_server_info(MsgServerInfo& msg);
	int free_msg_server_info(MsgServerInfo& msg);
	int get_msg_file_info(MsgFileInfo& msg);
	int get_msg_allfile_info(list<MsgFileInfo*>& ls);
	int free_msg_allfile_info(list<MsgFileInfo*>& ls);
private:
	void handle_root();
	static void _clear_message_func(Message* msg);
private:
	MessageQueue m_queue;

};


