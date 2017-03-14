#pragma once
#include "commons.h"

#define TRACKER_VERSION 30001
class Setting
{
public:
	Setting(void);
	~Setting(void);
	int init();
	void save_setting();
	void load_setting();
	int get_new_uid(puid_t& uid,uchar mac[]);
protected:
	GETSET(int,m_tracker_id,_tracker_id)
	GETSET(string,m_tracker_ip,_tracker_ip)
	GETSET(unsigned short,m_tracker_port,_tracker_port)
	GETSET(unsigned short,m_http_port,_http_port)
	GETSET(int,m_rsp_server_num,_rsp_server_num)
	GETSET(int,m_rsp_center_num,_rsp_center_num)
	GETSET(int,m_rsp_super_num,_rsp_super_num)
	//m_vip_super_minnum表示最少开始多少个super来加速playlist支持vip
	GETSET(int,m_vip_super_minnum,_vip_super_minnum)  

	GETSET(string,m_db_ip,_db_ip)
	GETSET(int,m_db_port,_db_port)
	GETSET(string,m_db_name,_db_name)
	GETSET(string,m_db_user,_db_user)
	GETSET(string,m_db_password,_db_password)
	GETSET(string,m_db_unix_socket,_db_unix_socket)
	GETSET(string,m_tracker_dblog_ip,_tracker_dblog_ip)

};
typedef Singleton<Setting> SettingSngl;

