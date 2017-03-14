#include "RDBFile64Base.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


#ifdef _WIN32
#pragma warning(disable:4996)
#else
#define stricmp strcasecmp
#endif


int BFile64::resize(ssize64_t size)
{
	if(_file.is_open())
	{
		if(size>0)
		{
			ssize64_t pos = _file.tell();
			_file.seek(size-1,SEEK_SET);
			char buf[2];
			buf[0] = 1;
			_file.write(buf,1);
			_file.flush();
			_file.seek(pos,SEEK_SET);
		}
		return 0;
	}
	return -1;
}
//int BFile64::eif_get_node(File64& file,ssize64_t bpos,RDBEIFNode_t& inf)
//{
//	char buf[256];
//	char buf_end[16];
//	if(bpos!=file.seek(bpos,SEEK_SET))
//		return -1;
//	if(EIF_HEAD_SIZE!=file.read(buf,EIF_HEAD_SIZE))
//		return -1;
//	if(0!=strncmp(buf,EIF_STX_BEG,8))
//		return -1;
//	int pos = 8;
//	memcpy((char*)&inf.head_size,buf+pos,4);
//	pos += 4;
//	memcpy((char*)&inf.file_size,buf+pos,8);
//	pos += 8;
//	memcpy((char*)&inf.id,buf+pos,4);
//	pos += 4;
//	memcpy((char*)inf.name,buf+pos,128);
//	pos += 128;
//	assert(pos == EIF_HEAD_SIZE);
//	assert(inf.head_size == EIF_HEAD_SIZE);
//
//	file.seek(inf.head_size-EIF_HEAD_SIZE+inf.file_size,SEEK_CUR);
//	if(8!=file.read(buf_end,8))
//		return -1;
//	if(0!=strncmp(buf_end,EIF_STX_END,8))
//		return -1;
//	inf.begin_pos = bpos + inf.head_size;
//	return 0;
//}
//
//int BFile64::eif_copy_file(File64& from,ssize64_t from_pos,File64& to,ssize64_t to_pos,size64_t size)
//{
//	size64_t write_size=0;
//	char buf[1204];
//	int len;
//	if(from_pos!=from.seek(from_pos,SEEK_SET))
//		return -1;
//	if(to_pos!=to.seek(to_pos,SEEK_SET))
//		return -1;
//
//	while(write_size<size)
//	{
//		len = 1024;
//		if(write_size+len>size)
//			len = (int)(size-write_size);
//		len = from.read(buf,len);
//		if(len<=0) break;
//		if(len != to.write(buf,len))
//			break;
//		write_size += len;
//	}
//	if(write_size==size)
//		return 0;
//	assert(false);
//	return -1;
//}
//******************************************************************************************
//RDBFile64Base

void ___rdbs_encrypt(char* buf,int size)
{
	for(int i=0;i<size;++i)
	{
		buf[i] = ~buf[i];
		buf[i] += 0x01;
	}
}
void ___rdbs_unencrypt(char* buf,int size)
{
	for(int i=0;i<size;++i)
	{
		buf[i] -= 0x01;
		buf[i] = ~buf[i];
	}
}

RDBFile64Simple::RDBFile64Simple(void)
:m_head_size(0)
,m_file_size(0)
,m_pos(0)
{
}

