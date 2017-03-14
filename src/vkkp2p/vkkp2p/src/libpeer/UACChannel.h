#pragma once
#include "Channel.h"
#include "uac_Socket.h"

class UACChannel : public Channel,public UAC_Socket
{
public:
	UACChannel(void);
	virtual ~UACChannel(void);

public:
	virtual int attach(SOCKET s,sockaddr_in& addr);
	virtual int connect(const char* ip,unsigned short port,const char* bindip=NULL,int nattype=0);
	virtual int connect(unsigned int ip,unsigned short port,const char* bindip=NULL,int nattype=0);
	virtual int disconnect();
	virtual int send(MemBlock *b,bool more=false);  //-1:false; 0:send ok; 1:put int sendlist
	virtual int recv(char *b,int size);

	//uac
	virtual int uac_on_read();
	virtual void uac_on_write();
	virtual void uac_on_connected(); //重载通知上层
};
