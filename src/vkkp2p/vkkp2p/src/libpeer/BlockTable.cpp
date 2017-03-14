#include "BlockTable.h"
#include <string.h>
#include <assert.h>

BlockTable::BlockTable(void)
:m_buf(NULL)
,m_buflen(0)
,m_blocks(0)
,m_block_offset(0)
,m_block_offset_remainder(0)
{
}

BlockTable::~BlockTable(void)
{
	resize(0,0);
}
bool BlockTable::resize(unsigned int blocks,unsigned int block_offset,bool copyold/*=true*/)
{
	if((m_blocks-m_block_offset_remainder)==blocks && (m_block_offset+m_block_offset_remainder) == block_offset)
		return true;
	unsigned int size = 0;
	unsigned char *ptr = NULL;
	unsigned int remainder = 0;
	//先拷贝可能重叠的数据
	if(blocks>0)
	{
		//如果是从8的整数倍起算
		remainder = block_offset%8;
		if(remainder>0)
		{
			blocks += remainder;
			block_offset -= remainder;
		}
		size = (blocks+7)/8;
		ptr = new unsigned char[size];
		if(!ptr) return false;
		memset(ptr,0,size);
		if(m_buf && copyold)
		{
			if(block_offset<m_block_offset)
			{
				//前移了
				if(block_offset+blocks>m_block_offset)
				{
					unsigned int off = m_block_offset-block_offset;
					unsigned int len = blocks - off;
					if(len>m_blocks)
						len = m_blocks;
					off /= 8;
					len = (len+7)/8;
					memcpy(ptr+off,m_buf,len);
				}
			}else
			{
				//可能后移了
				if(block_offset<m_block_offset+m_blocks)
				{
					unsigned int off = block_offset-m_block_offset;
					unsigned int len = m_blocks - off;
					if(len>blocks)
						len = blocks;
					off /= 8;
					len = (len+7)/8;
					memcpy(ptr,m_buf+off,len);
				}
			}
		}
	}
	if(m_buf)
	{
		delete[] m_buf;
		m_buf = NULL;
		m_buflen = 0;
		m_blocks = 0;
		m_block_offset = 0;
	}
	m_buf = ptr;
	m_buflen = size;
	m_blocks = blocks;
	m_block_offset = block_offset;
	m_block_offset_remainder = remainder;
	return true;
}


bool BlockTable::get_block(unsigned int i) const
{
	if(i<m_block_offset)
	{
		assert(false);
		return false;
	}
	i -= m_block_offset;
	if(i>=m_blocks)
	{
		assert(false);
		return false;
	}
	
	int index = i/8;
	int offset = i%8;
	unsigned char mask = (unsigned char)0x01<<offset;
	return (m_buf[index]&mask)!=0;
}
bool BlockTable::set_block(unsigned int i,bool b/*=true*/)
{
	if(i<m_block_offset)
	{
		assert(false);
		return false;
	}
	i -= m_block_offset;
	if(i>=m_blocks)
	{
		assert(false);
		return false;
	}
	
	int index = i/8;
	int offset = i%8;
	unsigned char mask = (unsigned char)0x01<<offset;
	if(b)
		m_buf[index] |= mask;
	else
		m_buf[index] &= ~mask;
	return true;
}
bool BlockTable::set_all_block(bool b)
{
	if(!m_buf)
		return false;
	if(b)
		memset(m_buf,-1,m_buflen);
	else
		memset(m_buf,0,m_buflen);
	return true;
}