RDBFile64Simple::~RDBFile64Simple(void)
{
}
int RDBFile64Simple::open(const char* path,int mode)
{
	assert(F64_READ==mode);
	if(F64_READ != mode)
		return -1;
	_file.close();
	if(0!=_file.open(path,F64_READ))
		return -1;
	size64_t size = _file.seek(0,SEEK_END);
	_file.seek(0,SEEK_SET);
	if(size<188)
	{
		_file.close();
		return -1;
	}
	char buf[188];
	int pos=0;
	int endian,ver;
	if(60!=_file.read(buf,60))
	{
		_file.close();
		return -1;
	}

	//40-stx;4-endian;4-ver;4-header; 8-file_size
	if(0!=strncmp(buf,RDBS_STX,40))
		return -1;
	pos = 40;
	memcpy((char*)&endian,buf+pos,4);
	pos += 4;
	memcpy((char*)&ver,buf+pos,4);
	pos += 4;
	if(1!=ver)
	{
		_file.close();
		return -1;
	}
	memcpy((char*)&m_head_size,buf+pos,4);
	pos += 4;
	memcpy((char*)&m_file_size,buf+pos,8);
	pos += 8;
	assert(60==pos);
	assert(m_head_size>=188 && m_head_size<1024000);
	if(m_head_size<188 || 0==m_file_size)
	{
		_file.close();
		return -1;
	}
	m_pos = 0;
	return 0;
}
int RDBFile64Simple::close()
{
	_file.close();
	m_pos =0;
	m_head_size=0;
	m_file_size=0;
	return 0;
}
ssize64_t RDBFile64Simple::seek(ssize64_t distance,int smode)
{
	if(SEEK_SET==smode)
		m_pos = distance;
	else if(SEEK_CUR==smode)
		m_pos += distance;
	else if(SEEK_END==smode)
		m_pos = m_file_size+distance;
	else
		return -1;
	if(m_pos>(ssize64_t)m_file_size)
		m_pos = m_file_size;
	if(m_pos<0)
		m_pos = 0;
	return m_pos;
}
int RDBFile64Simple::read(char *buf,int len)
{
	if(!_file.is_open())
		return 0;
	if(m_pos + len > (ssize64_t)m_file_size)
		len = (int)(m_file_size-m_pos);
	if(len<1)
		return 0;

	int read_len = 0;
	int n = 0,ret=0;
	if(m_pos<m_head_size)
	{
		n = m_head_size - (int)m_pos;
		if(n>len) n = len;
		if((m_pos+(ssize64_t)m_file_size)!=_file.seek(m_pos+m_file_size,SEEK_SET))
			return read_len;
		ret=_file.read(buf,n);
		if(ret>0)
		{
			___rdbs_unencrypt(buf,n);
			m_pos+=ret;
			read_len+=ret;
			len-=ret;
		}
		if(ret!=n)
			return read_len;
	}
	
	if(len>0)
	{
		assert(m_pos>=m_head_size);
		if(m_pos!=_file.seek(m_pos,SEEK_SET))
			return read_len;
		ret=_file.read(buf+read_len,len);
		if(ret>0)
		{
			m_pos+=ret;
			read_len+=ret;
			len-=ret;
		}
	}

	return read_len;
}

static const char* ___rdbs_get_name_by_path(const char* path)
{
	if(NULL==path)
		return path;
	const char* ptr1=strrchr(path,'/');
	const char* ptr2=strrchr(path,'\\');
	if(NULL==ptr1)
	{
		if(NULL==ptr2)
			return path;
		else
			return ptr2+1;
	}
	else
	{
		if(NULL==ptr2)
			return ptr1+1;
		else
		{
			if(ptr1>ptr2)
				return ptr1+1;
			else
				return ptr2+1;
		}
	}
}

