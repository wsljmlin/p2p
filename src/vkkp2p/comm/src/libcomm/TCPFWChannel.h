#pragma once
#include "TCPChannel.h"

//发送256个字节头
class TCPFWChannel : public TCPChannel
{
public:
	TCPFWChannel(void){}
	virtual ~TCPFWChannel(void){}
	virtual int handle_input();
protected:
	virtual void on_connected_ex();
	int  send_http(int type);
	int  recv_http();
	char* format_req(char* buf,int size);
	char* format_rsp(char* buf,int size);
protected:
	int m_recv_size;
};
