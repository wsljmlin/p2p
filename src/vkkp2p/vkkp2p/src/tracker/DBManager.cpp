#include "DBManager.h"
#include "Setting.h"
#include "Util.h"
#include "LogManager.h"

#define _DB_EMPTY SettingSngl::instance()->get_db_ip().empty()
#define _DB_EMPTY_RETURN  if(_DB_EMPTY) return
DBManager::DBManager(void)
: m_bopen(false)
{
	m_curr_sec = time(NULL);
	m_last_ping_sec = m_curr_sec;
	m_bwrite_trackerconf = false;
	LogManagerSngl::instance()->init();
}

DBManager::~DBManager(void)
{
	//DBConnectorSngl::destroy();
	LogManagerSngl::instance()->fini();
	LogManagerSngl::destroy();
}
int DBManager::run()
{
	if(m_bopen)
		return -1;
	if(_DB_EMPTY)
	{
		printf("*** DB ip empty,No Use DB!");
		//return 0;
	}
	m_bopen = true;

	m_packl.resize(2000);
	activate(1);
	printf("#DBManager::init \n");
	return 0;
}
int DBManager::end()
{
	if(!m_bopen)
		return -1;
	m_bopen = false;
	wait();
	Packet inf;
	while(m_packl.get_packet(inf))
	{
		free_data(inf);
	}
	m_packl.resize(0);
	printf("#DBManager::fini \n");
	return 0;
}

int DBManager::work(int e)
{
	//if(!_DB_EMPTY)
	//	DBConnectorSngl::instance()->connect();
	return root_cmd();
}
int DBManager::root_cmd()//处理命令包线程
{
	printf("+DBManager::root_cmd() \n");
	time_t curr_sec = 0;
	int pack_count=0;
	int max_pack_count=0;
	while(m_bopen)
	{
		pack_count = m_packl.get_blocking_count();
		if(pack_count>=max_pack_count)
			max_pack_count = pack_count;

		handle_command();

		curr_sec = time(NULL);
		if(curr_sec!=m_curr_sec)
		{
			m_curr_sec = curr_sec;
			handle_timer();

			if(max_pack_count >= 100)
				printf("...DBManager pack_count=%d...\n",max_pack_count);
			max_pack_count = 0;
		}
		Sleep(8);
	}
	printf("-DBManager::root_cmd() \n");
	return 0;
}

void DBManager::handle_command()
{
	static Packet pki;

	while(m_packl.get_packet(pki))
	{
		switch(pki.cmd)
		{
		case DBC_USER_LOGIN:
			{
				LogManagerSngl::instance()->on_userlogin((DBUserLogin*)pki.data);
				on_userlogin((DBUserLogin*)pki.data);
			}
			break;
		case DBC_USER_LOGIN_OUT:
			{
				LogManagerSngl::instance()->on_userloginout((DBUserLoginOut*)pki.data);
				on_userloginout((DBUserLoginOut*)pki.data);
			}
			break;
		case DBC_ERROR_INFO:
			{
				on_errorinfo((DBErrorInfo*)pki.data);
			}
			break;
		case DBC_START_FILE:
			{
				LogManagerSngl::instance()->on_startfile((DBStartFile*)pki.data);
				on_startfile((DBStartFile*)pki.data);
			}
			break;
		case DBC_SPEED_INFO:
			{
				on_speedinfo((DBSpeedInfo*)pki.data);
			}
			break;
		case DBC_DOWNLOADFILE_INFO:
			{
				LogManagerSngl::instance()->on_downloadfileinfo((DBDownloadFileInfo*)pki.data);
			}
			break;
		case DBC_DOWNLOAD_INFO:
			{
				LogManagerSngl::instance()->on_downloadinfo((DBDownloadInfo*)pki.data);
				on_downloadinfo((DBDownloadInfo*)pki.data);
			}
			break;
		case DBC_REALTIME_INFO:
			{
				LogManagerSngl::instance()->on_realtimeinfo((DBRealTimeInfo*)pki.data);
				on_realtimeinfo((DBRealTimeInfo*)pki.data);
			}
			break;
		default:
			assert(false);
			break;
		}
		free_data(pki);
	}
}
void DBManager::free_data(Packet &pki)
{
	switch(pki.cmd)
	{
	case DBC_USER_LOGIN:
		{
			delete (DBUserLogin*)pki.data;
		}
		break;
	case DBC_USER_LOGIN_OUT:
		{
			delete (DBUserLoginOut*)pki.data;
		}
		break;
	case DBC_ERROR_INFO:
		{
			delete (DBErrorInfo*)pki.data;
		}
		break;
	case DBC_START_FILE:
		{
			delete (DBStartFile*)pki.data;
		}
		break;
	case DBC_SPEED_INFO:
		{
			delete (DBSpeedInfo*)pki.data;
		}
		break;
	case DBC_DOWNLOADFILE_INFO:
		{
			delete (DBDownloadFileInfo*)pki.data;
		}
		break;
	case DBC_DOWNLOAD_INFO:
		{
			delete (DBDownloadInfo*)pki.data;
		}
		break;
	case DBC_REALTIME_INFO:
		{
			delete (DBRealTimeInfo*)pki.data;
		}
		break;
	default:
		assert(false);
		break;
	}
}
void DBManager::handle_timer()
{
	_DB_EMPTY_RETURN;
	if(m_last_ping_sec + 60 < m_curr_sec)
	{
		m_last_ping_sec = m_curr_sec;
		printf("-----DBConnector::ping()\n");
		//DBConnectorSngl::instance()->ping();
		if(!m_bwrite_trackerconf)
		{
			if(0==on_trackerconf())
			{
				m_bwrite_trackerconf = true;
			}
		}
	}
}

