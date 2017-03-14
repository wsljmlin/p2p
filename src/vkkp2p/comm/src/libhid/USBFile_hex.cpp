#include "USBFile_hex.h"
#include <stdio.h>
#pragma warning(disable:4996)

USBFile_hex::USBFile_hex(void)
	:m_hwnd(NULL)
	,m_isThreadOpen(false)
{
}


USBFile_hex::~USBFile_hex(void)
{
}
int USBFile_hex::open(const char* path,int packetlen,unsigned char reportid,HWND hwnd_on_read)
{
	int ret = 0;
	if(0!=(ret=USBFile::open(path,packetlen,reportid)))
		return ret;
	m_hwnd = hwnd_on_read;
	if(m_hwnd)
	{
		m_isThreadOpen = true;
		activate();
	}
	
	return ret;
}
void USBFile_hex::close()
{
	if(m_isThreadOpen)
	{
		m_isThreadOpen = false;
		wait();
	}
	USBFile::close();
	m_hwnd = NULL;
}
int USBFile_hex::work(int e)
{
	char buf[1024];
	int n;
	while(m_isThreadOpen)
	{
		n = read_packet(buf,m_packetlen);
		if(m_packetlen==n)
		{
			on_read_packet_to_hexstring(buf+1,m_packetlen-1);
		}
		else
		{
			Sleep(100);
		}
	}
	return 0;
}

//**********************************************************************************
int hex_Buf2Str(char* sOutStr,int& iOutSize, const char* sInBuf,int iSize)
{
	if(!sOutStr || !sInBuf)
		return -1;
	iOutSize = iSize*4;
	for(int i=0;i<iSize;++i)
		sprintf(sOutStr+4*i,"0x%02x",(const unsigned char)sInBuf[i]);
	sOutStr[iSize*4] = '\0';
	return 0;
}

char hex_itoc(unsigned int i)
{
	static int endian_type = 2;
	char c = 0;
	if(2==endian_type)
	{
		unsigned short i = 0x0901;
		if(0x01 == *(char*)&i)
			endian_type = 0; //little endian
		else
			endian_type = 1; //big_endian
	}
	if(0==endian_type)
		memcpy(&c,(char*)&i,1);
	else
		memcpy(&c,((char*)&i)+3,1);
	return c;
}
int hex_Str2Buf(char* sOutBuf,int& iOutSize, const char* sInStr,int iSize)
{
	if(!sOutBuf || !sInStr)
		return -1;
	unsigned int tmp=0;
	iOutSize = iSize/4;
	for(int i=0;i<iOutSize;++i)
	{
		sscanf(sInStr+4*i+2,"%2x",&tmp);
		sOutBuf[i] = hex_itoc(tmp);
	}
	return 0;
}

//==========================================================================
int USBFile_hex::write_hexstring(const char *str,int size)
{
	//¼ÙÉèstr=0x120x010x32
	char *buf = new char[size];
	int outsize=size;
	hex_Str2Buf(buf,outsize,str,size);
	int ret = write(buf,outsize);
	delete[] buf;
	return ret;
}
int USBFile_hex::on_read_packet_to_hexstring(const char* buf,int size)
{
	int outsize = size*4+4;
	char* pack = new char[outsize];
	hex_Buf2Str(pack,outsize,buf,size);
	::PostMessageA(m_hwnd,HIDMSG_ON_READ_PACKET_HEXSTRING,(WPARAM)pack,(LPARAM) outsize);
	return 0;
}
