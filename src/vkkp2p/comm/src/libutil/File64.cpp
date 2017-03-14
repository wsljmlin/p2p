#include "File64.h"



int File64::write_n(const char *buf,int len)
{
	if(!is_open())
		return -1;
	int n = 0;
	while(len>0)
	{
		n = write(buf,len);
		if(n<=0)
			break;
		len-=n;
		buf+=n;
	}
	if(len>0)
		return -1;
	//assert(0==len);
	return 0;
}

int File64::resize(const char* path,ssize64_t size)
{
	File64 f;
	int ret = -1;
	if(0==f.open(path,F64_RDWR))
	{
		ret = f.resize(size);
		f.close();
	}
	return ret;
}

#ifdef _WIN32
File64::File64(void)
: fd(INVALID_HANDLE_VALUE)
{
}

File64::~File64(void)
{
	close();
}
int File64::open(const char* path,int mode)
{
	if(INVALID_HANDLE_VALUE!=fd)
		return -1;

	DWORD access=0,share=0,createtype=0;
	if(mode&F64_READ)
		access |= GENERIC_READ;
	if(mode&F64_WRITE)
		access |= GENERIC_WRITE;

	share = FILE_SHARE_READ|FILE_SHARE_WRITE;

	if(mode&F64_TRUN)
		createtype = CREATE_ALWAYS /*| TRUNCATE_EXISTING*/;
	else
		createtype = OPEN_EXISTING;

	fd = CreateFileA(path,access,share,NULL,createtype,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==fd)
	{
		DWORD err = GetLastError();
		//perror("CreateFileA():");
		return -1;
	}
	return 0;
}
int File64::close()
{
	if(INVALID_HANDLE_VALUE!=fd)
	{
		CloseHandle(fd);
		fd = INVALID_HANDLE_VALUE;
	}
	return 0;
}
ssize64_t File64::seek(ssize64_t distance,int method)
{
	if(INVALID_HANDLE_VALUE==fd)
		return (ssize64_t)-1;
	//// 其实FILE_BEGIN==SEEK_SET,暂时不作转换
	//if(SEEK_SET==method)
	//	method = FILE_BEGIN;
	//else if(SEEK_CUR==method)
	//	method = FILE_CURRENT;
	//else
	//	method = FILE_END;
	LONG hi = (LONG)(distance>>32);
	SetLastError(NO_ERROR);
	DWORD ret = SetFilePointer(fd,(LONG)distance,&hi,method);
	//因为高位不置0,不能将返回值与INVALID_SET_FILE_POINTER相比.
	if(NO_ERROR!=GetLastError())
		return (ssize64_t)-1;
	distance = (ret&0x00000000ffffffff) | (((ssize64_t)hi)<<32);
	return distance;
}
int File64::write(const char *buf,int len)
{
	if(INVALID_HANDLE_VALUE==fd)
		return -1;
	DWORD realLen = 0;
	if(WriteFile(fd,buf,len,&realLen,NULL))
		return (int)realLen;
	else
		return -1;
}
int File64::read(char *buf,int len)
{
	if(INVALID_HANDLE_VALUE==fd)
		return -1;
	DWORD realLen = 0;
	if(ReadFile(fd,buf,len,&realLen,NULL))
		return (int)realLen;
	else
		return -1;
}
int File64::flush()
{
	if(INVALID_HANDLE_VALUE==fd)
		return -1;
	if(FlushFileBuffers(fd))
		return 0;
	return -1;
}
ssize64_t File64::tell()
{
	if(INVALID_HANDLE_VALUE==fd)
		return -1;
	LONG hi=0;
	SetLastError(NO_ERROR);
	DWORD ret = SetFilePointer(fd,0,&hi,FILE_CURRENT);
	//因为高位不置0,不能将返回值与INVALID_SET_FILE_POINTER相比.
	if(NO_ERROR!=GetLastError())
	{
		DWORD err = GetLastError();
		return (ssize64_t)-1;
	}
	ssize64_t pos = (ret&0x00000000ffffffff) | (((ssize64_t)hi)<<32);
	return pos;
}
int File64::resize(ssize64_t size)
{
	if(INVALID_HANDLE_VALUE==fd)
		return -1;
	if(-1==seek(size,SEEK_SET))
		return -1;
	if(SetEndOfFile(fd))
		return 0;
	return -1;
}
int File64::remove_file(const char* path)
{
	if(TRUE==DeleteFileA(path))
		return 0;
	return -1;
}

size64_t File64::get_file_size(const char* path)
{
	HANDLE fd = CreateFileA(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==fd)
	{
		DWORD err = GetLastError();
		perror("CreateFileA():");
		return 0;
	}
	
	LONG hi = (LONG)0;
	SetLastError(NO_ERROR);
	DWORD ret = SetFilePointer(fd,(LONG)0,&hi,FILE_END);
	CloseHandle(fd);
	//因为高位不置0,不能将返回值与INVALID_SET_FILE_POINTER相比.
	if(NO_ERROR!=GetLastError())
	{
		return (ssize64_t)0;
	}
	size64_t size = (ret&0x00000000ffffffff) | (((ssize64_t)hi)<<32);
	return size;
}
#else
//***************************************************************************
#if defined(_ECOS_8203) || defined(_OS)
#define O_LARGEFILE 0
#endif

File64::File64(void)
: fd(-1)
{
}

File64::~File64(void)
{
	close();
}
int File64::open(const char* path,int mode)
{
	if(-1!=fd)
		return -1;
	int md = O_LARGEFILE;
	if((mode&F64_READ) && (mode&F64_WRITE))
		md |= O_RDWR;
	else if(mode&F64_READ)
		md |= O_RDONLY;
	else if(mode&F64_WRITE)
		md |= O_WRONLY;
	if(mode&F64_TRUN)
		md |= O_CREAT;
	fd = ::open(path, md, 0666);
	if(-1==fd)
		return -1;
	return 0;
}
int File64::close()
{
	if(-1!=fd)
	{
		::close(fd);
		fd = -1;
	}
	return 0;
}
ssize64_t File64::seek(ssize64_t distance,int smode)
{
	return lseek(fd,distance,smode);
}
int File64::write(const char *buf,int len)
{
	return ::write(fd,buf,len);
}
int File64::read(char *buf,int len)
{
	return ::read(fd,buf,len);
}
int File64::flush()
{
	if(-1==fd)
		return -1;
	if(0!=fsync(fd))
	{
		printf("# ***fsync() failed \n");
		return -1;
	}
	//sync();
	return 0;
}
ssize64_t File64::tell()
{
	return lseek(fd,0,SEEK_CUR);
}
int File64::resize(ssize64_t size)
{
	if(-1==fd)
		return -1;
	return ftruncate(fd,size);
}
int File64::remove_file(const char* path)
{
	return unlink(path);
}
size64_t File64::get_file_size(const char* path)
{
	int fd = ::open(path, O_RDONLY | O_LARGEFILE, 0666);
	if(-1==fd)
		return 0;
	ssize64_t size = lseek(fd,0,SEEK_END);
	::close(fd);
	if(-1==size) size = 0;
	return size;
}

#endif

