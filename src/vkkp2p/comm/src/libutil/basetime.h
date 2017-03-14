#pragma once

#include <time.h>

//���i�Ƿ�base(һ���ǵ�ǰʱ��curr)�ĺ���,��Ҫ�ǿ��������0��������.
//�ٶ�һ����ʱ����ܴ�ܴ�.
int _timer_distance(unsigned int t,unsigned int base);
bool _timer_after(unsigned int t,unsigned int base); 

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
	#include <windows.h>

	unsigned long long GetUTickCount();
	//windows ��˯��0����ʵ��Ϊæ�ȴ���˯��1����ʱ����Ҫ2���������

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
	//linux ��˯��0����ʵ������Ϊ1���룬˯��1����ʱ����Ҫ2���������
#endif

