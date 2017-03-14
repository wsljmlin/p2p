#pragma once
#include <File32.h>

#define BDT_SUBNUM   5000

class BlockDataTable
{
public:
	BlockDataTable(void);
	~BlockDataTable(void);
	typedef struct tagBlockData
	{
		unsigned int    size;  //已经保存到硬盘的大小
		void*  data;
		tagBlockData(void): size(0),data(0){}
	}BlockData_t;

	typedef struct tagIndexSize
	{
		unsigned int index;
		unsigned int size;
	}IndexSize_t;

	bool resize(unsigned int blocks,unsigned int block_offset,bool copyold=true);
	BlockData_t& get_block(unsigned int i);
	BlockData_t& operator[](unsigned int i){return get_block(i);}
	BlockDataTable& operator=(const BlockDataTable& bt);
	unsigned int get_block_num() const {return m_blocks;}
	unsigned int get_block_offset() const {return m_block_offset;}
	bool is_range(unsigned int index) const { return index>=m_block_offset && index<(m_block_offset+m_blocks);}

	int write_file(File32& file) const;
	int read_file(File32& file);
public:
	BlockData_t **m_data;
	BlockData_t _tmp;
	unsigned int m_blocks; //blocks
	unsigned int m_block_offset;
};
