#pragma once
#include "MemBlock.h"
#include "SynchroObj.h"

class MemBlockList
{
public:
	MemBlockList(void);
	~MemBlockList(void);

	inline int size() const{return m_size;}
	inline bool empty() const{return m_head->next == m_head;}
	inline void clear() {__join(m_head,m_head);m_size=0;}

	void push_back(MemBlock *block);
	void push_front(MemBlock *block);
	MemBlock *pop_front();
	MemBlock *pop_back();
	
	void push_back_n(MemBlockList& ls);//注意:执行完push_back_n后,ls将被清空
	int pop_front_n(MemBlockList& ls,int n);

private:
	inline void __join(MemBlock *prev,MemBlock *next)
	{
		prev->next = next;
		next->prev = prev;
	}

	inline void add_head(MemBlock *newt)
	{
		__join(newt,m_head->next);//注意，此步一定要放前
		__join(m_head,newt);
	}
	inline void add_tail(MemBlock *newt)
	{
		__join(m_head->prev,newt);//注意，此步一定要放前
		__join(newt,m_head);
	}

	inline void del(MemBlock *entry)
	{
		__join(entry->prev,entry->next);
		entry->next = 0;
		entry->prev = 0;
	}
private:
	MemBlock *m_head,*m_tmp;
	int m_size;
};

//使用3个列表，1个线程1使用，1个线程2使用，1个公共表，公共表用于两个线程表的平衡，公共表仅在执行平衡操作时使用锁
class MemBlockManager : public MemBlockPoolI
{
public:
	MemBlockManager(void);
	virtual ~MemBlockManager(void);
	typedef Simple_Mutex Mutex;
public:
	virtual int init();
	virtual void fini();
	virtual MemBlock* get_block(int size,int token=0);
	virtual void put_block(MemBlock* b,int token=0);
private:
	void try_clear_some();
private:
	Mutex m_mt;
	MemBlockList m_list[3];
	bool m_binit;
	int m_block_num,m_free_num;

	MemBlock *block; //用作临时变量
};
