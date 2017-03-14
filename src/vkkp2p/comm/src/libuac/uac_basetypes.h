#pragma once

#include <ctype.h>
#include <time.h>
#include <stdio.h>



#ifdef _WIN32
	// 4482 4267 4018 4800 4311 4312 4102
	#pragma warning(disable:4996)
	#include <winsock2.h>
	#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
	#include <windows.h>
	typedef int socklen_t;
#else
	#include <unistd.h>
	#include <string.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <fcntl.h> 
	#include <arpa/inet.h>
	#include <net/if.h>
#endif

namespace UAC
{

#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr) typedef char __static_assert_t[ (expr) ]
#endif


//检查curr(一般是当前时间curr)是否base的后面,主要是考虑溢出从0计算的情况.
//假定一般两时间差不会很大很大.
int _timer_distance(unsigned int curr,unsigned int base);
bool _timer_after(unsigned int curr,unsigned int base); 


typedef unsigned long long  ULONGLONG;
typedef unsigned long DWORD;

inline void void_printf(const char* _Format, ...){}
//#define UACLOG void_printf
#define UACLOG printf
#define D printf("*--FILE[%s] FUNC[%s] LINE[%d]--*\n",__FILE__,__FUNCTION__,__LINE__);

#define UAC_MIN(x,y) (x)<(y)?(x):(y)

#ifdef _WIN32
	unsigned long long GetUTickCount();
	//windows 下睡眠0毫秒实际为忙等待，睡眠1毫秒时基本要2毫秒的周期
	//#define MSleep Sleep

	char *strcasestr(const char *haystack, const char *needle);

#else
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1

	#ifndef MAX_PATH
		#define MAX_PATH 512
	#endif


	#define TRUE 1
	#define FALSE 0

	typedef int SOCKET;
	typedef unsigned long       DWORD;
	typedef int                 BOOL;
	typedef unsigned short      WORD;
	typedef long                LONG;

	#define closesocket(s) close(s)
	#define stricmp strcasecmp
	//#define Sleep(dwMilliseconds) MSleep(dwMilliseconds)
	#define Sleep(dwMilliseconds) usleep(dwMilliseconds*1000)

	typedef struct _SYSTEMTIME {
		WORD wYear;
		WORD wMonth;
		WORD wDayOfWeek;
		WORD wDay;
		WORD wHour;
		WORD wMinute;
		WORD wSecond;
		WORD wMilliseconds;
	} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

	inline int GetLocalTime(LPSYSTEMTIME stm)
	{
		time_t tt = time(0);
		tm *t = localtime(&tt);
		memset(stm,0,sizeof(SYSTEMTIME));
		if(t)
		{
			stm->wYear = t->tm_year + 1900;
			stm->wMonth = t->tm_mon + 1;
			stm->wDay = t->tm_mday;
			stm->wHour = t->tm_hour;
			stm->wMinute = t->tm_min;
			stm->wSecond = t->tm_sec;
			stm->wMilliseconds = 0;
			stm->wDayOfWeek = t->tm_wday;
			return 0;
		}
		return -1;
	}

	unsigned long long GetUTickCount();
	inline DWORD GetTickCount(){ return (DWORD)(GetUTickCount()/1000);}
	//linux 下睡眠0毫秒实际周期为1毫秒，睡眠1毫秒时基本要2毫秒的周期
	//void MSleep(unsigned int msec);


	char* strlwr(char* str);

#endif


#define GETSET(type, name, name2) \
protected: type name; \
public: type& get##name2() { return name; } \
	void set##name2(const type& a##name2) { name = a##name2; }


}

//#include "uac_cyclist.h"
//#include "uac_rbtmap.h"

