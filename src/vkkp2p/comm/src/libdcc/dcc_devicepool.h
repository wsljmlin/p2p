#pragma once
#include "dcc_serialdevice.h"
#include "Singleton.h"
#include "SynchroObj.h"

class dcc_devicepool
{
public:
	dcc_devicepool(void);
	~dcc_devicepool(void);

	typedef Simple_Mutex Mutex;
	typedef TLock<Simple_Mutex> Lock;
public:
	int init(const char* conf_xmlpath);
	int fini();
	unsigned short get_httpport()const{return m_conf.m_httpport;}

	//执行1个指令，返回0成功， outmsg返回执行相关的信息描述
	//同一时刻只能执行一个指令
	int call(int call_id,string& outmsg);

private:
	dcc_serialdevice* find_serialdevice(const string& device);

private:
	bool m_binit;
	Mutex m_mt;
	dcc_config m_conf;
	map<string,dcc_serialdevice*> m_sdevmap;
	bool m_bcalling;

};
typedef Singleton<dcc_devicepool> dcc_devicepoolsngl;

