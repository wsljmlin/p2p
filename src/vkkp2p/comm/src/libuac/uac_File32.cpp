#include "uac_File32.h"


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifndef _WIN32
#include <unistd.h>
#else
// 4482 4267 4018 4800 4311 4312 4102
#pragma warning(disable:4996)
#endif

namespace UAC
{

File32::File32(void)
: m_fp(NULL)
{
}

File32::~File32(void)
{
	this->close();
}
int File32::open(const char *_Filename,int _Mode)
{
	if(m_fp)
		return -1;

	char cmode[12];
	memset(cmode,0,12);
	//目前全当二进制打开
	_Mode |= F32_BINARY;
	if(_Mode&F32_TRUNC)
	{
		if(_Mode&F32_BINARY)
			strcpy(cmode,"wb+");
		else
			strcpy(cmode,"w+");
	}else
	{
		//暂时简单处理，不管是否要写，都允许可写
		if(_Mode&F32_BINARY)
			strcpy(cmode,"rb+");
		else
			strcpy(cmode,"r+");
	}
	m_fp = fopen(_Filename,cmode);
	if(NULL==m_fp)
		return -1;
	return 0;
}
bool File32::is_open() const 
{
	return (m_fp!=NULL);
}
void File32::close() 
{
	if(m_fp)
	{
		fclose(m_fp);
		m_fp = NULL;
	}
}

int File32::seek(int _Offset,int _Origin) 
{
	if(!m_fp)
	{
		assert(0);
		return -1;
	}
	//考虑如果之前已经feof()到尾,可以清空eof
	//clearerr(m_fp);
	return fseek(m_fp,_Offset,_Origin); //成功返回0
}

int File32::tell()
{
	if(!m_fp)
	{
		assert(0);
		return -1;
	}
	return ftell(m_fp);
}
void File32::flush() 
{
	if(!m_fp)
	{
		assert(0);
		return;
	}
	fflush(m_fp);
}
bool File32::eof()
{
	if(!m_fp)
	{
		assert(0);
		return true;
	}
	//如果到文件尾feof返回非0
	return (0!=feof(m_fp));
}

int File32::write(const char *buf,int len) 
{
	if(!m_fp)
	{
		assert(0);
		return -1;
	}
	return (int)fwrite(buf,1,len,m_fp);
}
int File32::read(char *buf,int len)
{
	if(!m_fp)
	{
		assert(0);
		return -1;
	}
	return (int)fread(buf,1,len,m_fp);
}
char* File32::getline(char *buf,int maxlen)
{
	if(!m_fp)
	{
		assert(0);
		return NULL;
	}
	return fgets(buf,maxlen,m_fp);
}
bool File32::write_n(const char *buf,int len) 
{
	if(!m_fp)
	{
		assert(0);
		return false;
	}
	return (1==fwrite(buf,len,1,m_fp));
}
bool File32::read_n(char *buf,int len)
{
	if(!m_fp)
	{
		assert(0);
		return false;
	}
	//如果读长度不够len,实际读到多少就多少.pos照样移动,返回为0
	return (1==fread(buf,len,1,m_fp));
}
int File32::remove_file(const char* path)
{
	return unlink(path);
}
int File32::rename_file(const char* from,const char* to)
{
	return rename(from,to);
}
}
