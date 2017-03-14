#pragma once
#include "commons.h"

class MessagePusher
{
	friend class Singleton<MessagePusher>;
private:
	MessagePusher(void);
	~MessagePusher(void);
public:
	int init();
	int fini();

	int on_downloadlist_start(const string& url);
	int on_downloadlist_stop(const string& url);
	int on_download_file_start(const string& url);
	int on_download_file_stop(const string& url);
private:
	int open_sock();
	int close_sock();

	template<typename T>
	int SendPTLPacket(uint16 cmd,T& inf,int iMaxSize);
private:
	SOCKET m_fd;
	unsigned short m_port;
	sockaddr_in m_addr;
	PTL_Head	tmp_head;
	
};

typedef Singleton<MessagePusher> MessagePusherSngl;
