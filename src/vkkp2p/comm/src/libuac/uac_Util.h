#pragma once
#include "uac_SimpleString.h"


namespace UAC
{
class Util
{
private:
	Util(void){}
	~Util(void){}

public:
	static string ip_explain(const char* s);
	static string ip_explain_ex(const char* s,int maxTick=5000);


	static bool is_ip(const char* ip);
	static bool is_dev(const char* ip);

	static char* ip_htoa(unsigned int ip);
	static char* ip_ntoa(unsigned int nip);
	static unsigned int ip_atoh(const char* ip);


	static int get_mtu();
	
	static bool is_write_debug_log();
	static int write_debug_log(const char *strline,const char *path);
	static int write_log(const char *strline,const char *path);
};
}
