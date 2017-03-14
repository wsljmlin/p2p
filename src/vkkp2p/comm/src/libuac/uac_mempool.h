#pragma once
#include "uac_cyclist.h"
#include "uac_SynchroObj.h"
#include "uac_Singleton.h"

namespace UAC
{
#define UAC_THREAD_CORE 1
#define UAC_THREAD_APP 2
class mempool;
class memblock
{
	friend class mempool;
private:
	memblock(int size)
		:bufsize(size)
		,datasize(0)
		,datapos(0)
	{
		buffer = new char[size];
	}
	~memblock(void)
	{
		delete[] buffer;
	}
public:
	char *buffer;
	int bufsize;
	int datasize;
	int datapos;
public:
	static memblock* alloc(int threadtoken);
	void free(int threadtoken);
};
//使用3个列表，1个线程1使用，1个线程2使用，1个公共表，公共表用于两个线程表的平衡，公共表仅在执行平衡操作时使用锁
class mempool
{
public:
	mempool(void);
	~mempool(void);
	typedef CriticalSection Mutex;

public:
	int init();
	void fini();
	memblock* get_block(int threadtoken);
	void put_block(memblock* block,int threadtoken);

private:
	list<memblock*> m_ls[3];
	Mutex m_mt;
	bool m_binit;
	int m_bufsize;
	int m_block_num;
};

typedef Singleton<mempool> mempoolsngl;
}

