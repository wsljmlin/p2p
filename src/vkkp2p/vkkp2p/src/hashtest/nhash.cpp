#include "nhash.h"
#include "sha1.h"
#include <stdlib.h>

//mysql 中的 hash函数
static unsigned int calc_hashnr(const unsigned char *key,unsigned int length) 
{ 
	register unsigned int nr=1, nr2=4; 
	while(length--) 
	{ 
		nr^= (((nr & 63)+nr2)*((unsigned int)(unsigned char)*key++))+ (nr << 8); 
		nr2+=3; 
	} 
	return (unsigned int)nr; 
} 

void nhash::set_buffer(const unsigned char* hash_buf)
{
	char len = hash_buf[5]+6;
	memcpy(m_buf,hash_buf,__MIN2(HASHLEN,len));
}

//直播类种子：只是一个32位ID
void nhash::set_hash_id32(unsigned int id)
{
	write_little_endian_32(m_buf,id);
	m_buf[4]=HT_ID32;
	m_buf[5]=4;
	write_little_endian_32(m_buf+6,id);
}
int nhash::to_hash_id32(unsigned int& id) const
{
	if(HT_ID32!=m_buf[4])
	{
		assert(false);
		return -1;
	}
	read_little_endian_32(m_buf+6,id);
	return 0;
}

void nhash::set_sha1_buffer(const unsigned char *sha1_buf)
{
	unsigned int ihash = calc_hashnr(sha1_buf,20);
	write_little_endian_32(m_buf,ihash);
	m_buf[4]=HT_SHA1;
	m_buf[5]=20;
	memcpy(m_buf+6,sha1_buf,20);
}
void nhash::set_sha1_string(const char *sha1_str)
{
	assert(strlen(sha1_str)==40);
	unsigned char buf[24];
	unsigned int tmp = 0;
	for(int i=0;i<20;++i)
	{
		sscanf(sha1_str+2*i,"%2x",&tmp);
		buf[i] = (unsigned char)tmp;
	}
	set_sha1_buffer(buf);
}
int nhash::to_sha1_string(char *buf,int bufLen) const 
{
	if(bufLen<41 || HT_SHA1!= m_buf[4])
	{
		assert(false);
		return -1;
	}
	for(int i=0;i<20;++i)
		sprintf(buf+2*i,"%02x",m_buf[i+6]);
	buf[40] = '\0';
	return 0;
}

//url
void nhash::set_url_sha1buf(const unsigned char* sha1buf)
{
	unsigned int ihash = calc_hashnr(sha1buf,10);
	write_little_endian_32(m_buf,ihash);
	m_buf[4]=HT_URL;
	m_buf[5]=10;
	memcpy(m_buf+6,sha1buf,10);
}
void nhash::set_url_string(const char* url)
{
	char sha1buf[44];
	memset(sha1buf,0,44);
	assert(url && strlen(url));
	Sha1_BuildBuffer(url,(int)strlen(url),0,sha1buf);
	//转成10字节
	for(int i=0;i<10;i++)
		sha1buf[i] *= sha1buf[10+i];
	set_url_sha1buf((unsigned char*)sha1buf);
}
void nhash::set_url_string_hash(const char* strhash)
{
	//assert(strlen(strhash)==20); //有可能url2直接调url
	unsigned char sha1buf[44];
	unsigned int tmp = 0;
	memset(sha1buf,0,44);
	for(int i=0;i<10;++i)
	{
		sscanf(strhash+2*i,"%2x",&tmp);
		sha1buf[i] = (unsigned char)tmp;
	}
	set_url_sha1buf(sha1buf);
}
int nhash::to_url_string(char *buf,int bufLen) const
{
	if(bufLen<21 || HT_URL!= m_buf[4])
	{
		assert(false);
		return -1;
	}
	for(int i=0;i<10;++i)
		sprintf(buf+2*i,"%02x",m_buf[i+6]);
	buf[20] = '\0';
	return 0;
}

