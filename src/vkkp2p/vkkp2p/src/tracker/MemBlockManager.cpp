#include "MemBlockManager.h"
#include <assert.h>

//******************************************** ***********************************************

MemBlockList::MemBlockList(void)
{ 
	m_size = 0;
	m_head = new MemBlock();
	__join(m_head,m_head);
}
MemBlockList::~MemBlockList(void)
{
	assert(m_head->next==m_head && 0==m_size);
	delete m_head;
}

void MemBlockList::push_back(MemBlock *block)
{
	add_tail(block); //加到尾部
	m_size++;
}
void MemBlockList::push_front(MemBlock *block)
{
	add_head(block); //加到头部
	m_size++;
}
MemBlock *MemBlockList::pop_front()
{
	if(empty())
		return 0;
	m_tmp = m_head->next;
	del(m_tmp);
	m_size--;
	return m_tmp;
}
MemBlock *MemBlockList::pop_back()
{
	if(empty())
		return 0;
	m_tmp = m_head->prev;
	del(m_tmp);
	m_size--;
	return m_tmp;
}


void MemBlockList::push_back_n(MemBlockList& ls)
{
	if(!ls.empty())
	{
		__join(m_head->prev,ls.m_head->next);
		__join(ls.m_head->prev,m_head);
		m_size+=ls.size();
		ls.clear();
	}
}
int MemBlockList::pop_front_n(MemBlockList& ls,int n)
{
	MemBlock *block;
	int i=0;
	for(i=0,m_tmp=m_head->next; m_tmp!=m_head && i<n;m_tmp=m_tmp->next,i++);
	if(i>0 && m_tmp==m_head)
	{
		i--;
		m_tmp = m_tmp->prev;
	}
	if(0==i)
		return 0;
	block = m_tmp->next;
	__join(ls.m_head->prev,m_head->next); //ls 尾部 接 新列的头
	__join(m_tmp,ls.m_head);  //新列的尾 接 到 ls 的头
	__join(m_head,block);
	m_size -= i;
	ls.m_size += i;
	return i;
}

//***********************************************************************************************

MemBlockManager::MemBlockManager(void)
: m_binit(false)
, m_block_num(0)
, m_free_num(0)
, block(NULL)
{
}

MemBlockManager::~MemBlockManager(void)
{
	assert(!m_binit);
}
int MemBlockManager::init()
{
	assert(!m_binit);
	if(m_binit)
		return -1;
	m_binit = true;

	m_block_num = 0;
	MemBlock *block = 0;
	for(int i=0;i<3;++i)
	{
		for(int j=0;j<2000;++j)
		{
			block = new MemBlock(1024);
			m_list[i].push_back(block);
		}
		m_block_num += m_list[i].size();
	}

	m_free_num = m_block_num;
	//DEBUGMSG("-MemBlockManager::init \n");
	return 0;
}
void MemBlockManager::fini()
{
	assert(m_block_num == m_list[0].size() + m_list[1].size() + m_list[2].size());
	//assert(m_free_num == m_block_num);//多线程不对m_free_num安全
	if(!m_binit)
		return;
	m_binit = false;
	MemBlock *block=0;
	int n=0;
	for(int i=0;i<3;++i)
	{
		while((block=m_list[i].pop_front()))
		{
			delete block;
			++n;
		}
	}
	assert(n==m_block_num);
	m_block_num=0;
	m_free_num=0;
	assert(m_block_num == m_list[0].size() + m_list[1].size() + m_list[2].size());
	//DEBUGMSG("-MemBlockManager::fini \n");
}

MemBlock *MemBlockManager::get_block(int size,int token/*=0*/)
{
	assert(1==token || 2==token);
	if(size>1024||(token!=1&&token!=2))
		return 0;
	block=0;
	if(m_list[token].empty())
	{
		MemBlockList ls;
		{
			//加锁
			TLock<Mutex> lock(m_mt);
			m_list[0].pop_front_n(ls,800);
			for(int i=ls.size();i<500 && m_block_num<100000;++i)
			{
				block = new MemBlock(1024);
				if(block) 
				{
					ls.push_back(block);
					m_block_num++;
					m_free_num++;
				}
			}
		}
		m_list[token].push_back_n(ls);
	}
	block = m_list[token].pop_front();
	if(block) m_free_num--;
	return block;
}
void MemBlockManager::put_block(MemBlock* block,int token/*=0*/)
{
	assert(1==token || 2==token);
	if(!block||(token!=1&&token!=2))
		return;

	block->datalen = 0;
	block->datapos = 0;
	m_list[token].push_back(block);
	m_free_num++;
	if(m_list[token].size()>=3000)
	{
		MemBlockList ls;
		m_list[token].pop_front_n(ls,1000);
		{
			//加锁
			TLock<Mutex> lock(m_mt);
			m_list[0].push_back_n(ls);
			try_clear_some();
		}
	}
}
void MemBlockManager::try_clear_some()
{
	if(m_list[0].size() >= 5000)
	{
		MemBlock *block=0;
		int n=0;
		while(n<1000)
		{
			++n;
			block = m_list[0].pop_front();
			if(block)
			{
				delete block;
				m_block_num--;
				m_free_num--;
			}
		}
	}
}


