#include "Interface.h"
#include "Initializers.h"
#include "UserManager.h"
#include "StatManager.h"
#include "SourceService.h"

enum {
	CMD_GET_TRACKER_INFO
	,CMD_GET_SERVER_INFO
	,CMD_GET_FILE_INFO
	,CMD_GET_ALLFILE_INFO
};

typedef struct tagCmdInfo
{
	int result;
	void *data;
	bool is_set; //判断是否已经处理完成
	tagCmdInfo()
	{
		result = 0;
		data = NULL;
		is_set = false;
	}
	void set() { is_set=true;}
	void wait() { while(!is_set) Sleep(10);}
}CmdInfo;
//**********************************************************
TIF* TIF::__instance = NULL;
TIF::TIF(void)
{
}

TIF::~TIF(void)
{
	m_queue.ClearMessage(_clear_message_func);
}


int TIF::init()
{
	return tracker_init();
}
void TIF::fini()
{
	tracker_fini();
}
int TIF::get_msg_tracker_info(MsgTrackerInfo& msg)
{
	CmdInfo inf;
	inf.data = &msg;
	m_queue.AddMessage(new Message(CMD_GET_TRACKER_INFO,&inf,0));
	inf.wait();
	return inf.result;
}
int TIF::get_msg_server_info(MsgServerInfo& msg)
{
	CmdInfo inf;
	inf.data = &msg;
	m_queue.AddMessage(new Message(CMD_GET_SERVER_INFO,&inf,0));
	inf.wait();
	return inf.result;
}
int TIF::free_msg_server_info(MsgServerInfo& msg)
{
	for(list<MsgServerNode*>::iterator it=msg.svrs.begin();it!=msg.svrs.end();++it)
	{
		delete (MsgServerNode*)*it;
	}
	msg.svrs.clear();
	return 0;
}
int TIF::get_msg_file_info(MsgFileInfo& msg)
{
	CmdInfo inf;
	inf.data = &msg;
	m_queue.AddMessage(new Message(CMD_GET_FILE_INFO,&inf,0));
	inf.wait();
	return inf.result;
}
int TIF::get_msg_allfile_info(list<MsgFileInfo*>& ls)
{
	CmdInfo inf;
	inf.data = &ls;
	m_queue.AddMessage(new Message(CMD_GET_ALLFILE_INFO,&inf,0));
	inf.wait();
	return inf.result;
}
int TIF::free_msg_allfile_info(list<MsgFileInfo*>& ls)
{
	for(list<MsgFileInfo*>::iterator it=ls.begin();it!=ls.end();++it)
	{
		delete (MsgFileInfo*)*it;
	}
	ls.clear();
	return 0;
}
void TIF::handle_root()
{
	//注意：本函数只被userpeer线程调用
	Message* msg = NULL;
	CmdInfo *inf = NULL;
	while((msg=m_queue.GetMessage(0)))
	{
		switch(msg->cmd)
		{
		case CMD_GET_TRACKER_INFO:
			{
				inf = (CmdInfo*)msg->data;
				inf->result = StatManagerSngl::instance()->get_msg_tracker_info(*(MsgTrackerInfo*)inf->data);
				inf->set();
			}
			break;
		case CMD_GET_SERVER_INFO:
			{
				inf = (CmdInfo*)msg->data;
				inf->result = UserManagerSngl::instance()->get_msg_server_info(*(MsgServerInfo*)inf->data);
				inf->set();
			}
			break;
		case CMD_GET_FILE_INFO:
			{
				inf = (CmdInfo*)msg->data;
				inf->result = SourceServiceSngl::instance()->get_msg_file_info(*(MsgFileInfo*)inf->data);
				inf->set();
			}
			break;
		case CMD_GET_ALLFILE_INFO:
			{
				inf = (CmdInfo*)msg->data;
				inf->result = SourceServiceSngl::instance()->get_msg_allfile_info((*(list<MsgFileInfo*>*)inf->data));
				inf->set();
			}
			break;
		default:
			break;
		}
		_clear_message_func(msg);
	}
}
void TIF::_clear_message_func(Message* msg)
{
	if(!msg)
	{
		assert(0);
		return;
	}
	switch(msg->cmd)
	{
	case CMD_GET_TRACKER_INFO:
	case CMD_GET_SERVER_INFO:
	case CMD_GET_FILE_INFO:
	case CMD_GET_ALLFILE_INFO:
		{
		}
		break;
	default:
		break;
	}
	delete msg;
}

