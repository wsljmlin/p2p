#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>

class CriticalSection
{
public:
	CriticalSection()
	{
		::InitializeCriticalSection(&cs);
	}
	~CriticalSection()
	{
		::DeleteCriticalSection(&cs);
	}
	void lock()
	{
		::EnterCriticalSection(&cs);
	}
	void unlock()
	{
		::LeaveCriticalSection(&cs);
	}

private:
	CriticalSection(const CriticalSection*);
	CriticalSection& operator=(const CriticalSection&);

private:
	CRITICAL_SECTION cs;
};
typedef CriticalSection Simple_Mutex;
typedef CriticalSection Recursive_Mutex;

class Semaphore
{
public:
	Semaphore() 
	{
		//no signle init;
		h = CreateSemaphore(NULL,0,MAXLONG,NULL);
	}
	~Semaphore() 
	{
		CloseHandle(h);
	}
	int signal() 
	{
		return ReleaseSemaphore(h,1,NULL);
	}
	bool wait() 
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(h,INFINITE) ;
	}
	bool wait(unsigned int millis) 
	{
		return WAIT_OBJECT_0 == WaitForSingleObject(h,millis);
	}


private:
	Semaphore(const Semaphore&);
	Semaphore& operator=(const Semaphore&);
private:
	HANDLE h;
};

#else
//linux:
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

class Simple_Mutex
{
public:
	Simple_Mutex(void) { pthread_mutex_init(&m_mt,NULL);}
	~Simple_Mutex(void) { pthread_mutex_destroy(&m_mt);}
public:
	int lock(){ return pthread_mutex_lock(&m_mt);}
	int unlock(){ return pthread_mutex_unlock(&m_mt);}

private:
	Simple_Mutex(const Simple_Mutex&);
	Simple_Mutex& operator= (const Simple_Mutex&);
private:
	pthread_mutex_t m_mt;
};

class Recursive_Mutex
{
public:
	Recursive_Mutex(void) { 
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE); //PTHREAD_MUTEX_RECURSIVE_NP
		pthread_mutex_init(&m_mt,&attr);
		pthread_mutexattr_destroy(&attr);
	}
	~Recursive_Mutex(void) { pthread_mutex_destroy(&m_mt);}
public:
	int lock(){ return pthread_mutex_lock(&m_mt);}
	int unlock(){ return pthread_mutex_unlock(&m_mt);}

private:
	Recursive_Mutex(const Recursive_Mutex&);
	Recursive_Mutex& operator= (const Recursive_Mutex&);
private:
	pthread_mutex_t m_mt;
};

typedef Recursive_Mutex CriticalSection;

#ifndef INFINITE
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif

#ifndef _OS
class Semaphore
{
public:
	//不进行共享(参数2为0),初值为无信号(参数3为0)
	Semaphore(void) {sem_init(&m_sem,0,0);}
	~Semaphore(void) {sem_destroy(&m_sem);}
public:
	int signal() { return sem_post(&m_sem);}
	bool wait() { return 0==sem_wait(&m_sem);}
	bool trywait() {return 0==sem_trywait(&m_sem);} //非阻塞,-1表未获得信号,0获得信号
	bool wait(unsigned int millis) {
		if(INFINITE==millis) 
			return wait(); 
		else 
			return trywait(); //todo: 暂时只支持只要不无限等待，即try
	}
private:
	Semaphore(const Semaphore&);
	Semaphore& operator= (const Semaphore&);
private:
	sem_t m_sem;
};
#endif //_OS
#endif


template<class T>
class TLock
{
public:
	TLock(T& at)
		: t(at)
	{
		t.lock();
	}
	~TLock()
	{
		t.unlock();
	}
private:
	TLock& operator=(const TLock&);
private:
	T& t;
};
