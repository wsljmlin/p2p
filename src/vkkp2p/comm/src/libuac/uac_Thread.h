#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //�������ֻΪ�����������ط�����ʱ����������windows.h��winsock2.h���ض������⡣
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
	
	virtual int work(int e); //eָ���ǵڼ����̣߳���0����
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
	int get_work_i(); //��������̴߳�0��ʼ��ţ�0��ʾ��1�������صľ������ֵ
private:
	static int _icreate_state;
protected:
	pthread_t *m_handle;
	int m_nThr;
};

#endif

}

