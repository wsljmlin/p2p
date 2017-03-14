#include "FileBlock.h"
#include "Setting.h"

#define DEFAULT_FILE_BLOCKS 100

FileBlockPool::FileBlockPool(void)
: m_amount(0)
, m_out(0)
{
}

FileBlockPool::~FileBlockPool(void)
{
	assert(m_mpb_using.empty());
	for (BlockIter it = m_mpb_using.begin(); it != m_mpb_using.end(); it++)
		delete it->second;
	for (BlockIter it = m_mpb_free.begin(); it != m_mpb_free.end(); it++)
		delete it->second;
	m_amount -= (int)m_mpb_using.size();
	m_amount -= (int)m_mpb_free.size();
	assert(m_amount==0);
}
FileBlock *FileBlockPool::get_fileblock(const hash_t& hash,unsigned int index,unsigned int block_size)
{
	hash_t key = hash;
	unsigned char *p = (unsigned char*)key.buffer();
	key.write_little_endian_32(p+p[5]+6,index);
	p[5] += 4;
	BlockIter it = m_mpb_using.find(key);
	if(it!=m_mpb_using.end())
	{
		assert(block_size<=it->second->buflen);
		it->second->ref++;
		return it->second;
	}
	FileBlock *fb = NULL;
	fb = get_fromfree(key,block_size);
	if(fb)
	{
		fb->ref++;
		m_mpb_using[key] = fb;
		return fb;
	}	
	if (block_size < SettingSngl::instance()->get_block_size())
		block_size = SettingSngl::instance()->get_block_size();

	//堆内在分配过多可以失败
	fb = new FileBlock(block_size);
	if(NULL==fb)
		return NULL;
	if(NULL==fb->buf)
	{
		delete fb;
		return NULL;
	}
	fb->key = key;
	fb->ref++;
	m_mpb_using[key] = fb;
	m_amount++;
	reduce();
	return fb;
}
void FileBlockPool::put_fileblock(FileBlock* block)
{
	block->ref--;
	if (block->ref == 0)
	{
		m_mpb_using.remove(block->key);
		m_mpb_free[block->key] = block;
		reduce();
	}
}
FileBlock *FileBlockPool::get_fromfree(const hash_t& key,unsigned int block_size)
{
	FileBlock *fb=NULL;
	//取一个相同KEY的用
	BlockIter it = m_mpb_free.find(key);
	if(it != m_mpb_free.end())
	{
		assert(block_size<=it->second->buflen);
		fb = it->second;
		m_mpb_free.erase(it);
	}
	if(NULL==fb && m_amount>=DEFAULT_FILE_BLOCKS)
	{
		//尝试取一个不同KEY的用
		for(it=m_mpb_free.begin();it!=m_mpb_free.end();++it)
		{
			if(it->second->buflen>=block_size)
			{
				fb = it->second;
				m_mpb_free.erase(it);
				fb->reset();
				fb->key=key;
				break;
			}
		}
	}
	return fb;
}

void FileBlockPool::reduce()
{
	if(m_amount > (DEFAULT_FILE_BLOCKS+20))
	{
		//最少保留5块空闲
		while (m_amount > DEFAULT_FILE_BLOCKS && m_mpb_free.size()>5)
		{
			BlockIter it = m_mpb_free.begin();
			delete it->second;
			m_mpb_free.erase(it);
			m_amount--;
		}
	}
}

