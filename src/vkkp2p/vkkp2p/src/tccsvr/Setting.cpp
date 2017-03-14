#include "Setting.h"
#include "IniFile.h"

Setting::Setting(void)
{
	m_accept_ip="";
	m_accept_port=7150;
}

Setting::~Setting(void)
{
}
int Setting::init()
{	
	DEBUGMSG("#-Setting::init \n");
	load_setting();
	return 0;
}
int Setting::fini()
{
	DEBUGMSG("#-Setting::fini() \n");
	return 0;
}
void Setting::load_setting()
{
	//使用uacstun的配置，accept_port1用作TCP连接性测试端口，accept_port2用作HTTP接口端口
	IniFile ini;
	if(-1==ini.open("./uacstun.ini"))
		return;

	char buf[1024];
	m_accept_ip=ini.read_string("stunsvr","accept_ip","",buf,1024);
	m_accept_port=ini.read_int("stunsvr","accept_port1",8111);
	m_accept_http_port=ini.read_int("stunsvr","accept_port2",8112); //
}
