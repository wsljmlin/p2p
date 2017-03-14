#pragma once
#include "USBFile.h"
#include "Thread.h"

class API_DECLSPEC USBFile_hex : public USBFile  
	, public Thread
{
public:
	USBFile_hex(void);
	virtual ~USBFile_hex(void);
public:
	//��hwnd_on_read!=NULL���࿪�����̣߳����Ҷ���������hwnd_on_read������Ϣ
	int open(const char* path,int packetlen,unsigned char reportid,HWND hwnd_on_read);
	virtual void close();
	virtual int work(int e);
	int write_hexstring(const char *str,int size);
protected:
	int on_read_packet_to_hexstring(const char* buf,int size);
protected:
	HWND m_hwnd;
	bool m_isThreadOpen;
};

