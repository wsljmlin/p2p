#pragma once
#include <string>
#include <list>
using namespace std;

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
	// 4482 4267 4018 4800 4311 4312 4102
	#pragma warning(disable:4996)
#endif

class Util
{
public:
	Util(void){}
	~Util(void){}

	//*********************************
	static string get_string_index(const string& source,int index,const string& sp);
	static int get_string_index_pos(const string& source,int index,const string& sp);
	static int get_string_index_count(const string& source,const string& sp);
	static string set_string_index(string &source,int index,const string& val, const string& sp);
	static string& string_trim(string& str,char c=' ');
	static string& string_trim_endline(string& str);
	static string& str_replace(string& str,const string& str_old,const string& str_new);

	//*********************************
	static bool get_stringlist_from_file(const string& path,list<string>& ls);
	static bool put_stringlist_to_file(const string& path,list<string>& ls);

	static int write_log(const char *strline,const char *path);
};
