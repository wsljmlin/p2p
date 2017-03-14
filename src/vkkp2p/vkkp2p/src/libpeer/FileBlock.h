#pragma once
#include "commons.h"


class FileBlock
{
public:
	FileBlock(int size)
	{
		assert(size>0);
		buf = new char[size];
		buflen = size;
		ref = 0;
		begin = 0;
		end = 0;
	}
	~FileBlock(void)
	{
		if(buf)
			delete[] buf;
	}
	void reset()
	{
		assert(0==ref);
		begin = end = ref = 0;
	}
	int write(const char* b,unsigned int len,unsigned int offset)
	{
		if(offset>end)
			return 0;
		unsigned int tmp = end - offset; //去掉重叠部分
		if(len<=tmp)
			return 0;
		len -= tmp;
		assert(end+len<=buflen);
		memcpy(buf+end,b+tmp,len);
		end += len;
		return len;
	}
public:
	char* buf;
	unsigned int buflen;
	int ref;
	unsigned int begin;
	unsigned int end;
	hash_t key;
};

class FileBlockPool
{
	friend class Singleton<FileBlockPool>;
private:
	FileBlockPool(void);
	~FileBlockPool(void);
	typedef map<hash_t,FileBlock*> BlockMap;
	typedef BlockMap::iterator BlockIter;
public:
	FileBlock *get_fileblock(const hash_t& hash,unsigned int index,unsigned int block_size);
	void put_fileblock(FileBlock* block);
private:
	FileBlock *get_fromfree(const hash_t& key,unsigned int block_size);
	void reduce();
private:
	BlockMap m_mpb_using;
	BlockMap m_mpb_free;
	int m_amount;
	int m_out;
};
typedef Singleton<FileBlockPool> FileBlockPoolSngl;

