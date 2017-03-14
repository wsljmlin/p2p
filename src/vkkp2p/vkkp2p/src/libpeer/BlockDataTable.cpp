#include "BlockDataTable.h"
#include <string.h>
#include <assert.h>

#define MIN_UINT32(a,b) (a)<(b)?(a):(b)
#define MAX_UINT32(a,b) (a)>(b)?(a):(b)

BlockDataTable::BlockDataTable(void)
:m_data(NULL)
,m_blocks(0)
,m_block_offset(0)
{
}

BlockDataTable::~BlockDataTable(void)
{
	resize(0,0);
}
bool BlockDataTable::resize(unsigned int blocks,unsigned int block_offset,bool copyold/*=true*/)
{
	if(blocks==m_blocks && m_block_offset == block_offset)
		return true;
	BlockData_t **ptr = NULL;
	//先拷贝可能重叠的数据
	if(blocks>0)
	{
		int n = 0;
		n = (blocks-1)/BDT_SUBNUM + 1;
		ptr = new BlockData_t*[n];
		if(!ptr)
			return false;
		memset(ptr,0,sizeof(BlockData_t*)*n);
		for(int i=0;i<n;++i)
		{
			ptr[i] = new BlockData_t[BDT_SUBNUM];
			if(NULL==ptr[i])
			{
				for(int j=0;j<i;++j)
					delete[] ptr[j];
				delete[] ptr;
				return false;
			}
		}
		if(m_data&&copyold)
		{
			BlockDataTable bt;
			bt.m_data = ptr;
			bt.m_blocks = blocks;
			bt.m_block_offset = block_offset;
			unsigned int i = MAX_UINT32(block_offset,m_block_offset);
			unsigned int n = MIN_UINT32(block_offset+blocks,m_block_offset+m_blocks);
			for(;i<n;++i)
				bt[i] = operator[](i);
			bt.m_data = NULL;
			bt.m_blocks = 0;
			bt.m_block_offset = 0;
		}
	}
	if(m_data)
	{
		int n = (m_blocks-1)/BDT_SUBNUM + 1;
		for(int i=0;i<n;++i)
			delete[] m_data[i];
		delete[] m_data;
		m_data = NULL;
		m_blocks = 0;
		m_block_offset = 0;
	}
	m_data = ptr;
	m_blocks = blocks;
	m_block_offset = block_offset;
	return true;
}
BlockDataTable::BlockData_t& BlockDataTable::get_block(unsigned int i)
{
	assert(i>=m_block_offset && i<m_block_offset+m_blocks);
	if(i>=m_block_offset && i<m_block_offset+m_blocks)
	{
		i-=m_block_offset;
		return m_data[i/BDT_SUBNUM][i%BDT_SUBNUM];
	}
	return _tmp;
}

BlockDataTable& BlockDataTable::operator=(const BlockDataTable& bt)
{
	if(this == &bt)
		return *this;
	if(resize(bt.m_blocks,bt.m_block_offset,false))
	{
		if(m_data)
		{
			int n = (m_blocks-1)/BDT_SUBNUM + 1;
			for(int i=0;i<n;++i)
				memcpy(m_data[i],bt.m_data[i],sizeof(BlockData_t)*BDT_SUBNUM);
		}
	}
	else
		assert(false);
	return *this;
}

int BlockDataTable::write_file(File32& file) const
{
	//最多保存100个
	IndexSize_t arr[100];
	unsigned int size = 0;
	if(m_blocks>0)
	{
		int n = (m_blocks-1)/BDT_SUBNUM + 1;
		for(int i=0;i<n;++i)
		{
			for(int j=0;j<BDT_SUBNUM;++j)
			{
				if(m_data[i][j].size>0)
				{
					arr[size].index = i*BDT_SUBNUM + j;
					arr[size].size = m_data[i][j].size;
					size++;
					if(size>=100)
						break;
				}
			}
			if(size>=100)
				break;
		}
	}
	unsigned int flag = m_blocks + m_block_offset + size + 69;
	FILE32_WRITE_RETURN(file,(char*)&flag,4,-1);
	FILE32_WRITE_RETURN(file,(char*)&m_blocks,4,-1);
	FILE32_WRITE_RETURN(file,(char*)&m_block_offset,4,-1);
	FILE32_WRITE_RETURN(file,(char*)&size,4,-1);
	if(size>0)
		FILE32_WRITE_RETURN(file,(char*)arr,(int)(size*sizeof(IndexSize_t)),-1);
	return 0;
}
int BlockDataTable::read_file(File32& file)
{
	//最多保存100个
	IndexSize_t arr[100];
	unsigned int flag = 0,blocks = 0,block_offset = 0,size = 0;
	FILE32_READ_RETURN(file,(char*)&flag,4,-1);
	FILE32_READ_RETURN(file,(char*)&blocks,4,-1);
	FILE32_READ_RETURN(file,(char*)&block_offset,4,-1);
	FILE32_READ_RETURN(file,(char*)&size,4,-1);
	if(flag!=blocks + block_offset + size + 69 || blocks>2000000 || size>100)
	{
		assert(0);
		return -1;
	}
	if(size>0)
		FILE32_READ_RETURN(file,(char*)arr,(int)(size*sizeof(IndexSize_t)),-1);
	if(blocks>0)
	{
		if(!resize(blocks,block_offset,false))
			return -1;
		for(unsigned int i=0;i<size;++i)
		{
			if(arr[i].index<blocks)
				m_data[arr[i].index/BDT_SUBNUM][arr[i].index%BDT_SUBNUM].size = arr[i].size;
			else
				assert(false);
		}
	}
	return 0;
}