//urldl
void nhash::set_urldl_sha1buf(const unsigned char* sha1buf)
{
	unsigned int ihash = calc_hashnr(sha1buf,10);
	write_little_endian_32(m_buf,ihash);
	m_buf[4]=HT_URLDL;
	m_buf[5]=10;
	memcpy(m_buf+6,sha1buf,10);
}
void nhash::set_urldl_string(const char* url)
{
	char sha1buf[44];
	memset(sha1buf,0,44);
	assert(url && strlen(url));
	Sha1_BuildBuffer(url,(int)strlen(url),0,sha1buf);
	//转成10字节
	for(int i=0;i<10;i++)
		sha1buf[i] *= sha1buf[10+i];
	set_urldl_sha1buf((unsigned char*)sha1buf);
}
void nhash::set_urldl_string_hash(const char* strhash)
{
	//assert(strlen(strhash)==20); //有可能url2直接调url
	unsigned char sha1buf[44];
	unsigned int tmp = 0;
	memset(sha1buf,0,44);
	for(int i=0;i<10;++i)
	{
		sscanf(strhash+2*i,"%2x",&tmp);
		sha1buf[i] = (unsigned char)tmp;
	}
	set_urldl_sha1buf(sha1buf);
}
int nhash::to_urldl_string(char *buf,int bufLen) const
{
	if(bufLen<21 || HT_URLDL!= m_buf[4])
	{
		assert(false);
		return -1;
	}
	for(int i=0;i<10;++i)
		sprintf(buf+2*i,"%02x",m_buf[i+6]);
	buf[20] = '\0';
	return 0;
}

//url2
void nhash::set_url2_string(const char* url,const char* sub_url)
{
	char sha1buf[44];
	memset(sha1buf,0,44);
	assert(url && strlen(url));
	//url转成10字节
	Sha1_BuildBuffer(url,(int)strlen(url),0,sha1buf);
	for(int i=0;i<10;i++)
		sha1buf[i] *= sha1buf[10+i];
	//sub_url转成10字节
	Sha1_BuildBuffer(sub_url,(int)strlen(sub_url),0,sha1buf+10);
	for(int i=10;i<20;i++)
		sha1buf[i] *= sha1buf[10+i];
	unsigned int ihash = calc_hashnr((unsigned char*)sha1buf,20);
	write_little_endian_32(m_buf,ihash);
	m_buf[4]=HT_URL2;
	m_buf[5]=20;
	memcpy(m_buf+6,sha1buf,20);
}
void nhash::set_url2_string_hash(const char* strhash)
{
	unsigned char sha1buf[44];
	unsigned int tmp = 0;
	memset(sha1buf,0,44);
	for(int i=0;i<20;++i)
	{
		sscanf(strhash+2*i,"%2x",&tmp);
		sha1buf[i] = (unsigned char)tmp;
	}
	unsigned int ihash = calc_hashnr((unsigned char*)sha1buf,20);
	write_little_endian_32(m_buf,ihash);
	m_buf[4]=HT_URL2;
	m_buf[5]=20;
	memcpy(m_buf+6,sha1buf,20);
}
int nhash::to_url2_string(char *buf,int bufLen) const
{
	if(bufLen<41 || HT_URL2!= m_buf[4])
	{
		assert(false);
		return -1;
	}
	for(int i=0;i<20;++i)
		sprintf(buf+2*i,"%02x",m_buf[i+6]);
	buf[41] = '\0';
	return 0;
}
bool nhash::url2hash_to_urldlhash(nhash& hash) const
{
	if(HT_URL2!= m_buf[4])
		return false;
	hash.set_urldl_sha1buf(m_buf+6);
	return true;
}
int nhash::set_string_hash(const char* strhash)
{
	int ht =atoi(strhash);
	if(HT_SHA1 == ht)
	{
		if(40!=strlen(strhash+2))
			return -1;
		set_sha1_string(strhash+2);
	}
	else if(HT_ID32 == ht)
	{
		unsigned int id = atoi(strhash+2);
		set_hash_id32(id);
		
	}
	else if(HT_URL==ht)
	{
		if(20!=strlen(strhash+2))
			return -1;
		set_url_string_hash(strhash+2);
	}
	else if(HT_URLDL==ht)
	{
		if(20!=strlen(strhash+2))
			return -1;
		set_urldl_string_hash(strhash+2);
	}
	else if(HT_URL2==ht)
	{
		if(40!=strlen(strhash+2))
			return -1;
		set_url2_string_hash(strhash+2);
	}
	else
	{
		//assert(false);
		return -1;
	}
	return 0;
}
int nhash::to_string(char *buf,int bufLen) const 
{
	sprintf(buf,"%d_",m_buf[4]);
	if(HT_SHA1 == m_buf[4])
	{
		return to_sha1_string(buf+2,bufLen-2);
	}
	else if(HT_ID32 == m_buf[4])
	{
		unsigned int id = 0;
		to_hash_id32(id);
		sprintf(buf+2,"%d",id);
		return 0;
	}
	else if(HT_URL==m_buf[4])
	{
		return to_url_string(buf+2,bufLen-2);
	}
	else if(HT_URLDL==m_buf[4])
	{
		return to_urldl_string(buf+2,bufLen-2);
	}
	else if(HT_URL2==m_buf[4])
	{
		return to_url2_string(buf+2,bufLen-2);
	}
	else
		assert(false);
	return -1;
}