//insert into xxx () values ();
//void DBManager::on_userlogin(DBUserLogin *inf)
//{
//	_DB_EMPTY_RETURN;
//	//user_info 表 ,先update,不成功再insert
//	int rows;
//	sprintf(buf,"update user_info set last_login_time=now(),last_login_ip='%s',last_server_id=(last_server_id<<8)+%d,"
//		"peer_type=%d,prog_ver='%d',os_ver='%d' where uid='%s' ;",
//		Util::ip_htoas(inf->ip).c_str(),SettingSngl::instance()->get_tracker_id(),
//		inf->peer_type,inf->prog_ver,inf->os_ver,inf->uid);
//	//printf("SQL-->%s \n",buf);
//	if(0==query.query(buf,&rows) && rows<=0)
//	{
//		//数据库没有的，插入
//		sprintf(buf,"insert into user_info (uid,first_login_time,last_login_time,last_logout_time,last_login_ip,last_server_id,peer_type,prog_ver,os_ver,"
//	                 "dump_num,file_num,download_flow,upload_flow) "
//					 "values ('%s',now(),now(),now(),'%s',%d,%d,'%d','%d',"
//					 "0,0,0,0);"
//					 ,inf->uid,Util::ip_htoas(inf->ip).c_str(),SettingSngl::instance()->get_tracker_id(),inf->peer_type,inf->prog_ver,inf->os_ver);
//		//printf("SQL-->%s \n",buf);
//		query.query(buf);
//	}
//}
//void DBManager::on_userloginout(DBUserLoginOut *inf)
//{
//	_DB_EMPTY_RETURN;
//	//user_info 只管更新，不成功就算
//	sprintf(buf,"update user_info set last_logout_time=now(),dump_num=dump_num+%d,file_num=file_num+%d,download_flow=download_flow+%d,upload_flow=upload_flow+%d where uid='%s' ;",
//		inf->dump_num,inf->file_num,inf->download_flow,inf->upload_flow,inf->uid);
//	//printf("SQL-->%s \n",buf);
//	query.query(buf);
//}
//void DBManager::on_errorinfo(DBErrorInfo *inf)
//{
//	_DB_EMPTY_RETURN;
//	//error_info
//	sprintf(buf,"insert into error_info (server_id,record_time,miss_file_num,only_server_num,program_except_num,hash_check_fail_num) "
//		"values (%d,now(),%d,%d,%d,%d) ;",
//		SettingSngl::instance()->get_tracker_id(),inf->miss_file_num,inf->only_server_num,inf->program_except_num,inf->hash_check_fail_num);
//	//printf("SQL-->%s \n",buf);
//	query.query(buf);
//}
//void DBManager::on_startfile(DBStartFile *inf)
//{
//	_DB_EMPTY_RETURN;
//}
//void DBManager::on_speedinfo(DBSpeedInfo *inf)
//{
//	_DB_EMPTY_RETURN;
//	//sepeed_info
//	sprintf(buf,"insert into speed_info (server_id,uid,ip,hash_type,info_hash,size,speed,wait_num,download_duration,record_time) "
//		"values (%d,'%s','%s',%d,'%s',%lld,%d,%d,%d,now()) ;",
//		SettingSngl::instance()->get_tracker_id(),inf->uid,Util::ip_htoas(inf->ip).c_str(),(int)inf->hash_type,inf->str_hash,
//		inf->size,inf->speed,inf->wait_num,inf->download_duration);
//	//printf("SQL-->%s \n",buf);
//	query.query(buf);
//}
//void DBManager::on_downloadinfo(DBDownloadInfo *inf)
//{
//	_DB_EMPTY_RETURN;
//	//inc_info
//	sprintf(buf,"insert into inc_info (server_id,record_time,request_files,tcp_download_flow,udp_download_flow,http_download_flow,"
//		"edge_download_flow,client_download_flow,center_download_flow,super_download_flow,"
//		"tcp_connect_success,tcp_connect_fail,tcp_accept_success,tcp_accept_fail,udp_connect_success,udp_connect_fail,"
//		"udp_receive_success,udp_receive_fail,udp_nat_success,udp_nat_fail) "
//		"values (%d,now(),%d,%d,%d,%d,"
//		"%d,%d,%d,%d,"
//		"%d,%d,%d,%d,%d,%d,"
//		"%d,%d,%d,%d);",
//		SettingSngl::instance()->get_tracker_id(),inf->request_files,inf->tcp_download_flow,inf->udp_download_flow,inf->http_download_flow,
//		inf->edge_download_flow,inf->client_download_flow,inf->center_download_flow,inf->super_download_flow,
//		inf->tcp_connect_success,inf->tcp_connect_fail,inf->tcp_accept_success,inf->tcp_accept_fail,inf->udp_connect_success,inf->udp_connect_fail,
//		inf->udp_receive_success,inf->udp_receive_fail,inf->udp_nat_success,inf->udp_nat_fail);
//	//printf("SQL-->%s \n",buf);
//	query.query(buf);
//}
//void DBManager::on_realtimeinfo(DBRealTimeInfo *inf)
//{
//	_DB_EMPTY_RETURN;
//	//real_info
//	sprintf(buf,"insert into real_info (server_id,record_time,online_users,edge_servers,center_servers,super_servers,"
//		"online_progs,online_sources,"
//		"nat_0,nat_1,nat_2,nat_3,nat_4,nat_5) "
//		"values (%d,now(),%d,%d,%d,%d,"
//		"%d,%d,"
//		"%d,%d,%d,%d,%d,%d);",
//		SettingSngl::instance()->get_tracker_id(),inf->online_users,inf->edge_servers,inf->center_servers,inf->super_servers,
//		inf->online_progs,inf->online_sources,
//		inf->nat_0,inf->nat_1,inf->nat_2,inf->nat_3,inf->nat_4,inf->nat_5);
//	//printf("SQL-->%s \n",buf);
//	query.query(buf);
//}
//
//int DBManager::on_trackerconf()
//{
//	if(_DB_EMPTY)
//		return -1;
//	//tracker_info
//	int rows = 0,ret=0;
//	sprintf(buf,"update tracker_info set ip='%s',insert_time=now() where id=%d ; ",
//		SettingSngl::instance()->get_tracker_dblog_ip().c_str(),SettingSngl::instance()->get_tracker_id());
//	//printf("SQL-->%s \n",buf);
//	if(0==(ret=query.query(buf,&rows)) && rows<=0)
//	{
//		sprintf(buf,"insert into tracker_info (id,ip,description,insert_time) values (%d,'%s',' ',now()) ;",
//			SettingSngl::instance()->get_tracker_id(),SettingSngl::instance()->get_tracker_dblog_ip().c_str());
//		//printf("SQL-->%s \n",buf);
//		ret = query.query(buf);
//	}
//	return ret;
//}


