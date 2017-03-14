#pragma once

#include "rlist.h"
#include "SynchroObj.h"


typedef struct tagMessage
{
	int cmd;
	void *data;
	int hid;
	tagMessage():cmd(0),data(NULL),hid(0) {}
	tagMessage(int aCmd, void* aData,int aHid):cmd(aCmd),data(aData),hid(aHid){}
}Message;

class MessageQueue
{
public:
	MessageQueue(void);
	~MessageQueue(void);

	typedef struct tagMessageNode
	{
		Message *msg;
		rlist_head_t leaf;
		tagMessageNode(Message *aMsg) : msg(aMsg) {}
	}MessageNode;

	typedef CriticalSection Mutex;
	typedef TLock<Mutex> Lock;

public:
	Message* GetMessage(unsigned int millsec=INFINITE);
	void AddMessage(Message *msg);
	void ClearMessage(void (*CLEAR_MESSAGE_FUNC)(Message*) = 0);
	unsigned int Size()const {return m_size;}
private:
	rlist_head_t m_head;
	int m_size;
	//Semaphore m_sem;
	CriticalSection m_mt;
};

