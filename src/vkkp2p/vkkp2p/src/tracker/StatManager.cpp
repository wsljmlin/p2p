#include "StatManager.h"
#include "Util.h"
#include "UserManager.h"
#include "PeerManager.h"
#include "SourceService.h"
#include "DBManager.h"
#include "Setting.h"

//***************************************************************************
string format_curr_day()
{
	time_t tt = time(0);
	tm* t = localtime(&tt);
	char buf[32];
	sprintf(buf,"%04d.%02d.%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday);
	return buf;
}

//***************************************************************************
StatManager::StatManager(void)
{
	m_begin_time = time(0);
	tm* t = localtime(&m_begin_time);
	char buf[64];
	sprintf(buf,"%04d.%02d.%02d %02d:%02d;%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	m_str_begin_time = buf;

	m_str_curr_day = format_curr_day();
	Util::my_create_directory("./tracker_log");
	reset();
	m_server_num = 0;
	m_center_num = 0;
	m_client_num = 0;
	m_super_num = 0;
}

StatManager::~StatManager(void)
{
}
void StatManager::reset()
{
	int i=0;
	m_req_file_num = 0;
	m_client_dump_num = 0;
	m_seach_nonfile = 0;
	m_seach_nonsource = 0;
	m_download_wrong_num = 0;
	
	for(i=0;i<5;++i)
	{
		connect_succceed_count[i] = 0;
		connect_failed_count[i] = 0;
	}
	for(i=0;i<3;++i)
	{
		download_bytes_through_channel[i] = 0;
	}
	for(i=0;i<5;++i)
	{
		download_bytes_from_peer[i] = 0;
	}
	for(i=0;i<7;++i)
		m_nat_type_num[i]=0;

}
void StatManager::on_req_file(const char* hashbuf,UserInfo* user) 
{ 
	m_req_file_num++; 
	
	Packet pack;
	DBStartFile *inf = new DBStartFile();
	hash_t hash;
	hash.set_buffer((unsigned char*)hashbuf);
	hash.to_string(inf->str_hash,48);
	inf->hash_type = hash.hash_type();
	inf->ip = user->ip;
	strcpy(inf->uid,user->uid);
	inf->user_type = user->userType;

	pack.cmd = DBC_START_FILE;
	pack.data = inf;
	if(!DBManagerSngl::instance()->put_packet(pack))
		delete inf;
}

#ifdef SM_VOD
void StatManager::on_req_file(const char* hashbuf,UserInfo* user,int filetype) {
	m_req_file_num++; 
	
	Packet pack;
	DBStartFile *inf = new DBStartFile();
	hash_t hash;
	hash.set_buffer((unsigned char*)hashbuf);
	hash.to_string(inf->str_hash,48);
	inf->hash_type = hash.hash_type();
	inf->ip = user->ip;
	inf->filetype = filetype;
	strcpy(inf->uid,user->uid);
	inf->user_type = user->userType;

	pack.cmd = DBC_START_FILE;
	pack.data = inf;
	if(!DBManagerSngl::instance()->put_packet(pack))
		delete inf;

}

#endif
void StatManager::on_login_in(int userType)
{
	if(UT_CLIENT==userType)
	{
		m_client_num++;
	}
	else if(UT_SERVER==userType)
	{
		m_server_num++;
	}
	else if(UT_CENTER==userType)
	{
		m_center_num++;
	}
	else if(UT_SUPER==userType)
	{
		m_super_num++;
	}

}
void StatManager::on_login_out(int userType)
{
	if(UT_CLIENT==userType)
	{
		m_client_num--;
	}
	else if(UT_SERVER==userType)
	{
		m_server_num--;
	}
	else if(UT_CENTER==userType)
	{
		m_center_num--;
	}
	else if(UT_SUPER==userType)
	{
		m_super_num--;
	}
}

void StatManager::on_nonfile_seach(const hash_t& hash)
{
	char shash[45];
	string tth;
	hash.to_string(shash,45);
	tth = shash;
	m_seach_nonfile++;

	char buf[64];
	sprintf(buf,"#***no file:%s",tth.c_str());
	char path[64];
	sprintf(path,"./tracker_log/%s.err.log",m_str_curr_day.c_str());
	Util::write_log(buf,path);
}
void StatManager::on_nonsource_seach(const hash_t& hash)
{
	char shash[45];
	string tth;
	hash.to_string(shash,45);
	tth = shash;
	m_seach_nonsource++;

	char buf[64];
	char path[64];
	sprintf(buf,"***no source:%s",tth.c_str());
	sprintf(path,"./tracker_log/%s.err.log",m_str_curr_day.c_str());
	Util::write_log(buf,path);
}
void StatManager::on_noncssource_seach(const hash_t& hash)
{
	char shash[45];
	string tth;
	hash.to_string(shash,45);
	tth = shash;
	m_seach_nonsource++;

	char buf[64];
	char path[64];
	sprintf(buf,"***no cs source(have center):%s",tth.c_str());
	sprintf(path,"./tracker_log/%s.err.log",m_str_curr_day.c_str());
	Util::write_log(buf,path);
}

void StatManager::handle_timeout()
{
	m_str_curr_day = format_curr_day();
	//on_timer_log();
	on_timer_dblog();
	reset();
}

int StatManager::get_msg_tracker_info(MsgTrackerInfo& msg)
{
	msg.begin_time = m_str_begin_time;
	msg.tracker_id = SettingSngl::instance()->get_tracker_id();
	msg.tracker_ver = TRACKER_VERSION;
	msg.user_num = UserManagerSngl::instance()->get_user_num();
	msg.peer_num = PeerManagerSngl::instance()->get_peer_num();
	msg.file_num = SourceServiceSngl::instance()->get_file_num();
	msg.ut_client = m_client_num;
	msg.ut_server = m_server_num;
	msg.ut_center = m_center_num;
	msg.ut_super = m_super_num;
	UserManagerSngl::instance()->get_nat_type(msg.nt);
	return 0;
}


void StatManager::on_client_error(const PTL_P2T_ReportError& req)
{
	//todo:
}

void StatManager::on_download_wrong(const PTL_P2T_ReportDownloadWrong& req,UserInfo &pi)
{
	m_download_wrong_num++;
	char buf[256];
	char path[64];

	char oldHash[45],newHash[45];
	hash_t hash;
	hash.set_buffer((uchar*)req.fhash);
	hash.to_string(oldHash,45);
	hash.set_buffer((uchar*)req.fhashnew);
	hash.to_string(newHash,45);
	sprintf(buf,"download wrong: ip=0x%x,uid=%s,ver=%d,oldhash=%s,newhash=%s",pi.ip,pi.uid,pi.version,
		oldHash,newHash);

	sprintf(path,"./tracker_log/%s.err.log",m_str_curr_day.c_str());
	Util::write_log(buf,path);
}
void StatManager::on_req_stat(const PTL_P2T_ReportStat& st)
{
	int i=0;
	for(i=0;i<5;++i)
	{
		connect_succceed_count[i] += st.connSucceedPerNetT[i];
		connect_failed_count[i] += st.connFailedPerNetT[i];
	}
	for(i=0;i<3;++i)
	{
		download_bytes_through_channel[i] += (st.downBytesPerIPT_KB[i]+512)>>10;
	}
	for(i=0;i<5;++i)
	{
		download_bytes_from_peer[i] += (st.downBytesPerUserT_KB[i]+512)>>10;
	}
}

void StatManager::on_timer_log()
{
	int user_num,user_max_num,user_max_pack_num;
	int ch_num,ch_max_num,ch_max_pack_num;
	int file_num; //文件数
	int source_num; //文件源次之各
#ifdef SM_VOD
	int file_num_vod; //文件数
	int source_num_vod; //文件源次之各
#endif /* end of SM_VOD */
	int nall;
	int conn_succeed_all,conn_failed_all;
	int byte_ch_all,byte_peer_all;
	int i=0;
	char path[64];
	string day = format_curr_day();

	user_num = UserManagerSngl::instance()->get_user_num();
	user_max_num = UserManagerSngl::instance()->get_max_user_num_and_reset();
	user_max_pack_num = UserManagerSngl::instance()->get_max_pack_num_and_reset();

	ch_num = PeerManagerSngl::instance()->get_peer_num();
	ch_max_num = PeerManagerSngl::instance()->get_max_peer_num_and_reset();
	ch_max_pack_num = PeerManagerSngl::instance()->get_max_pack_num_and_reset();
#ifdef SM_VOD
	SourceServiceSngl::instance()->get_source_num(file_num,source_num,file_num_vod,source_num_vod);
	SourceServiceSngl::instance()->print_sources();
#else
	SourceServiceSngl::instance()->get_source_num(file_num,source_num);
#endif /* end of SM_VOD */
	UserManagerSngl::instance()->get_nat_type(m_nat_type_num);
	nall=0;
	for(i=0;i<7;++i)
		nall += m_nat_type_num[i];

	char buf[4096];
	sprintf(buf,"un/cn=%d/%d ; clt/svr/cen=%d/%d/%d ; umn/cmn=%d/%d ; ump/cmp=%d/%d ; fn/sn=%d/%d ; natT=[nall=%d,n0=%d,n1=%d,n2=%d,n3=%d,n4=%d,n5=%d,n6=%d]",
		user_num,ch_num,
		m_client_num,m_server_num,m_center_num,
		user_max_num,ch_max_num,
		user_max_pack_num,ch_max_pack_num,
		file_num,source_num,
		nall,m_nat_type_num[0],m_nat_type_num[1],m_nat_type_num[2],m_nat_type_num[3],m_nat_type_num[4],m_nat_type_num[5],m_nat_type_num[6]);

	sprintf(path,"./tracker_log/%s_line.log",day.c_str());
	Util::write_log(buf,path);

	/////////////////////////////////////////////////
	conn_succeed_all = 0;
	conn_failed_all = 0;
	for(i=0;i<5;++i)
	{
		conn_succeed_all += connect_succceed_count[i];
		conn_failed_all += connect_failed_count[i];
	}
	byte_ch_all=0;
	byte_peer_all=0;
	for(i=0;i<3;++i)
	{
		byte_ch_all += download_bytes_through_channel[i];
		byte_peer_all += download_bytes_from_peer[i];
	}
	sprintf(buf,"reqf=%d ; seach nof/nos=%d/%d ; cdump=%d ; dwrong=%d ; byte_ch/tcp/udp(MB)=%d/%d/%d ; byte_peer/svr/usr(MB)=%d/%d/%d ; cnn true/false=[all=%d/%d,tcp=%d/%d,tcp_bk=%d/%d,udp=%d/%d,udp_bk=%d/%d,udp_nat=%d/%d]",
		m_req_file_num,
		m_seach_nonfile,m_seach_nonsource,
		m_client_dump_num,
		m_download_wrong_num,
		byte_ch_all,download_bytes_through_channel[0],download_bytes_through_channel[1],
		byte_peer_all,download_bytes_from_peer[0],download_bytes_from_peer[1],
		conn_succeed_all,conn_failed_all,
		connect_succceed_count[0],connect_failed_count[0],
		connect_succceed_count[1],connect_failed_count[1],
		connect_succceed_count[2],connect_failed_count[2],
		connect_succceed_count[3],connect_failed_count[3],
		connect_succceed_count[4],connect_failed_count[4]);

	sprintf(path,"./tracker_log/%s_inc.log",day.c_str());
	Util::write_log(buf,path);

	if(day!=m_str_curr_day)
	{
		m_str_curr_day = day;
		reset();
	}
}
void StatManager::on_timer_dblog()
{
	DBErrorInfo *err_inf = new DBErrorInfo();
	DBDownloadInfo *down_inf = new DBDownloadInfo();
	DBRealTimeInfo * real_inf = new DBRealTimeInfo();
	Packet pack;
	if(err_inf)
	{
		err_inf->hash_check_fail_num = this->m_download_wrong_num;
		err_inf->miss_file_num = this->m_seach_nonfile;
		err_inf->only_server_num = this->m_seach_nonsource;
		err_inf->program_except_num = this->m_client_dump_num;

		pack.cmd = DBC_ERROR_INFO;
		pack.data = err_inf;
		if(!DBManagerSngl::instance()->put_packet(pack))
			delete err_inf;
	}

	if(down_inf)
	{
		down_inf->request_files = this->m_req_file_num;
		
		down_inf->tcp_download_flow = this->download_bytes_through_channel[0];
		down_inf->udp_download_flow = this->download_bytes_through_channel[1];

		down_inf->client_download_flow = this->download_bytes_from_peer[0];
		down_inf->edge_download_flow = this->download_bytes_from_peer[1];
		down_inf->http_download_flow = this->download_bytes_from_peer[2];
		down_inf->center_download_flow = this->download_bytes_from_peer[3];
		down_inf->super_download_flow = this->download_bytes_from_peer[4];

		down_inf->tcp_connect_success = this->connect_succceed_count[0];
		down_inf->tcp_connect_fail = this->connect_failed_count[0];
		down_inf->tcp_accept_success = this->connect_succceed_count[1];
		down_inf->tcp_accept_fail = this->connect_failed_count[1];
		down_inf->udp_connect_success = this->connect_succceed_count[2];
		down_inf->udp_connect_fail = this->connect_failed_count[2];
		down_inf->udp_receive_success = this->connect_succceed_count[3];
		down_inf->udp_receive_fail = this->connect_failed_count[3];
		down_inf->udp_nat_success = this->connect_succceed_count[4];
		down_inf->udp_nat_fail = this->connect_failed_count[4];

		pack.cmd = DBC_DOWNLOAD_INFO;
		pack.data = down_inf;
		if(!DBManagerSngl::instance()->put_packet(pack))
			delete down_inf;
	}

	if(real_inf)
	{
		int file_num,source_num;
#ifdef SM_VOD
		int file_num_vod,source_num_vod;
		SourceServiceSngl::instance()->get_source_num(file_num,source_num, file_num_vod,source_num_vod);
		SourceServiceSngl::instance()->print_sources();
#else
		SourceServiceSngl::instance()->get_source_num(file_num,source_num);
#endif /* end of SM_VOD */
		UserManagerSngl::instance()->get_nat_type(m_nat_type_num);

		real_inf->online_users = UserManagerSngl::instance()->get_max_user_num_and_reset();//this->m_client_num + this->m_server_num + this->m_center_num + this->m_super_num;
		real_inf->edge_servers = this->m_server_num;
		real_inf->center_servers = this->m_center_num;
		real_inf->super_servers = this->m_super_num;

#ifdef SM_VOD
		real_inf->online_progs = file_num + file_num_vod;
		real_inf->online_sources = source_num + source_num_vod;
#else
		real_inf->online_progs = file_num;
		real_inf->online_sources = source_num;
#endif /* end of SM_VOD */

		real_inf->nat_0 = this->m_nat_type_num[0];
		real_inf->nat_1 = this->m_nat_type_num[1];
		real_inf->nat_2 = this->m_nat_type_num[2];
		real_inf->nat_3 = this->m_nat_type_num[3];
		real_inf->nat_4 = this->m_nat_type_num[4];
		real_inf->nat_5 = this->m_nat_type_num[5];

		pack.cmd = DBC_REALTIME_INFO;
		pack.data = real_inf;
		if(!DBManagerSngl::instance()->put_packet(pack))
			delete real_inf;
	}
}



