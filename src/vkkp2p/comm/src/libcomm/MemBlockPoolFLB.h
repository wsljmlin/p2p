#pragma once
#include "MemBlock.h"
#include "Timer.h"

class MemBlockQueue
{
public:
	MemBlockQueue(void);
	~MemBlockQueue(void);
	typedef list<MemBlock*> BlockList;
	typedef BlockList::iterator BlockIter;
public:
	void put(MemBlock* b);
	MemBlock *get();

	int init(int _i,int _bsize,int _lsize);
	void clear();
	void reduce(); //裁减
	int getbsize() const { return bsize;}
private:
	int __i;
	int bsize; //block 大小
	int outs;
	int maxouts;  //近段时间内最大需求量
	BlockList ls;
};

//这是一个定长缓冲内存池：只缓冲指定size的内存块，超出大小的实时new和delete
//缓冲策略为每20秒进行一次缓冲清理，清理时最多只保留比最近20秒内的最大需求量块数多5个
//使用到Timer计时器，所以要在Timer计时器启动后进行初始化
class MemBlockPoolFLB : public MemBlockPoolI
	,public TimerHandler
{
public:
	MemBlockPoolFLB(int* sizes,int n);
	virtual ~MemBlockPoolFLB(void);
public:

	virtual MemBlock* get_block(int size,int token=0);
	virtual void put_block(MemBlock* b,int token=0);

	
	virtual void on_timer(int e);
private:
	MemBlockQueue *m_queue;
	int m_queue_num;
	int m_outs;
};
