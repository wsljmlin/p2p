#include "MemBlockPoolFLB.h"
#include "sorter.h"

//***********************************
//
//MemBlockQueue:
//
MemBlockQueue::MemBlockQueue(void)
: __i(-1)
, bsize(0)
, outs(0)
, maxouts(0)
{
}
MemBlockQueue::~MemBlockQueue(void)
{
	assert(0==outs);
	clear();
}

void MemBlockQueue::put(MemBlock* b)
{
	assert(b->__i == __i);
	b->datapos = 0;
	b->datalen = 0;
	ls.push_back(b);
	outs--;
}
MemBlock *MemBlockQueue::get()
{
	MemBlock *b = NULL;
	if(!ls.empty())
	{
		b = ls.front();
		ls.pop_front();
	}
	else
	{
		b = new MemBlock(bsize,__i);
		if(!b || !b->buf)
		{
			if(b) delete b;
			return NULL;
		}
	}
	outs++;
	if(maxouts<outs)
		maxouts = outs;
	return b;
}

int MemBlockQueue::init(int _i,int _bsize,int _lsize)
{
	assert(0==bsize);
	__i = _i;
	bsize = _bsize;
	MemBlock *b=NULL;
	for(int i=0;i<_lsize;++i)
	{
		b = new MemBlock(bsize,__i);
		if(!b || !b->buf)
		{
			if(b) delete b;
			break;
		}
		ls.push_back(b);
	}
	return 0;
}
void MemBlockQueue::clear()
{
	for(BlockIter it=ls.begin();it!=ls.end();++it)
		delete *it;
	ls.clear();
}
void MemBlockQueue::reduce() //裁减
{
	assert(maxouts>=outs);
	MemBlock *b = NULL;
	unsigned int n = maxouts-outs + 5; //总共块数最多保持最近最大需求量+5;
	int i=0; //
	while(ls.size()>n)
	{
		b = ls.front();
		ls.pop_front();
		delete b;
		//一次释放10块
		if(++i>=10)
			break;
	}
	maxouts = outs; //maxouts 最大需求量重置
}

//***********************************
//
//MemBlockPoolFLB:
//


MemBlockPoolFLB::MemBlockPoolFLB(int* sizes,int n)
{
	m_outs = 0;
	if(n>0)
	{
		adc::sort_bubble(sizes,n);
		m_queue_num = n;
		m_queue = new MemBlockQueue[m_queue_num];
		//int as[]={1024*4,1024*8,1024*64}; //一定要从小到大排序
		for(int i=0;i<m_queue_num;++i)
			m_queue[i].init(i,sizes[i],1);
		TimerSngl::instance()->register_timer(this,1,20000);
	}
	else
	{
		m_queue=NULL;
		m_queue_num = 0;
	}
}

MemBlockPoolFLB::~MemBlockPoolFLB(void)
{
	assert(m_outs==0);
	if(m_queue)
	{
		TimerSngl::instance()->unregister_all(this);
		for(int i=0;i<m_queue_num;++i)
			m_queue[i].clear();
		delete[] m_queue;
		m_queue = NULL;
		m_queue_num = 0;
	}
}


MemBlock *MemBlockPoolFLB::get_block(int size,int token/*=0*/)
{
	MemBlock *b = NULL;
	if(size<=0)
		return NULL;
	for(int i=0;i<m_queue_num;++i)
	{
		if(size <= m_queue[i].getbsize())
		{
			//在此仅提示用户预计的定长内存块尺寸不适合
			if(size+2048< m_queue[i].getbsize())
				assert(false);

			b = m_queue[i].get();
			break;
		}
	}
	if(NULL==b)
	{
		//在此加assert()仅提示用户预计的定长内存块尺寸不适合
		assert(false);
		//-1表示游离的
		b = new MemBlock(size,-1);
		if(!b || !b->buf)
		{
			if(b) delete b;
			b = NULL;
		}
	}
	if(b) m_outs++;
	return b;
}

void MemBlockPoolFLB::put_block(MemBlock* b,int token/*=0*/)
{
	assert(b);
	if(!b)
		return;
	if(-1==b->__i)
		delete b;
	else
		m_queue[b->__i].put(b);
	m_outs--;
}


void MemBlockPoolFLB::on_timer(int e)
{
	switch(e)
	{
	case 1:
		{
			for(int i=0;i<m_queue_num;++i)
				m_queue[i].reduce();
		}
		break;
	default:
		assert(0);
		break;
	}
}