int BlockTable::get_block_buf(char* buf,unsigned int len,unsigned int buf_offset) const
{
	unsigned int n = m_block_offset/8;
	memset(buf,0,len);
	if(buf_offset<=n)
	{
		unsigned int off = n - buf_offset;
		if(len<=off)
			return 0;
		len -= off;
		buf += off;
		if(len>m_buflen)
			len = m_buflen;
		if(len==0)
			return 0;
		memcpy(buf,m_buf,len);
		return len;
	}
	else
	{
		buf_offset -= n;
		if(buf_offset>=m_buflen)
			return 0;
		n = m_buflen-buf_offset;
		if(n>len)
			n = len;
		if(n==0)
			return 0;
		memcpy(buf,m_buf+buf_offset,n);
		return n;
	}
}
bool BlockTable::set_block_buf(const char* buf,unsigned int len,unsigned int buf_offset,unsigned int& real_off,unsigned int& real_len)
{
	unsigned int n = m_block_offset/8;
	real_off = real_len = 0;
	if(buf_offset<=n)
	{
		unsigned int off = n - buf_offset;
		if(len<=off)
			return false;
		len -= off;
		buf += off;
		if(len>m_buflen) len=m_buflen;
		if(len==0)
			return false;
		memcpy(m_buf,buf,len);
		real_off = off;
		real_len = len;
		return true;
	}
	else
	{
		buf_offset -= n;
		if(buf_offset>=m_buflen)
			return false;
		n = m_buflen-buf_offset;
		if(n>len)
			n = len;
		if(n==0)
			return false;
		memcpy(m_buf+buf_offset,buf,n);
		real_off = 0;
		real_len = n;
		return true;
	}
}
bool BlockTable::operator[](unsigned int i) const
{
	return get_block(i);
}
BlockTable& BlockTable::operator=(const BlockTable& bt)
{
	if(this == &bt)
		return *this;
	if(resize(bt.m_blocks-bt.m_block_offset_remainder,bt.m_block_offset+bt.m_block_offset_remainder,false))
	{
		if(m_buf)
			memcpy(m_buf,bt.m_buf,bt.m_buflen);
	}
	else
		assert(false);
	return *this;
}
bool BlockTable::is_all_set() const
{
	unsigned int n = m_blocks/8;
	unsigned int m = m_blocks%8;
	unsigned int i=0;
	for(i=0;i<n;++i)
	{
		if(m_buf[i]!=(unsigned char)0xff)
			return false;
	}
	unsigned char mask = 0;
	for(i=0;i<m;++i)
	{
		mask = (unsigned char)0x01<<i;
		if((m_buf[n]&mask)==0)
			return false;
	}
	return true;
}

int BlockTable::write_file(File32& file) const
{
	unsigned int flag = m_buflen + m_blocks + m_block_offset + m_block_offset_remainder + 69;
	FILE32_WRITE_RETURN(file,(char*)&flag,4,-1);
	FILE32_WRITE_RETURN(file,(char*)&m_buflen,4,-1);
	FILE32_WRITE_RETURN(file,(char*)&m_blocks,4,-1);
	FILE32_WRITE_RETURN(file,(char*)&m_block_offset,4,-1);
	FILE32_WRITE_RETURN(file,(char*)&m_block_offset_remainder,4,-1);
	if(m_buflen>0)
	{
		FILE32_WRITE_RETURN(file,(char*)m_buf,m_buflen,-1);
	}
	return 0;
}
int BlockTable::read_file(File32& file)
{
	unsigned int flag = 0,buflen=0,blocks=0,block_offset=0,remainder=0;
	FILE32_READ_RETURN(file,(char*)&flag,4,-1);
	FILE32_READ_RETURN(file,(char*)&buflen,4,-1);
	FILE32_READ_RETURN(file,(char*)&blocks,4,-1);
	FILE32_READ_RETURN(file,(char*)&block_offset,4,-1);
	FILE32_READ_RETURN(file,(char*)&remainder,4,-1);
	if(flag!=buflen + blocks + block_offset + remainder + 69 || buflen>2000000 || remainder>7)
	{
		assert(0);
		return -1;
	}
	if(blocks<remainder || (blocks+7)/8!=buflen)
	{
		assert(0);
		return -1;
	}
	if(buflen<=0)
		return 0;
	if(!(m_buf=new unsigned char[buflen]))
		return -1;
	m_buflen = buflen;
	m_blocks = blocks;
	m_block_offset = block_offset;
	m_block_offset_remainder = remainder;
	FILE32_READ_RETURN(file,(char*)m_buf,buflen,-1);
	return 0;
}