int RDBFile64Simple::normal_to_rdbs_file(const char* path,const char* to_path)
{
	File64 file;
	if(0!=file.open(path,F64_RDWR))
		return -1;
	char buf[256];
	size64_t file_size = file.seek(0,SEEK_END);
	file.seek(0,SEEK_SET);
	int endian,ver;
	int pos=0;
	if(file_size>=188)
	{
		if(188==file.read(buf,188))
		{
			if(0==memcmp(RDBS_STX,buf,40))
			{
				printf("--- is rdbs file return \n");
				return 1;
			}
		}
	}
	int ret;
	int head_size = 0;
	ssize64_t n = file_size/10;
	if(n<256) head_size=256;
	else if(n>102400) head_size=102400;
	else head_size=(int)n;
	char *data = new char[10240];

	endian = RDB_ENDIAN;
	ver = 1;
	memcpy(buf+pos,RDBS_STX,40);
	pos += 40;
	memcpy(buf+pos,(char*)&endian,4);
	pos += 4;
	memcpy(buf+pos,(char*)&ver,4);
	pos += 4;
	memcpy(buf+pos,(char*)&head_size,4);
	pos += 4;
	memcpy(buf+pos,(char*)&file_size,8);
	pos += 8;
	strcpy(buf+pos,___rdbs_get_name_by_path(path));
	pos += 128;
	assert(188==pos);

	//copy data to end
	ssize64_t copy_size = head_size;
	ssize64_t to_pos = file_size;
	ssize64_t from_pos = 0;
	if((size64_t)head_size>file_size)
	{
		copy_size = file_size;
		to_pos = head_size;
	}

	while(from_pos<copy_size)
	{
		ret = 10240;
		if(from_pos+ret>copy_size)
			ret = (int)(copy_size-from_pos);
		if(from_pos!=file.seek(from_pos,SEEK_SET))
			break;
		ret = file.read(data,ret);
		if(ret<=0)
			break;
		from_pos += ret;
		___rdbs_encrypt(data,ret);

		if(to_pos!=file.seek(to_pos,SEEK_SET))
			break;
		if(ret!=file.write(data,ret))
			break;
		to_pos += ret;
	}
	
	delete[] data;
	assert(from_pos==copy_size);
	if(from_pos!=copy_size)
		return -1;

	//write head
	if(0!=file.seek(0,SEEK_SET))
		return -1;
	if(188!=file.write(buf,188))
		return -1;
	file.close();
	if(to_path)
	{
		rename(path,to_path);
	}
	return 0;
}

int RDBFile64Simple::rdbs_to_normal_file(const char* path,const char* to_path)
{
	File64 file;
	if(0!=file.open(path,F64_RDWR))
		return -1;
	char buf[256];
	char name[128];
	size64_t end_size = file.seek(0,SEEK_END);
	file.seek(0,SEEK_SET);
	int endian,ver;
	int pos=0;
	if(end_size>=188)
	{
		if(188==file.read(buf,188))
		{
			if(0!=memcmp(RDBS_STX,buf,40))
			{
				printf("--- is normal file return \n");
				return 1;
			}
		}
	}
	else
	{
		return 1;
	}

	int head_size;
	size64_t file_size;
	pos = 40;
	memcpy(&endian,buf+pos,4);
	pos+=4;
	memcpy(&ver,buf+pos,4);
	pos+=4;
	memcpy(&head_size,buf+pos,4);
	pos+=4;
	memcpy(&file_size,buf+pos,8);
	pos+=8;
	memcpy(name,buf+pos,128);
	pos+=128;
	assert(188==pos);
	if(1!=ver || head_size<188 || file_size==(size64_t)-1)
		return -1;
	
	char *data = new char[10240];
	int ret = 0;
	size64_t copy_size = head_size;
	ssize64_t to_pos = 0;
	ssize64_t from_pos = file_size;
	if(copy_size>file_size)
	{
		copy_size = file_size;
		from_pos = head_size;
	}

	while(to_pos<(ssize64_t)copy_size)
	{
		ret = 10240;
		if(to_pos+ret>(ssize64_t)copy_size)
			ret = (int)(copy_size-to_pos);
		if(from_pos!=file.seek(from_pos,SEEK_SET))
			break;
		ret = file.read(data,ret);
		if(ret<=0)
			break;
		from_pos += ret;
		___rdbs_unencrypt(data,ret);

		if(to_pos!=file.seek(to_pos,SEEK_SET))
			break;
		if(0!=file.write_n(data,ret))
			break;
		to_pos += ret;
	}
	delete[] data;
	assert(to_pos==(ssize64_t)copy_size);
	if(to_pos<(ssize64_t)copy_size)
		return -1;

	//ÐÞ¼ôÎÄ¼þ,SetEndOfFile()/ftruncate()
	file.resize(file_size);
	file.close();

	if(to_path)
	{
		rename(path,to_path);
	}
	return 0;
}
