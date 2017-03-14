#pragma once

#include <string.h>


class String
{
public:
	String(const char* sz=0);
	String(const String& str);
	~String(void);
	static size_t npos;
public:
	String& operator=(const char* sz);
	String& operator=(const String& str);
	String operator+(const char* sz);
	String operator+(const String& str);
	String& operator+=(const char* sz);
	String& operator+=(const String& str);
	bool replace(size_t pos,size_t len,const char* buf,size_t buflen=(size_t)-1); //len 为要替换原来串的长度
	bool operator==(const char* sz) const;
	bool operator==(const String& str) const;
	bool operator!=(const char* sz) const;
	bool operator!=(const String& str) const; 
	bool operator<(const char* sz) const;
	bool operator<(const String& str) const;
	size_t find(const char* sz,size_t pos=0) const;
	size_t find(const String& str,size_t pos=0) const;
	size_t rfind(const char s) const;
	String substr(size_t pos,size_t len=(size_t)-1) const;
	String& erase(size_t pos,size_t len=(size_t)-1);
	size_t length() const;
	bool empty() const;
	const char* c_str() const {if(!m_data) return &m_tmp; return m_data;}
	const char& at(size_t i) const;
	const char& operator[](size_t i) const;
private:
	bool mem_ok(size_t size,bool useold=false);
private:
	char *m_data;
	size_t m_size;
	char m_tmp;
};

typedef String string;

