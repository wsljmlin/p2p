#pragma once
#include <File32.h>

class BlockTable
{
public:
	BlockTable(void);
	~BlockTable(void);
	
	bool resize(unsigned int blocks,unsigned int block_offset,bool copyold=true);
	bool get_block(unsigned int i) const;
	bool set_block(unsigned int i,bool b=true);
	bool set_all_block(bool b);
	int get_block_buf(char* buf,unsigned int len,unsigned int buf_offset) const;
	bool set_block_buf(const char* buf,unsigned int len,unsigned int buf_offset,unsigned int& real_off,unsigned int& real_len);
	bool operator[](unsigned int i) const;
	BlockTable& operator=(const BlockTable& bt);
	unsigned int get_block_num() const {return m_blocks-m_block_offset_remainder;}
	unsigned int get_block_offset() const {return m_block_offset+m_block_offset_remainder;}
	bool is_range(unsigned int index) const { return index>=get_block_offset() && index<(m_block_offset+m_blocks);}
	bool is_all_set() const;

	int write_file(File32& file) const;
	int read_file(File32& file);
private:
	unsigned char *m_buf;
	unsigned int m_buflen;   //�����С
	unsigned int m_blocks; //blocks
	//��ʾ��table�ǵ���ʼλ������ļ�����ʼ���.��table��һ����ʾ�����ļ����.
	unsigned int m_block_offset; 
	//m_block_offset/8������,��Ҫ�ǲ���m_block_offset 8λ����. ��֤m_block_offset��8�����������㿽��.
	unsigned int m_block_offset_remainder; 
};
