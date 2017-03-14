#pragma once
#include "commons.h"

class Setting
{
public:
	Setting(void);
	~Setting(void);
public:
	int init();
	int fini();
private:
	void load_setting();
protected:
	GETSET(string,m_accept_ip,_accept_ip) //¿ÉÄÜÎª¿Õ
	GETSET(unsigned short,m_accept_port,_accept_port)
	GETSET(unsigned short,m_accept_http_port,_accept_http_port)
};
typedef Singleton<Setting> SettingSngl;
