#pragma once

#include <time.h>

//检查i是否base(一般是当前时间curr)的后面,主要是考虑溢出从0计算的情况.
//假定一般两时间差不会很大很大.
int _timer_distance(unsigned int t,unsigned int base);
bool _timer_after(unsigned int t,unsigned int base); 

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
	#include <windows.h>

	unsigned long long GetUTickCount();
	//windows 下睡眠0毫秒实际为忙等待，睡眠1毫秒时基本要2毫秒的周期

#else
	
	#include <unistd.h>

	#define Sleep(dwMilliseconds) usleep(dwMilliseconds*1000)

	typedef unsigned short      WORD;
	typedef unsigned long       DWORD;
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
#endif

