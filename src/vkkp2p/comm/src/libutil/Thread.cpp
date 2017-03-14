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
		//���²��ж��Ƿ�INVALID_HANDLE_VAULE
		//���svc��ʹ��m_handle[i]����ע�⿼�ǿ���m_handle[i]��δ����ֵ�Ĳ���ȫ���
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
	//���ȫ���̶߳��Ѿ��˳�����reset()
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
		//ע��:������,������ú�,m_pthreadt[i]δ����ֵǰ���ܾ��Ѿ�ִ����_svc(),�⿴��ͬƽ̨��ʵ��.Ҫ��ϸ�����������
		//����ͨ��_icreate_state״̬��������.0״̬����,�������Ϊ1״̬,1״̬��_svc()���Լ������н�����ֵ
		while(0!=_icreate_state)
			usleep(1000);
		//Ŀǰ����pthread_create()����valgrind���߲��Ի���144Bй©����ʹpthread_join() or pthread_detache()����Ч��pthread_exit()������й©��
		if(0 != pthread_create(&m_handle[i],NULL,_work_T,(void*)(new ThreadData_t(this,i))))
		{
			//return value EAGAIN ��ʾϵͳ���ƴ������߳�
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
		pthread_join(m_handle[i],NULL);//һ���̲߳�Ӧ������̵߳ȴ�,����ֻ��һ���ɹ�,�����ķ���ESRCH
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
	//�����ԣ���������Ż��˳�ʱ�����ͷ��߳���Դ������Ҫpthread_join(hthread,NULL);
	//pthread_detach(pthread_self()); //join()����������
	//pthread_exit(NULL); //���ڴ�й©
	//return (void*)ret;
	return 0;
}

#endif

