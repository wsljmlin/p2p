#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //增加这个只为方便在其它地方编译时不会遇到先windows.h再winsock2.h的重定义问题。
#include <windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#endif

namespace UAC
{

class Thread;
typedef struct tagThreadData
{
	Thread *thr;
	int e;
	tagThreadData(Thread *athr,int ae):thr(athr),e(ae){}
}ThreadData_t;

#ifdef _WIN32

class Thread
{
public:
	Thread(void);
	virtual ~Thread(void);
public:
	int activate(int n=1);
	int wait(DWORD milliseconds=INFINITE);
	
	virtual int work(int e); //e指明是第几条线程，从0算起
	static DWORD WINAPI _work_T(LPVOID p);
protected:
	HANDLE *m_handle;
	DWORD *m_thrId;
	int m_nThr;
};

#else

class Thread
{
public:
	Thread(void);
	virtual ~Thread(void);
public:
	int activate(int n=1);
	int wait();
	
	virtual int work(int e);
	static void* _work_T(void* p);
	int get_work_i(); //定义多条线程从0开始编号，0表示第1条，返回的就是这个值
private:
	static int _icreate_state;
protected:
	pthread_t *m_handle;
	int m_nThr;
};

#endif

}

