#include "uac_mempool.h"
#include <assert.h>
#include "uac_basetypes.h"

namespace UAC
{
//*****************************************************
memblock* memblock::alloc(int threadtoken)
{
	memblock* b = mempoolsngl::instance()->get_block(threadtoken);
	return b;
}
void memblock::free(int threadtoken)
{
	mempoolsngl::instance()->put_block(this,threadtoken);
}
//*****************************************************
mempool::mempool(void)
:m_binit(false)
,m_bufsize(2048)
,m_block_num(0)
{
}

mempool::~mempool(void)
{
}

int mempool::init()
{
	if(m_binit)
		return -1;
	m_binit = true;

	m_bufsize = 2048;
	for(int i=0;i<3;++i)
	{
		for(int j=0;j<5000;j++)
		{
			m_ls[i].push_back(new memblock(m_bufsize));
		}
		m_block_num += m_ls[i].size();
	}
	return 0;
}
void mempool::fini()
{
	memblock *_block;
	assert(m_block_num == m_ls[0].size() + m_ls[1].size() + m_ls[2].size());
	if(!m_binit)
		return;
	m_binit = false;
	
	for(int i=0;i<3;++i)
	{
		while((_block=m_ls[i].front()))
		{
			m_ls[i].pop_front();
			delete _block;
		}
	}
	m_block_num = 0;
}
memblock* mempool::get_block(int threadtoken)
{
	memblock *_block;
	//0列为公共
	assert(1==threadtoken || 2==threadtoken);
	if(m_ls[threadtoken].empty())
	{
		TLock<Mutex> l(m_mt);
		int n=0;
		while((_block=m_ls[0].front()))
		{
			m_ls[0].pop_front();
			m_ls[threadtoken].push_back(_block);
			n++;
			if(n>1000)
				break;
		}
		if(n<1000)
		{
			UACLOG("# mempool new block(%d) - n=%d !! \n",m_bufsize,1000-n);
			for(int i=n;i<1000;++i)
			{
				m_ls[threadtoken].push_back(new memblock(m_bufsize));
				m_block_num++;
			}
		}
	}
	_block = m_ls[threadtoken].front();
	m_ls[threadtoken].pop_front();
	return _block;
}
void mempool::put_block(memblock* block,int threadtoken)
{
	memblock *_block;
	//0列为公共
	assert(1==threadtoken || 2==threadtoken);
	block->datasize = 0;
	block->datapos = 0;
	m_ls[threadtoken].push_back(block);
	if(m_ls[threadtoken].size()>7000)
	{
		TLock<Mutex> l(m_mt);
		for(int i=0;i<1000;++i)
		{
			_block = m_ls[threadtoken].front();
			m_ls[threadtoken].pop_front();
			if(m_ls[0].size()<10000)
				m_ls[0].push_back(_block);
			else
			{
				delete _block;
				m_block_num--;
				UACLOG("# mempool free block(%d) !! \n",m_bufsize);
			}
		}
	}

}

}

