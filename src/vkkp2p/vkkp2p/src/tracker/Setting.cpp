#include "Setting.h"
#include "IniFile.h"
#include "Util.h"

Setting::Setting(void)
{
}

Setting::~Setting(void)
{
}


int Setting::init()
{
	Util::my_create_directory("./log/");
	load_setting();
	save_setting();
	return 0;
}
void Setting::load_setting()
{
	IniFile ini;
	if(-1==ini.open("./tracker_config.ini"))
	{
		m_tracker_id = 1;
		m_tracker_ip = "";
		m_tracker_port = 8000;
		m_rsp_server_num = 0;
		m_rsp_center_num = 0;
		m_rsp_super_num = 0;
		m_vip_super_minnum = 0;
		return;
	}

	char buf[1024];
	m_tracker_id   = ini.read_int("public","tracker_id",0);
	m_tracker_ip    = ini.read_string("public","tracker_ip","",buf,1024);
	m_tracker_port  = ini.read_int("public","tracker_port",8000);
	m_http_port  = ini.read_int("public","http_port",10080);

	//m_db_ip        = ini.read_string("db","db_ip","",buf,1024);
	//m_db_port      = ini.read_int("db","db_port",0);
	//m_db_name      = ini.read_string("db","db_name","",buf,1024);
	//m_db_user      = ini.read_string("db","db_user","",buf,1024);
	//m_db_password  = ini.read_string("db","db_password","",buf,1024);
	//m_db_unix_socket = ini.read_string("db","db_unix_socket","",buf,1024);
	//m_tracker_dblog_ip    = ini.read_string("public","tracker_dblog_ip","",buf,1024);

	m_rsp_server_num = ini.read_int("search","rsp_server_num",5);
	m_rsp_center_num = ini.read_int("search","rsp_center_num",2);
	m_rsp_super_num = ini.read_int("search","rsp_super_num",2);
	m_vip_super_minnum = ini.read_int("search","vip_super_minnum",0);
}
void Setting::save_setting()
{
	IniFile ini;
	if(-1==ini.open("./tracker_config.ini"))
		return;
	ini.write_int("public","tracker_id",m_tracker_id);
	ini.write_string("public","tracker_ip",m_tracker_ip.c_str());
	ini.write_int("public","tracker_port",m_tracker_port);
	ini.write_int("public","http_port",m_http_port);

	//ini.write_string("db","db_ip",m_db_ip.c_str());
	//ini.write_int("db","db_port",m_db_port);
	//ini.write_string("db","db_name",m_db_name.c_str());
	//ini.write_string("db","db_user",m_db_user.c_str());
	//ini.write_string("db","db_password",m_db_password.c_str());
	//ini.write_string("db","db_unix_socket",m_db_unix_socket.c_str());
	//ini.write_string("db","tracker_dblog_ip",m_tracker_dblog_ip.c_str());

	ini.write_int("search","rsp_server_num",m_rsp_server_num);
	ini.write_int("search","rsp_center_num",m_rsp_center_num);
	ini.write_int("search","rsp_super_num",m_rsp_super_num);
	ini.write_int("search","vip_super_minnum",m_vip_super_minnum);
}

int Setting::get_new_uid(puid_t& uid,uchar mac[])
{
	memset(uid,0,sizeof(puid_t));
	srand((unsigned int)time(NULL));
	char buf[16];
	for(int i=0;i<6;++i)
		sprintf(buf+2*i,"%02x",mac[i]);
	buf[12] = '\0';
	sprintf(uid,"vttv-%d-%s-%d",m_tracker_id,buf,rand()%9000+1000);
	return 0;
}

