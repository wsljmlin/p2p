#include "Thread.h"

#include <assert.h>
#ifdef _WIN32

Thread::Thread(void)
: m_handle(NULL)
, m_thrId(NULL)
, m_nThr(0)
{
}

Thread::~Thread(void)
{
	assert(0==m_nThr);
	if(m_nThr)
	{
		for(int i=0;i<m_nThr;++i)
		{
			if(m_handle[i] == 0)
				continue;
			CloseHandle(m_handle[i]);
			m_handle[i] = 0;
		}
		delete[] m_handle;
		delete[] m_thrId;
		m_handle = 0;
		m_thrId = 0;
		m_nThr = 0;
	}
}


int Thread::activate(int n/*=1*/)
{
	assert(0==m_nThr);
	if(m_nThr>0 || n<=0)
		return 0;

	if(n>100) n=100;
	m_handle = new HANDLE[n];
	m_thrId = new DWORD[n];
	if(!m_handle || !m_thrId)
	{
		if(m_handle) delete[] m_handle;
		if(m_thrId) delete[] m_thrId;
		return 0;
	}
	m_nThr = n;
	for(int i=0;i<m_nThr;++i)
	{
		//以下不判断是否INVALID_HANDLE_VAULE
		//如果svc中使用m_handle[i]，请注意考虑可能m_handle[i]还未被赋值的不安全情况
		m_handle[i] = CreateThread(0,0,_work_T,LPVOID(new ThreadData_t(this,i)),0,&m_thrId[i]);
	}
	return 0;
}
int Thread::wait(DWORD milliseconds/*=INFINITE*/)
{
	
	DWORD ms1,ms2,ret;
	int i=0;
	bool bempty = true;
	for(i=0;i<m_nThr;++i)
	{
		if(m_handle[i] == 0)
			continue;
		ms1 = ::GetTickCount();
		ret = ::WaitForSingleObject(m_handle[i],milliseconds);
		if(WAIT_OBJECT_0 == ret)
		{
			CloseHandle(m_handle[i]);
			m_handle[i] = 0;
		}
		else if(WAIT_TIMEOUT == ret)
			break;

		ms2 = ::GetTickCount();
		if(ms2 >= ms1)
			milliseconds -= ms2 - ms1;
		else
			milliseconds -= 0x7fffffff - ms1 +  ms2;
		if(milliseconds < 0)
			break;
	}
	//如果全部线程都已经退出，则reset()
	for(i=0;i<m_nThr;++i)
	{
		if(m_handle[i] != 0)
		{
			bempty = false;
			break;
		}
	}
	if(bempty)
	{
		if(m_nThr)
		{
			delete[] m_handle;
			delete[] m_thrId;
			m_handle = 0;
			m_thrId = 0;
			m_nThr = 0;
		}
	}
	return 0;
}


int Thread::work(int e)
{
	return 0;
}
DWORD WINAPI Thread::_work_T(LPVOID p)
{
	ThreadData_t* td = (ThreadData_t*)p;
	int ret = td->thr->work(td->e);
	delete td;
	return ret;
}
#elif defined _ECOS_8203

#else

int Thread::_icreate_state = 0;
Thread::Thread(void): 
m_handle(NULL)
, m_nThr(0)
{
}
Thread::~Thread(void)
{
	assert(0==m_nThr && NULL==m_handle);
	if(m_handle)
	{
		delete[] m_handle;
		m_handle = NULL;
	}
}

int Thread::activate(int n/*=1*/)
{
	if(n < 1 || m_nThr>0)
		return -1;

	if(n>100) n=100;
	m_nThr = n;
	m_handle = new pthread_t[m_nThr];
	for(int i=0;i<m_nThr;++i)
	{
		//注意:经测试,这里调用后,m_pthreadt[i]未被赋值前可能就已经执行了_svc(),这看不同平台的实现.要仔细考虑这个问题
		//这里通过_icreate_state状态单步控制.0状态创建,创建完成为1状态,1状态后_svc()可以继续运行将赋回值
		while(0!=_icreate_state)
			usleep(1000);
		//目前测试pthread_create()，用valgrind工具测试会有144B泄漏，即使pthread_join() or pthread_detache()都无效，pthread_exit()会增加泄漏。
		if(0 != pthread_create(&m_handle[i],NULL,_work_T,(void*)(new ThreadData_t(this,i))))
		{
			//return value EAGAIN 表示系统限制创建新线程
			_icreate_state = 0;
			m_handle[i] = 0;
		}
		_icreate_state = 1;
	}
	return 0;
}
int Thread::wait()
{
	if(m_nThr < 1)
		return 0;
	for(int i=0;i<m_nThr;++i)
	{
		pthread_join(m_handle[i],NULL);//一个线程不应被多个线程等待,否则只有一个成功,其它的返回ESRCH
	}
	
	if(m_handle)
	{
		delete[] m_handle;
		m_handle = NULL;
	}
	m_nThr = 0;
	return 0;
}

int Thread::work(int e)
{
	return 0;
}
void* Thread::_work_T(void* p)
{
	while(1!=_icreate_state)
		usleep(1000);
	_icreate_state = 0;
	ThreadData_t* td = (ThreadData_t*)p;
	//int ret = td->thr->work(td->e);
	td->thr->work(td->e);
	delete td;
	//经测试，必须脱离才会退出时立即释放线程资源。否则要pthread_join(hthread,NULL);
	//pthread_detach(pthread_self()); //join()会立即返回
	//pthread_exit(NULL); //会内存泄漏
	//return (void*)ret;
	return 0;
}

#endif

