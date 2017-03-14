#include "SimpleString.h"

#include <assert.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#endif

size_t String::npos=(size_t)-1;
String::String(const char* sz/*=NULL*/)
{
	m_data = NULL;
	m_size = 0;
	m_tmp = '\0';
	*this = sz;
}
String::String(const String& str)
{
	m_data = NULL;
	m_size = 0;
	*this = str;
}

String::~String(void)
{
	if(m_data)
		delete[] m_data;
	m_size = 0;
}
bool String::mem_ok(size_t size,bool useold/*=false*/)
{
	//清空
	if((size_t)-1==size)
	{
		if(m_data)
		{
			delete[] m_data;
			m_data = NULL;
		}
		m_size = 0;
		return true;
	}
	if(size<=m_size)
		return true;
	char *sz = new char[size];
	if(!sz)
	{
		assert(0);
		return false;
	}
	if(m_data && useold)
		strcpy(sz,m_data);
	else
		memset(sz,0,size);
	if(m_data)
		delete[] m_data;
	m_data = sz;
	m_size = size;
	return true;
}
String& String::operator=(const char* sz)
{
	if(sz)
	{
		if(mem_ok((int)strlen(sz)+2))
			strcpy(m_data,sz);
	}
	else
	{
		mem_ok((size_t)-1);
	}
	return *this;
}
String& String::operator=(const String& str)
{
	return operator=(str.c_str());
}
String String::operator+(const char* sz)
{
	String str;
	int size = 0;
	if(sz)
		size += (int)strlen(sz);
	if(m_data)
		size += (int)strlen(m_data);
	if(0==size)
		return str;
	size += 2;
	if(str.mem_ok(size))
	{
		if(m_data)
			strcpy(str.m_data,m_data);
		if(sz)
			strcat(str.m_data,sz);
	}
	return str;
}
String String::operator+(const String& str)
{
	return operator+(str.c_str());
}
String& String::operator+=(const char* sz)
{
	if(sz)
	{
		size_t size = strlen(sz) + 2;
		if(m_data)
			size += strlen(m_data);
		if(mem_ok(size,true))
			strcat(m_data,sz);
	}

	return *this;
}
String& String::operator+=(const String& str)
{
	return operator+=(str.c_str());
}
bool String::operator==(const char* sz) const
{
	if(m_data == sz)
		return true;
	if(empty() && (!sz || 0==strlen(sz)))
		return true;
	if(m_data&&sz)
	{
		if(0==strcmp(m_data,sz))
			return true;
	}
	return false;
}
bool String::replace(size_t pos,size_t len,const char* buf,size_t buflen/*=-1*/) //len 为要替换原来串的长度
{
	size_t slen = strlen(m_data);
	if((size_t)-1==buflen)
		buflen = strlen(buf);
	if(pos+len>slen)
		return false;
	if(buflen>len)
	{
		if(!mem_ok(slen+(buflen-len)+2,true))
			return false;
	}
	if(buflen!=len)
	{
		memmove(m_data+pos+buflen,m_data+pos+len,slen-(pos+len));
		m_data[slen+buflen-len] = '\0';
	}
	memcpy(m_data + pos,buf,buflen);
	return true;
}
bool String::operator==(const String& str) const
{
	return operator==(str.c_str());
}

bool String::operator!=(const char* sz) const
{
	return !(operator==(sz));
}
bool String::operator!=(const String& str) const
{
	return operator!=(str.c_str());
}
bool String::operator<(const char* sz) const
{
	if(m_data == sz)
		return false;
	if(!sz || 0==strlen(sz))
		return false;
	if(empty())
		return true;
	if(strcmp(m_data,sz)<0)
		return true;
	return false;
}
bool String::operator<(const String& str) const
{
	return operator<(str.c_str());
}
size_t String::find(const char* sz,size_t pos/*=0*/) const
{
	if(pos>=m_size)
		return npos;
	if(sz&&m_data)
	{
		char* ptr = strstr(m_data+pos,sz);
		if(ptr)
			return (int)(ptr-m_data);
		else
			return npos;
	}
	else
		return npos;
}
size_t String::find(const String& str,size_t pos/*=0*/) const
{
	return find(str.c_str(),pos);
}
size_t String::rfind(const char s) const
{
	if(m_data)
	{
		char* ptr = strrchr(m_data,s);
		if(ptr)
			return (int)(ptr-m_data);
		else
			return npos;
	}
	else
		return npos;
}

String String::substr(size_t pos,size_t len/*=-1*/) const
{
	if(0==len)
		return NULL;
	String str;
	if(m_data)
	{
		size_t size = strlen(m_data);
		if(pos>=size)
			return str;
		if((size_t)-1==len)
			len = size - pos;
		if(len>(size-pos))
			len = size-pos;
		if(str.mem_ok(len+2))
		{
			memcpy(str.m_data,m_data+pos,len);
		}
	}
	return str;
}
String& String::erase(size_t pos,size_t len/*=(size_t)-1*/)
{
	if(!m_data || 0==len)
		return *this;
	size_t slen = strlen(m_data);
	if(pos>=slen)
		return *this;
	size_t sub_len = slen-pos;
	if((size_t)-1==len || len>sub_len)
		len = sub_len;
	if(sub_len>len)
		memmove(m_data+pos,m_data+pos+len,sub_len-len);
	m_data[slen-len] = '\0';
	return *this;
}
size_t String::length()  const
{
	if(m_data)
		return (int)strlen(m_data);
	return 0;
}
bool String::empty() const
{
	if(!m_data||0==strlen(m_data))
		return true;
	return false;
}
const char& String::at(size_t i) const
{
	if(i<m_size)
		return m_data[i];
	assert(0);
	return m_tmp;
}
const char& String::operator[](size_t i) const
{
	return at(i);
}


