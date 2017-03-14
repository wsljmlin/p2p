#pragma once
#include <Windows.h>

#ifndef API_DECLSPEC
	#define API_DECLSPEC
#endif

#define HIDMSG_ON_READ_PACKET_HEXSTRING WM_USER+102

class API_DECLSPEC USBFile
{
public:
	USBFile(void);
	virtual ~USBFile(void);
	
public:
	
	int open(const char* path,int packetlen,unsigned char reportid);
	bool is_open()const { return m_h!=INVALID_HANDLE_VALUE;}
	virtual void close();
	int write(const char* buf,int size);
	int read_packet(char* buf,int size); //size±ØÐë>=packetlen
	
protected:
	int write_pack(const char* buf);
	

protected:
	HANDLE m_h;
	OVERLAPPED m_read_overlapped;
	OVERLAPPED m_write_overlapped;
	char m_sendbuf[1024];
	int m_packetlen;
	unsigned char m_reportid;

};

