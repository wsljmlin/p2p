#pragma once

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) typedef char __static_assert_t[ (expr) ]
#endif


#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>
#else
//注意:编译时一定要_FILE_OFFSET_BITS
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE
//#define _FILE_OFFSET_BITS 64

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


//似乎android系统的ndk不支持大文件
//STATIC_ASSERT(sizeof(off_t)==8);

#define INVALID_HANDLE_VALUE -1
#endif



//注意：暂时使用fstream实现
//gcc -D_FILE_OFFSET_BITS=64

//windows: CreateFile() ; SetFilePointer(); ReadFile(); WriteFile();FlushFileBuffers();
//linux: open(); lseek(); read(); write(); flush;

typedef unsigned long long size64_t;
typedef long long ssize64_t;

#define F64_READ        0x01
#define F64_WRITE       0x02
#define F64_RDWR        0x03
#define F64_TRUN        0x04


STATIC_ASSERT(sizeof(ssize64_t)==8);
STATIC_ASSERT(sizeof(size64_t)==8);
class File64
{
public:
	File64(void);
	~File64(void);

	int open(const char* path,int mode);
	int close();
	ssize64_t seek(ssize64_t distance,int smode);
	int write(const char *buf,int len);
	int write_n(const char *buf,int len);
	int read(char *buf,int len);
	int flush();
	ssize64_t tell();
	bool is_open() const { return fd != INVALID_HANDLE_VALUE; } 
	size64_t get_file_size()
	{
		if(!is_open())
			return 0;
		ssize64_t pos = tell();
		ssize64_t size = seek(0,SEEK_END);
		seek(pos,SEEK_SET);
		return (size64_t)size;
	}
	int resize(ssize64_t size);

	static int remove_file(const char* path);
	static size64_t get_file_size(const char* path);
	static int rename_file(const char* from,const char* to) {return rename(from,to);}
	static int resize(const char* path,ssize64_t size);
private:
#ifdef _WIN32
	HANDLE fd;
#else
	int fd;
#endif
};


//$ cat a.c
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE
//#define _FILE_OFFSET_BITS 64
//
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <stdio.h>
//#include <fcntl.h>
//
//int main()
//{
//    printf("sizeof(off_t) = %d", sizeof(off_t));
//    int fd = open("test", O_WRONLY | O_CREAT | O_LARGEFILE, 0600);
//    if ( fd > 0 )
//    {
//        off_t l = 2047LL * 1024 * 1024 * 1024;
//        off_t l_new = lseek(fd, l, SEEK_SET);
//        write(fd, &fd, sizeof(fd));
//        close(fd);
//    }
//}
//
//$ gcc a.c -o a -std=c99
//$ ./a
//sizeof(off_t) = 8
//$ ls -l test
//-rw-------    1 root root  2197949513732 Jan 17 14:26 test
