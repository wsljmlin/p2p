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
	void reduce(); //�ü�
	int getbsize() const { return bsize;}
private:
	int __i;
	int bsize; //block ��С
	int outs;
	int maxouts;  //����ʱ�������������
	BlockList ls;
};

//����һ�����������ڴ�أ�ֻ����ָ��size���ڴ�飬������С��ʵʱnew��delete
//�������Ϊÿ20�����һ�λ�����������ʱ���ֻ���������20���ڵ����������������5��
//ʹ�õ�Timer��ʱ��������Ҫ��Timer��ʱ����������г�ʼ��
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
