#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "uac_SimpleString.h"
#include "uac_cyclist.h"
#include "uac_basetypes.h"
#include "uac_File32.h"

#ifdef _WIN32
#pragma warning(disable:4996)
#endif

namespace UAC
{
class IniFile
{
public:
	IniFile(void);
	~IniFile(void);
	typedef struct ini_key{
		string name;
		string value;
		string seq;
		//ini_key(void){}
		//ini_key(const ini_key& akey): name(akey.name),value(akey.value),seq(akey.seq) {}
		//~ini_key(void){}
	}ini_key_t;

	typedef struct ini_section{
		string name;
		list<ini_key_t> key_list;
		~ini_section(void){}
	}ini_section_t;
	
	static string& string_trim(string& str,char c=' ');
	static string& string_trim_endline(string& str);
public:
	int open(const char* path);
	int close();

	int write_string(const char* section_name,const char* key_name,const char* inval);
	int write_int(const char* section_name,const char* key_name,int inval);

	char* read_string(const char* section_name,const char* key_name,const char* default_val,char *outbuf,unsigned int outbuflen);
	int read_int(const char* section_name,const char* key_name,int default_val);
private:
	int add_section(const char* section_name,const ini_key_t& key);
	void save();
private:
	bool m_bopen;
	bool m_bchanged;
	list<ini_section_t*> m_data_list;
	string m_path;
};

#ifndef _WIN32
int GetPrivateProfileIntA(const char* szAppName,const char* szKeyName,int nDefault,const char* szFileName);
int WritePrivateProfileIntA(const char* szAppName,const char* szKeyName,int nValue,const char* szFileName);
char* GetPrivateProfileStringA(const char* szAppName,const char* szKeyName,const char* szDefault,char* szOut,unsigned int nOutLen,const char* szFileName);
int WritePrivateProfileStringA(const char* szAppName,const char* szKeyName,const char* szValue,const char* szFileName);
#endif

}
