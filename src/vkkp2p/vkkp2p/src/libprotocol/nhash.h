#pragma once
#include "ntypes.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifdef _WIN32
	#pragma warning(disable:4996)
#endif

//满足map<hash_t,int>类型
#define __MIN2(i,j)  (i)<(j)?(i):(j)

enum HASHTYPE{ HT_SHA1=1,HT_ID32,HT_URL,HT_URL2,HT_URLDL};  //HT_URLDL:用于downlist的url hash

//:hash 字节说明: 4字节little_endial 32位hash值 + 1字节HASH类型 + 1字节HASH长度（不包括前面6字节）+HASH
class nhash
{
public:
	nhash(void) : m_buf(new unsigned char[HASHLEN]) {memset(m_buf,0,6);}
	nhash(const nhash& h) : m_buf(new unsigned char[HASHLEN]) {memcpy(m_buf,h.m_buf,HASHLEN);}
	~nhash(void) { delete[] m_buf;}
	
	void reset() {memset(m_buf,0,6);}
	const unsigned char* buffer() const { return m_buf;}
	unsigned char hash_type() const {return m_buf[4];}
	static unsigned char hash_type(char* hashbuf) {return hashbuf[4];}
	bool empty()const {return 0==m_buf[4];}
	void clear() {m_buf[4]=0;}
	//直接设置内存
	void set_buffer(const unsigned char* hash_buf);

	//HT_ID32类.直播类种子：只是一个32位ID
	void set_hash_id32(unsigned int id);
	int to_hash_id32(unsigned int& id) const;

	//HT_SHA1类。只设置20位的sha1值,注意4位hash数值放在第0~3字节
	void set_sha1_buffer(const unsigned char *sha1_buf);
	void set_sha1_string(const char *sha1_str);
	int to_sha1_string(char *buf,int bufLen) const;

	//HT_URL类,URL类将URL转成10字节二进制数据。
	void set_url_sha1buf(const unsigned char* sha1buf); //
	void set_url_string(const char* url);
	void set_url_string_hash(const char* strhash);
	int to_url_string(char *buf,int bufLen) const;

	//HT_URLDL类,URLDL类将URL转成10字节二进制数据。跟url类只是型号不同
	void set_urldl_sha1buf(const unsigned char* sha1buf); //
	void set_urldl_string(const char* url);
	void set_urldl_string_hash(const char* strhash);
	int to_urldl_string(char *buf,int bufLen) const;

	//HT_URL2类,URL2类将URL转成10字节二进制数据。
	void set_url2_string(const char* url,const char* sub_url);
	void set_url2_string_hash(const char* strhash);
	int to_url2_string(char *buf,int bufLen) const;
	bool url2hash_to_urldlhash(nhash& hash) const;

	//转换成字符串, [type].[string]
	int set_string_hash(const char* strhash); //strhash由to_string转换出来
	int to_string(char *buf,int bufLen) const;

	//***********************************
	nhash& operator=(const nhash& h)
	{
		if(this==&h)
			return *this;
		memcpy(m_buf,h.m_buf,HASHLEN);
		return *this;
	}
	int compare(const unsigned char* hash_buf) const
	{
		return memcmp(m_buf+4,hash_buf+4,__MIN2(m_buf[5]+2,HASHLEN-4)); //HASHLEN(48)-4
	}
	int compare(const nhash& h) const 
	{ 
		if(this==&h) return 0; 
		return compare(h.m_buf);
	}
	bool operator<(const nhash& h) const { return compare(h)<0;}
	bool operator>(const nhash& h) const { return compare(h)>0;}
	bool operator<=(const nhash& h) const { return compare(h)<=0;}
	bool operator>=(const nhash& h) const { return compare(h)>=0;}
	bool operator==(const nhash& h) const { return compare(h)==0;}
	bool operator!=(const nhash& h) const { return compare(h)!=0;}

	static void write_little_endian_32(unsigned char* buf,unsigned int val)
	{
		buf[0] = (unsigned char)val;
		buf[1] = (unsigned char)(val>>8);
		buf[2] = (unsigned char)(val>>16);
		buf[3] = (unsigned char)(val>>24);
	}
	static void read_little_endian_32(unsigned char* buf,unsigned int& val)
	{
		val = buf[0];
		val |= ((unsigned int)buf[1])<<8;
		val |= ((unsigned int)buf[2])<<16;
		val |= ((unsigned int)buf[3])<<24;
	}
private:
	unsigned char *m_buf;
};

typedef nhash hash_t;

