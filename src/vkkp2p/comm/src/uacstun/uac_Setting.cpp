#include "uac_Setting.h"
#include "uac_IniFile.h"
#include "uac_Util.h"

#define DEBUGMSG printf

namespace UAC
{
Setting::Setting(void)
{
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
	IniFile ini;
	if(-1==ini.open("./uacstun.ini"))
		return;

	char buf[1024];
	m_stunsvr_config.accept_ip = Util::ip_atoh(ini.read_string("stunsvr","accept_ip","",buf,1024));
	m_stunsvr_config.accept_port1 = ini.read_int("stunsvr","accept_port1",8111);
	m_stunsvr_config.accept_port2 = ini.read_int("stunsvr","accept_port2",8112);
	m_stunsvr_config.stunB_ip = Util::ip_atoh(ini.read_string("stunsvr","stunB_ip","127.0.0.1",buf,1024));
	m_stunsvr_config.stunB_port1 = ini.read_int("stunsvr","stunB_port1",8111);
	m_stunsvr_config.stunB_port2 = ini.read_int("stunsvr","stunB_port2",8112);
	m_stunsvr_config.stunC_ip = Util::ip_atoh(ini.read_string("stunsvr","stunC_ip","127.0.0.1",buf,1024));
	m_stunsvr_config.stunC_port1 = ini.read_int("stunsvr","stunC_port1",8111);

	//save:
	ini.write_string("stunsvr","accept_ip",Util::ip_htoa(m_stunsvr_config.accept_ip));
	ini.write_int("stunsvr","accept_port1",m_stunsvr_config.accept_port1);
	ini.write_int("stunsvr","accept_port2",m_stunsvr_config.accept_port2);
	ini.write_string("stunsvr","stunB_ip",Util::ip_htoa(m_stunsvr_config.stunB_ip));
	ini.write_int("stunsvr","stunB_port1",m_stunsvr_config.stunB_port1);
	ini.write_int("stunsvr","stunB_port2",m_stunsvr_config.stunB_port2);
	ini.write_string("stunsvr","stunC_ip",Util::ip_htoa(m_stunsvr_config.stunC_ip));
	ini.write_int("stunsvr","stunC_port1",m_stunsvr_config.stunC_port1);
}

}

