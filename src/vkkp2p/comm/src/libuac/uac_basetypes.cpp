#include "uac_basetypes.h"

#ifndef _WIN32
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#endif


// int printf(const char *format,[argument]);
// format ��������ĸ�ʽ�������ʽΪ��
// %[flags][width][.perc] [F|N|h|l]type

namespace UAC
{

int _timer_distance(unsigned int curr,unsigned int base)
{
	return (int)curr - (int)base;
}
bool _timer_after(unsigned int curr,unsigned int base)
{
	return (((int)(curr) - (int)(base)) >= 0 );
}


#ifdef _WIN32
unsigned long long GetUTickCount()
{
	static bool b=false;
	static unsigned long long begin_count=0;
	static unsigned long long rate=0;
	static unsigned long long curr_count=0;
	static unsigned long long tick=0;
	static unsigned long long tmp=0;
	static LARGE_INTEGER litmp;
	if(!b)
	{
		b = true;
		QueryPerformanceFrequency(&litmp);//���ʱ��Ƶ��
		rate = litmp.QuadPart;
		QueryPerformanceCounter(&litmp);//��ó�ʼֵ
		begin_count = litmp.QuadPart;
	}
	QueryPerformanceCounter(&litmp);//
	curr_count = litmp.QuadPart - begin_count;
	tick = (curr_count/rate)*1000000 + (unsigned long long)((curr_count%rate)/(double)rate * 1000000);
	//tick = (long long)(curr_count/(double)rate * 1000000);
	return tick;
}
char* strcasestr(const char* haystack,const char* needle)
{
    int i=0;
    while(*haystack&&*needle)
	{           
        while(*haystack==*needle||\
              *haystack==((*needle>='a'&&*needle<='z')?*needle-32:*needle)||\
              *haystack==((*needle>='A'&&*needle<='Z')?*needle+32:*needle))
		{
			if(!*(++needle))return (char*)haystack-i;
			if(!*(++haystack))return 0;
			i++;
        }
        needle-=i;
        haystack-=(i-1);
        i=0;
    }
    return 0;
}

#else

//******************************************************GetUTickCount()  *******************************************

unsigned int g_uptime = 0;
void sigalrm_handler(int signo)
{
    switch (signo) {
        case SIGALRM:
            g_uptime++; //ϵͳ��ʱ��������,ִ��n�ο������n����
			//printf("uptime=%d / %.3f \n",g_uptime,(double)GetUTickCount()/1000000); 
            alarm(1);
            break;
        default:
            break;
    }
}
inline unsigned int get_uptime()
{
    static bool b = false;
    if(!b)
    {
        b = true;
        signal(SIGALRM,sigalrm_handler);
        alarm(1);
    }
    return g_uptime;
}
inline void set_uptime(unsigned int a)
{
	g_uptime = a;
}
//
//#include <sys/sysinfo.h>
//inline unsigned int get_uptime()
//{
//    unsigned int last_uptime=0;
//    static struct sysinfo si;
//    sysinfo(&si);
//    if(0==last_uptime)
//        last_uptime = si.uptime;
//    
//    return si.uptime - last_uptime;;
//}

unsigned long long GetUTickCount()
{	
	static unsigned int last_sec=0;
	static unsigned int sec=0,uptime=0,tmp=0;
	static unsigned long long ret=0;
	static timeval tv={0,0};
	
	gettimeofday(&tv,0);
	if(0==last_sec)
		last_sec = tv.tv_sec;

	//���ʱ�������
	if(tv.tv_sec>=(int)last_sec)
		sec = tv.tv_sec-last_sec;
	else
		sec = 0;
	//ʹ��g_uptime����Ϊ�ж�ϵͳʱ���Ƿ����ù�
	uptime = get_uptime();
	tmp = sec>uptime?(sec-uptime):(uptime-sec);
	if(tmp>2)
	{
		printf("#****** time of day changed! \n");
		last_sec = tv.tv_sec - uptime;
	}
	else
	{
		//�����������Ϊϵͳ��ʱ����������
		set_uptime(sec);
	}

	ret = ((unsigned long long)(tv.tv_sec-last_sec))*1000000 + tv.tv_usec;
	return ret;
}
//******************************************************GetUTickCount() end *******************************************
//
//void MSleep(unsigned int msec)
//{
//	//usleep(msec*1000);
//	struct timespec tv;
//	tv.tv_sec = msec/1000;
//	tv.tv_nsec = (msec%1000)*1000000;
//	while(1)
//	{
//		/* ˯��ʱ��ָ������tv���档�����һ���ź��жϣ���ʣ���˯��ʱ���ٷŻ�tv��*/
//		int ret = nanosleep(&tv,&tv);
//		if(0==ret)
//			return;  /* ��������˯��ʱ�䣻���й�����ɡ�*/
//		else if(EINTR==errno)
//			continue;/* �ź��жϣ����ԡ�*/
//		else
//		{
//			/* ��������������*/
//			assert(0);
//			return;
//		}
//	}
//}


//string   to   low
char* strlwr(char* str)
{
	char * p = str;
	if(!p)
		return p;
	for(;*p!='\0';++p)
		*p=tolower(*p);
	return str;
}

#endif

}


