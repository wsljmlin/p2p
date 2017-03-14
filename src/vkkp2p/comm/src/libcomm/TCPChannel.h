#pragma once
#include "Channel.h"
#include "Handler.h"

//

class TCPChannel : public Channel,public SockHandler
{
	typedef list<MemBlock*> SendList;
	typedef SendList::iterator SendIter;
public:
	TCPChannel(void);
	virtual ~TCPChannel(void);
public:
	virtual int attach(SOCKET s,sockaddr_in& addr);
	virtual int connect(const char* ip,unsigned short port,const char* bindip=NULL,int nattype=0);
	virtual int connect(unsigned int ip,unsigned short port,const char* bindip=NULL,int nattype=0);
	virtual int disconnect();
	virtual int send(MemBlock *b,bool more=false);
	virtual int recv(char *b,int size);
	virtual unsigned int get_my_ip(); //获取我自己的IP，在连接成功后可以获取
	virtual int set_sndbuf(int bufsize);

	virtual int sock(){return (int)m_fd;}
	virtual int handle_input();
	virtual int handle_output();
	virtual int handle_error();
protected:
	virtual void reset();
	void close_socket();
	int on_connected();
	//增加以下接口仅用于支持穿透80端口http防火墙而增加
	virtual void on_connected_ex();
protected:
	SOCKET m_fd;
	SendList m_slist; //sending list
	int m_smore; //recode last fire writable if call send();
	int m_is_regwrite; //only be used after connected
};
