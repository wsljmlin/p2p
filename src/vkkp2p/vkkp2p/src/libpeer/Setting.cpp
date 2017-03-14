#include "Setting.h"
#include "IniFile.h"
#include "Util.h"
#include "Httpc.h"
#include "md5.h"

Setting::Setting(void)
{
	init_default_value();
}

Setting::~Setting(void)
{
}

void Setting::load_arg(int argc,char** argv)
{
	string str1,str2;
	for(int i=1;i<argc;++i)
	{
		str1 = argv[i];
		str2 = "";
		if(i+1<argc && argv[i+1][0]!='-')
		{
			i++;
			str2 = argv[i];
		}
		m_args[str1] = str2;
	}
}
int Setting::init()
{
	m_conf_path = "./";
	m_log_path = m_conf_path + "log/";
	m_data_path = m_conf_path + "data/";
	m_path_localm3u8 = m_conf_path + "localm3u8.inf";

	if ( m_peerconfig_name.empty() == true )
		m_confini_path = m_conf_path + "peerconfig.ini";
	else
		m_confini_path = m_conf_path + m_peerconfig_name;

	Util::my_create_directory(m_log_path);
	Util::my_create_directory(m_data_path);

	if (  m_strict_mode != 0 || m_peerconfig_name == "none" ) {
		// skip local config
		printf("skip local config\n");
	}
	else {
		printf("local local config %s\n", m_confini_path.c_str());
		load_peerconf();
	}

	check_arg();
	return 0;
}
void Setting::fini()
{
}

int Setting::update_user_id(puid_t uid)
{
	//
	m_user_id = uid;
	WritePrivateProfileStringA("local","user_id",m_user_id.c_str(),m_confini_path.c_str());
	return 0;
}
void Setting::set_memcache_only()
{
	m_cache_flag = 0;
	m_cache_flag_vod = 0;
}
void Setting::check_arg()
{
	// -m:次版本， -p: 参数为2个端口（accept_port:http_port）
	if(m_args.find("-m")!=m_args.end())
	{
		m_is_minor_version = 1;
	}
	map<string,string>::iterator it=m_args.find("-p");
	if(it!=m_args.end())
	{
		string str = it->second;
		int port=0;
		port = atoi(Util::get_string_index(str,0,":").c_str());
		if(port>0)
			m_accept_port = port;
		port = atoi(Util::get_string_index(str,1,":").c_str());
		if(port>0)
			m_http_port = port;
	}
}


void Setting::load_peerconf( const char * content ) 
{
	char buf[1024];
	string str;
	int i=0;
	IniFile ini;
	if(0==ini.parse(content))
	{
		//**************************************************
		//comm

		if ( m_ostype == "" ) 
			m_ostype = ini.read_string("comm", "ostype", "none", buf, 1024);

		str = ini.read_string("comm","tracker_ip",m_tracker_ip.c_str(),buf,1024);
		m_tracker_ip = str;

		i = ini.read_int("comm","tracker_port", m_tracker_port);
		m_tracker_port = i;

		str = ini.read_string("comm","stun_ip",m_stun_ip.c_str(),buf,1024);
		m_stun_ip = str;

		i = ini.read_int("comm","stun_port", m_stun_port);
		m_stun_port = i;

		str = ini.read_string("comm","accept_ip",m_accept_ip.c_str(),buf,1024);
		m_accept_ip = str;

		i = ini.read_int("comm","accept_port",m_accept_port);
		m_accept_port = i;

		i = ini.read_int("comm","http_port",m_http_port);
		m_http_port = i;
		
		str = ini.read_string("comm","message_ip",m_message_ip.c_str(),buf,1024);
		m_message_ip = str;

		i = ini.read_int("comm","message_port",m_message_port);
		m_message_port = i;
		

		//*************************************************
		//local
		str = ini.read_string("local","user_id", m_user_id.c_str(),buf,1024);
		m_user_id = str;

		i = ini.read_int("local","user_type", m_user_type);
		m_user_type = i;

		i = ini.read_int("local","nat_type", m_nat_type);
		m_nat_type = i;

		str = ini.read_string("local","user_ip", Util::ip_htoa(m_user_ip),buf,1024);
		m_user_ip = Util::ip_atoh(str.c_str());

		i = ini.read_int("local","menu", m_menu);
		m_menu = i;

		i = ini.read_int("local","is_minor_version", m_is_minor_version);
		m_is_minor_version = i;

		i = ini.read_int("local","block_size", m_block_size);
		if(0==i) i=102400;
		if(i<10240) i=10240;
		m_block_size = i;
		
		i = ini.read_int("local","sn_multinum", m_sn_multinum);
		if(i<1) i=1;
		if(i>20) i=20;
		m_sn_multinum = i;

		i = ini.read_int("local","assign_num", m_assign_num);
		if(i<1) i=1;
		m_assign_num = i;

		//m_is_use_super = 0;

		//*************************************************
		//cache
		str = ini.read_string("cache","cache_path","",buf,1024);
		printf("get cache path=%s\n", str.c_str());
		check_set_cache_path(str);
		
		str = ini.read_string("cache","download_path",m_download_path.c_str(),buf,1024);
		m_download_path = str;

		i = ini.read_int("cache","cache_sizeMB",m_cache_sizeMB);
		m_cache_sizeMB = i;
		
		i = ini.read_int("cache","disk_min_free_spaceMB",m_disk_min_free_spaceMB);
		m_disk_min_free_spaceMB = i;
		
		i = ini.read_int("cache","cache_rdbf",m_cache_filetype == RDBF_AUTO ? 1: 0);
		m_cache_filetype = i?RDBF_AUTO:RDBF_BASE;

		i = ini.read_int("cache","cache_flag",m_cache_flag);
		m_cache_flag = i; 
		
		i = ini.read_int("cache","cache_flag_vod", m_cache_flag_vod);
		m_cache_flag_vod = i; 

		i = ini.read_int("cache","memcache_winMB", m_memcache_size>>20);
		if(i<=0) i=5;
		m_memcache_size = i<<20;
		m_memcache_win = m_memcache_size/m_block_size;
		
		i = ini.read_int("cache","memcache_playlist_win",m_memcache_playlist_win);
		if(i<1) i=1;
		m_memcache_playlist_win = i;

		i = ini.read_int("cache","diskcache_playlist_num",m_diskcache_playlist_num);
		if(i<1) i=1;
		m_diskcache_playlist_num = i;

		char buf2[1024];
		sprintf(buf2,"%d,%d",m_playlist_http_pause_win_down,m_playlist_http_pause_win_up);
		str = ini.read_string("cache","playlist_http_pause_win",buf2,buf,1024);
		m_playlist_http_pause_win_down = atoi(Util::get_string_index(str,0,",").c_str());
		m_playlist_http_pause_win_up = atoi(Util::get_string_index(str,1,",").c_str());
		if(m_playlist_http_pause_win_down>m_playlist_http_pause_win_up)
			m_playlist_http_pause_win_down = m_playlist_http_pause_win_up;
		
		i = ini.read_int("cache","is_clear_url2_cache", m_is_clear_url2_cache);
		m_is_clear_url2_cache = i; 

		i = ini.read_int("cache","urgent_win", m_urgent_win);
		if(i<1) i=1;
		m_urgent_win = i;
		
		i = ini.read_int("cache","smooth_win", m_smooth_win);
		if(i<1) i=1;
		m_smooth_win = i;
		
		i = ini.read_int("cache","random_win", m_random_win);
		if(i<1) i=1;
		m_random_win = i;
		
		i = ini.read_int("cache","http_pause_win", m_http_pause_win );
		if(i>0 && i<3) i=3;
		m_http_pause_win = i;
		
		i = ini.read_int("cache","http_no_keep_alive", m_http_no_keep_alive);
		if(i!=1) i=0;
		m_http_no_keep_alive = i;

		//*************************************************
		//limit
		i = ini.read_int("limit","download_speed_KB", m_download_speed >> 10 );
		m_download_speed = i<<10;

		i = ini.read_int("limit","downloadi_cnns", m_downloadi_cnns);
		if(i<=0) i=50;
		m_downloadi_cnns = i;
		
		i = ini.read_int("limit","downloadhttpi_cnns", m_downloadhttpi_cnns);
		if(i>10) i=10;
		if(i<0) i=0; //不再允许出现-1的情况
		m_downloadhttpi_cnns = i;

		i = ini.read_int("limit","share_speed_KB", m_share_speed>>10);
		m_share_speed = i<<10;

		i = ini.read_int("limit","share_cnns", m_share_cnns);
		m_share_cnns = i;
		
		i = ini.read_int("limit","sharefiles_max_timer_sec",m_sharefiles_max_timer_sec);
		m_sharefiles_max_timer_sec = i?i:120;
		
		i = ini.read_int("limit","download_maxnum", m_download_maxnum);
		if(i<=0) i=3;
		if(i>20) i=20;
		m_download_maxnum = i;

		i = ini.read_int("limit","downloadlist_maxnum", m_downloadlist_maxnum);
		if(i<=0) i=10;
		if(i>100) i=100;
		m_downloadlist_maxnum = i;

		i = ini.read_int("limit","downloadlist_active_partnum", m_downloadlist_active_partnum );
		if(i<1) i=1;
		if(i>100) i=100;
		m_downloadlist_active_partnum = i;

		i = ini.read_int("limit","min_httpi_speedKB", m_min_httpi_speedB >> 10);
		if(i<0) i=0;
		m_min_httpi_speedB = i<<10;

#ifdef SM_VOD
		i = ini.read_int("limit","http_close_peercnt", m_http_close_peercnt);
		if(i<0) i=5;
		m_http_close_peercnt = i;
#endif /* end of SM_VOD */
		//****************************************************
		//extra
		
		m_update_peerconf = ini.read_int("extra", "update_peerconf", m_update_peerconf );
		m_log_mode = ini.read_string("extra", "log_mode", m_log_mode.c_str(), buf, 1024 );
		m_preload_playlist = ini.read_string("extra", "preload_playlist", m_preload_playlist.c_str(), buf, 1024);
		m_daemon_mode = ini.read_int("extra", "daemon_mode", m_daemon_mode );
		m_peerconfig_name = ini.read_string("extra", "peerconfig_name", m_peerconfig_name.c_str(), buf, 1024 );
		m_playlist_timeshift = ini.read_int("extra", "playlist_timeshift", m_playlist_timeshift );
		m_playlist_delaynumber = ini.read_int("extra", "playlist_delaynumber", m_playlist_delaynumber );
		m_playlist_size = ini.read_int("extra", "playlist_size", m_playlist_size );
		m_export_list = ini.read_int("extra", "export_list", m_export_list );
		m_export_prefix = ini.read_string("extra", "export_prefix", m_export_prefix.c_str(), buf, 1024);
		m_export_filename = ini.read_string("extra", "export_filename", m_export_filename.c_str(), buf, 1024);
		m_export_logfile = ini.read_string("extra", "export_logfile", m_export_logfile.c_str(), buf, 1024);
		m_export_timeshift = ini.read_int("extra", "export_timeshift", m_export_timeshift );
		m_hosts_file = ini.read_string("extra", "hosts_file", m_hosts_file.c_str() , buf , 1024);
		m_localonly = ini.read_int("extra", "localonly", m_localonly );
		m_strict_mode = ini.read_int("extra", "strict_mode", m_strict_mode );
		m_auth_response = ini.read_int("extra", "auth_response", m_auth_response);
		m_auth_message = ini.read_string("extram", "auth_message", m_auth_message.c_str(), buf, 1024);
	
		m_force_download = ini.read_int("test", "force_download",0);
}

	if(m_menu & MENU_VIP)
		m_is_use_super = 1;
	else
		m_is_use_super = 0;

	unsigned int new_menu = m_http_port;
	new_menu = new_menu << 16;
	m_menu = new_menu | (m_menu&0x00ff);
}


void Setting::init_default_value()
{
	//comm
		m_tracker_ip = "";
		m_tracker_port = 80;
		m_stun_ip = "";
		m_stun_port = 7150;
		m_accept_ip = "";
		m_accept_port = 7170;
		m_http_port = 7080;
		m_message_ip = "";
		m_message_port = 0;

	//local
		m_user_id = "";
		m_user_type = UT_CLIENT;
		m_nat_type = 6;
		m_user_ip = 0;
		m_menu = 7;
		m_is_minor_version = 0;
		m_block_size = 102400;
		m_sn_multinum = 1;
		m_is_use_super = 0;
		m_assign_num = 2;

	//cache
		m_cache_path = m_data_path + "cache/";
		m_download_path = "";
		m_cache_sizeMB = 1024;
		m_disk_min_free_spaceMB = 1024;
		m_cache_filetype = RDBF_AUTO;
		m_cache_flag = 1; //写硬盘
		m_cache_flag_vod = 1; //写硬盘
		m_memcache_size = 5<<20;
		m_memcache_win = m_memcache_size/m_block_size;
		m_memcache_playlist_win = 2;
		m_diskcache_playlist_num = 10;
		m_playlist_http_pause_win_down = -1;
		m_playlist_http_pause_win_up = -1;
		m_is_clear_url2_cache = 1;
		m_urgent_win=2;  //紧急窗，抢动作，随机1
		m_smooth_win=6;	 //平稳窗，随机1； 应该为紧急窗的2倍以上为佳
		m_random_win=5; //随机选分块任务
		m_http_pause_win=0; //允许http暂停的窗口大小，0表示不暂停
		m_http_no_keep_alive=0;

	//limit
		m_download_speed = 0;
		m_downloadi_cnns = 50;
		m_downloadhttpi_cnns = 1;
		m_share_speed = 0;
		m_share_cnns = 500;
		m_sharefiles_max_timer_sec = 120;
		m_download_maxnum = 3;
		m_downloadlist_maxnum = 10;
		m_downloadlist_active_partnum = 1;
		m_min_httpi_speedB = 0;
#ifdef SM_VOD
		m_http_close_peercnt = 5;
#endif /* end of SM_VOD */


	// extra
		m_update_peerconf=0;
                m_log_mode="";
                m_preload_playlist="playlist.inf";
                m_daemon_mode=0;
                m_peerconfig_name = "peerconfig.ini";

                m_playlist_timeshift=0;
                m_playlist_delaynumber=0;
                m_playlist_size=0;
                m_export_list = 0;
                m_export_prefix = "";
                m_export_filename = "";
                m_export_logfile = "";
                m_export_timeshift = 0;
                m_hosts_file = "";
                m_localonly = 1;
                m_strict_mode = 0;

                m_auth_response = 0;
                m_auth_message = "";

	m_force_download = 0;

	// anti session-based url config!
		m_m3u8_tailer = "?";
		m_tsseg_tailer = "?";

		m_ostype = "";

}

void Setting::load_peerconf()
{
	char buf[1030];
	FILE * cfile = fopen(m_confini_path.c_str(),"r");
        if ( cfile !=NULL )
        {
                string str ="";
                while( !feof(cfile) ) {
                        int size = (int)fread( buf, 1, 1024, cfile );
                        if ( size <= 0 ) break;
                        buf[size] = 0;
                        str += buf;
                }
                fclose( cfile);
                load_peerconf( str.c_str() );
        }

	save_peerconf();
}

void Setting::save_peerconf()
{
	IniFile ini;
	char buf[1024];
	if(0!=ini.open(m_confini_path.c_str()))  {
		printf("can not save peerconfig.ini\n");
		return ;
	}

	if ( m_update_peerconf == 0 ) {
		// only save few items for security!
		ini.write_int("comm","version",VERSION_NUM);
		ini.write_int("comm","accept_port",m_accept_port);
		ini.write_int("comm","http_port",m_http_port);

		ini.write_string("local","user_id",m_user_id.c_str());
		ini.write_string("local","user_ip",Util::ip_htoa(m_user_ip));
		ini.write_int("local","nat_type",m_nat_type);
		// nat_type must be write, since some other function
		// need read peerconfig.ini to load the value
		// todo, change the code to get data here

	}
	else
	if ( m_update_peerconf == 1 ) {
		//**************************************************
		//comm
		ini.write_int("comm","version",VERSION_NUM);
		ini.write_string("comm","tracker_ip",m_tracker_ip.c_str());
		ini.write_int("comm","tracker_port",m_tracker_port);
		ini.write_string("comm","stun_ip",m_stun_ip.c_str());
		ini.write_int("comm","stun_port",m_stun_port);
		ini.write_string("comm","accept_ip",m_accept_ip.c_str());
		ini.write_int("comm","accept_port",m_accept_port);
		ini.write_int("comm","http_port",m_http_port);
		ini.write_string("comm","message_ip",m_message_ip.c_str());
		ini.write_int("comm","message_port",m_message_port);

		//*************************************************
		//local
		ini.write_string("local","user_id",m_user_id.c_str());
		ini.write_int("local","user_type",m_user_type);
		ini.write_int("local","nat_type",m_nat_type);
		ini.write_string("local","user_ip",Util::ip_htoa(m_user_ip));
		ini.write_int("local","menu",m_menu);
		ini.write_int("local","is_minor_version",m_is_minor_version);
		ini.write_int("local","block_size",m_block_size);
		ini.write_int("local","sn_multinum",m_sn_multinum);
		ini.write_int("local","assign_num",m_assign_num);

		//*************************************************
		//cache
		ini.write_string("cache","cache_path",m_cache_path.c_str());
		ini.write_string("cache","download_path",m_download_path.c_str());
		ini.write_int("cache","cache_sizeMB",m_cache_sizeMB);
		ini.write_int("cache","disk_min_free_spaceMB",m_disk_min_free_spaceMB);
		ini.write_int("cache","cache_rdbf", m_cache_filetype == RDBF_AUTO ? 1: 0);
		ini.write_int("cache","cache_flag",m_cache_flag);
		ini.write_int("cache","cache_flag_vod",m_cache_flag_vod);
		ini.write_int("cache","memcache_winMB", m_memcache_size>>20);
		ini.write_int("cache","memcache_playlist_win",m_memcache_playlist_win);
		ini.write_int("cache","diskcache_playlist_num",m_diskcache_playlist_num);

		sprintf(buf,"%d,%d",m_playlist_http_pause_win_down,m_playlist_http_pause_win_up);
		ini.write_string("cache","playlist_http_pause_win",buf);

		ini.write_int("cache","is_clear_url2_cache",m_is_clear_url2_cache);
		ini.write_int("cache","urgent_win",m_urgent_win);
		ini.write_int("cache","smooth_win",m_smooth_win);
		ini.write_int("cache","random_win",m_random_win);
		ini.write_int("cache","http_pause_win",m_http_pause_win);	
		ini.write_int("cache","http_no_keep_alive",m_http_no_keep_alive);

		//*************************************************
		//limit
		ini.write_int("limit","download_speed_KB",m_download_speed>>10);
		ini.write_int("limit","downloadi_cnns",m_downloadi_cnns);
		ini.write_int("limit","downloadhttpi_cnns",m_downloadhttpi_cnns);
		ini.write_int("limit","share_speed_KB",m_share_speed>>10);
		ini.write_int("limit","share_cnns",m_share_cnns);
		ini.write_int("limit","sharefiles_max_timer_sec",m_sharefiles_max_timer_sec);
		ini.write_int("limit","download_maxnum",m_download_maxnum);
		ini.write_int("limit","downloadlist_maxnum",m_downloadlist_maxnum);
		ini.write_int("limit","downloadlist_active_partnum",m_downloadlist_active_partnum);
		ini.write_int("limit","min_httpi_speedKB",m_min_httpi_speedB>>10);
#ifdef SM_VOD
		ini.write_int("limit","http_close_peercnt",m_http_close_peercnt);
#endif /* end of SM_VOD */

		//****************************************************
		//extra
		ini.write_string("extra","log_mode",m_log_mode.c_str());
		ini.write_string("extra","preload_playlist",m_preload_playlist.c_str());
		ini.write_int("extra","daemon_mode",m_daemon_mode);

		ini.write_int("extra","playlist_timeshift",m_playlist_timeshift);
		ini.write_int("extra","playlist_delaynumber",m_playlist_delaynumber);
		ini.write_int("extra","playlist_size",m_playlist_size);
		ini.write_int("extra","export_list",m_export_list);
		ini.write_string("extra","export_prefix",m_export_prefix.c_str());
		ini.write_string("extra","export_filename",m_export_filename.c_str());
		ini.write_string("extra","export_logfile",m_export_logfile.c_str());
		ini.write_int("extra","export_timeshift",m_export_timeshift);
		ini.write_string("extra","hosts_file",m_hosts_file.c_str());
		ini.write_int("extra","localonly",m_localonly);
	}
	ini.close();

}

/*
#ifdef COMMENT_OLDCODE
void Setting::load_peerconf()
{
	char buf[1024];
	string str;
	int i=0;
	IniFile ini;
	if(0==ini.open(m_confini_path.c_str()))
	{
		//      **************************************************
		//comm
		ini.write_int("comm","version",VERSION_NUM);

		str = ini.read_string("comm","tracker_ip","",buf,1024);
		m_tracker_ip = str;
		ini.write_string("comm","tracker_ip",m_tracker_ip.c_str());

		i = ini.read_int("comm","tracker_port",8000);
		m_tracker_port = i;
		ini.write_int("comm","tracker_port",m_tracker_port);

		str = ini.read_string("comm","stun_ip","",buf,1024);
		m_stun_ip = str;
		ini.write_string("comm","stun_ip",m_stun_ip.c_str());

		i = ini.read_int("comm","stun_port",8111);
		m_stun_port = i;
		ini.write_int("comm","stun_port",m_stun_port);

		str = ini.read_string("comm","accept_ip","",buf,1024);
		m_accept_ip = str;
		ini.write_string("comm","accept_ip",m_accept_ip.c_str());

		i = ini.read_int("comm","accept_port",7170);
		m_accept_port = i;
		ini.write_int("comm","accept_port",m_accept_port);

		i = ini.read_int("comm","http_port",7080);
		m_http_port = i;
		ini.write_int("comm","http_port",m_http_port);
		
		str = ini.read_string("comm","message_ip","",buf,1024);
		m_message_ip = str;
		ini.write_string("comm","message_ip",m_message_ip.c_str());

		i = ini.read_int("comm","message_port",0);
		m_message_port = i;
		ini.write_int("comm","message_port",m_message_port);
		

		//    *************************************************
		//local
		str = ini.read_string("local","user_id","",buf,1024);
		m_user_id = str;
		ini.write_string("local","user_id",m_user_id.c_str());

		i = ini.read_int("local","user_type",UT_CLIENT);
		m_user_type = i;
		ini.write_int("local","user_type",m_user_type);

		str = ini.read_string("local","user_ip","",buf,1024);
		m_user_ip = Util::ip_atoh(str.c_str());
		ini.write_string("local","user_ip",str.c_str());

		i = ini.read_int("local","menu",7);
		m_menu = i;
		ini.write_int("local","menu",m_menu);

		i = ini.read_int("local","is_minor_version",0);
		m_is_minor_version = i;
		ini.write_int("local","is_minor_version",m_is_minor_version);

		i = ini.read_int("local","block_size",102400);
		if(0==i) i=102400;
		if(i<10240) i=10240;
		m_block_size = i;
		ini.write_int("local","block_size",m_block_size);
		
		i = ini.read_int("local","sn_multinum",1);
		if(i<1) i=1;
		if(i>20) i=20;
		m_sn_multinum = i;
		ini.write_int("local","sn_multinum",m_sn_multinum);

		//m_is_use_super = 0;

		//     *************************************************
		//cache
		str = ini.read_string("cache","cache_path",m_cache_path.c_str(),buf,1024);
		check_set_cache_path(str);
		ini.write_string("cache","cache_path",m_cache_path.c_str());
		
		str = ini.read_string("cache","download_path","",m_download_path.c_str(),1024);
		m_download_path = str;
		ini.write_string("cache","download_path",m_download_path.c_str());

		i = ini.read_int("cache","cache_sizeMB",m_cache_sizeMB);
		m_cache_sizeMB = i;
		ini.write_int("cache","cache_sizeMB",m_cache_sizeMB);
		
		i = ini.read_int("cache","disk_min_free_spaceMB",m_disk_min_free_spaceMB);
		m_disk_min_free_spaceMB = i;
		ini.write_int("cache","disk_min_free_spaceMB",m_disk_min_free_spaceMB);
		
		i = ini.read_int("cache","cache_rdbf", m_cache_filetype);
		m_cache_filetype = i?RDBF_AUTO:RDBF_BASE;
		ini.write_int("cache","cache_rdbf",i);

		i = ini.read_int("cache","cache_flag", m_cache_flag);
		m_cache_flag = i; 
		ini.write_int("cache","cache_flag",m_cache_flag);
		
		i = ini.read_int("cache","cache_flag_vod", m_cache_flag_vod);
		m_cache_flag_vod = i; 
		ini.write_int("cache","cache_flag_vod",m_cache_flag_vod);

		i = ini.read_int("cache","memcache_winMB", m_memcache_size>>20);
		if(i<=0) i=5;
		m_memcache_size = i<<20;
		m_memcache_win = m_memcache_size/m_block_size;
		ini.write_int("cache","memcache_winMB",i);
		
		i = ini.read_int("cache","memcache_playlist_win", m_memcache_playlist_win);
		if(i<1) i=1;
		m_memcache_playlist_win = i;
		ini.write_int("cache","memcache_playlist_win",i);

		i = ini.read_int("cache","diskcache_playlist_num",m_diskcache_playlist_num);
		if(i<1) i=1;
		m_diskcache_playlist_num = i;
		ini.write_int("cache","diskcache_playlist_num",i);

		sprintf(buf,"%d,%d",m_playlist_http_pause_win_down,m_playlist_http_pause_win_up);
		str = ini.read_string("cache","playlist_http_pause_win",buf,buf,1024);
		m_playlist_http_pause_win_down = atoi(Util::get_string_index(str,0,",").c_str());
		m_playlist_http_pause_win_up = atoi(Util::get_string_index(str,1,",").c_str());
		if(m_playlist_http_pause_win_down>m_playlist_http_pause_win_up)
			m_playlist_http_pause_win_down = m_playlist_http_pause_win_up;
		sprintf(buf,"%d,%d",m_playlist_http_pause_win_down,m_playlist_http_pause_win_up);
		ini.write_string("cache","playlist_http_pause_win",buf);
		
		i = ini.read_int("cache","is_clear_url2_cache",m_is_clear_url2_cache);
		m_is_clear_url2_cache = i; 
		ini.write_int("cache","is_clear_url2_cache",m_is_clear_url2_cache);

		i = ini.read_int("cache","urgent_win",m_urgent_win);
		if(i<1) i=1;
		m_urgent_win = i;
		ini.write_int("cache","urgent_win",i);
		
		i = ini.read_int("cache","smooth_win",m_smooth_win);
		if(i<1) i=1;
		m_smooth_win = i;
		ini.write_int("cache","smooth_win",i);
		
		i = ini.read_int("cache","random_win",m_random_win);
		if(i<1) i=1;
		m_random_win = i;
		ini.write_int("cache","random_win",i);
		
		i = ini.read_int("cache","http_pause_win",m_http_pause_win);
		if(i>0 && i<3) i=3;
		m_http_pause_win = i;
		ini.write_int("cache","http_pause_win",i);
		
		i = ini.read_int("cache","http_no_keep_alive",m_http_no_keep_alive);
		if(i!=1) i=0;
		m_http_no_keep_alive = i;
		ini.write_int("cache","http_no_keep_alive",i);

		//    *************************************************
		//limit
		i = ini.read_int("limit","download_speed_KB",m_download_speed>>10);
		m_download_speed = i<<10;
		ini.write_int("limit","download_speed_KB",i);

		i = ini.read_int("limit","downloadi_cnns",  m_downloadi_cnns );
		if(i<=0) i=50;
		m_downloadi_cnns = i;
		ini.write_int("limit","downloadi_cnns",m_downloadi_cnns);
		
		i = ini.read_int("limit","downloadhttpi_cnns", m_downloadhttpi_cnns);
		if(i>10) i=10;
		m_downloadhttpi_cnns = i;
		ini.write_int("limit","downloadhttpi_cnns",m_downloadhttpi_cnns);

		i = ini.read_int("limit","share_speed_KB", m_share_speed>>10);
		m_share_speed = i<<10;
		ini.write_int("limit","share_speed_KB",i);

		i = ini.read_int("limit","share_cnns",m_share_cnns );
		m_share_cnns = i;
		ini.write_int("limit","share_cnns",m_share_cnns);
		
		i = ini.read_int("limit","sharefiles_max_timer_sec",m_sharefiles_max_timer_sec);
		m_sharefiles_max_timer_sec = i?i:120;
		ini.write_int("limit","sharefiles_max_timer_sec",m_sharefiles_max_timer_sec);
		
		i = ini.read_int("limit","download_maxnum",m_download_maxnum);
		if(i<=0) i=3;
		if(i>20) i=20;
		m_download_maxnum = i;
		ini.write_int("limit","download_maxnum",m_download_maxnum);

		i = ini.read_int("limit","downloadlist_maxnum",m_downloadlist_maxnum);
		if(i<=0) i=10;
		if(i>100) i=100;
		m_downloadlist_maxnum = i;
		ini.write_int("limit","downloadlist_maxnum",m_downloadlist_maxnum);

		i = ini.read_int("limit","downloadlist_active_partnum", m_downloadlist_active_partnum);
		if(i<1) i=1;
		if(i>100) i=100;
		m_downloadlist_active_partnum = i;
		ini.write_int("limit","downloadlist_active_partnum",m_downloadlist_active_partnum);

		i = ini.read_int("limit","min_httpi_speedKB", m_min_httpi_speedB>>10);
		if(i<0) i=0;
		m_min_httpi_speedB = i<<10;
		ini.write_int("limit","min_httpi_speedKB",i);
	}
	else
	{
		//comm
		m_tracker_ip = "";
		m_tracker_port = 80;
		m_stun_ip = "";
		m_stun_port = 7150;
		m_accept_ip = "";
		m_accept_port = 7170;
		m_http_port = 7080;
		m_message_ip = "";
		m_message_port = 0;

		//local
		m_user_id = "";
		m_user_type = UT_CLIENT;
		m_user_ip = 0;
		m_menu = 7;
		m_is_minor_version = 0;
		m_block_size = 102400;
		m_sn_multinum = 1;
		m_is_use_super = 0;

		//cache
		m_cache_path = m_data_path + "cache/";
		m_download_path = "";
		m_cache_sizeMB = 1024;
		m_disk_min_free_spaceMB = 1024;
		m_cache_filetype = RDBF_AUTO;
		m_cache_flag = 1; //写硬盘
		m_cache_flag_vod = 1; //写硬盘
		m_memcache_size = 5<<20;
		m_memcache_win = m_memcache_size/m_block_size;
		m_memcache_playlist_win = 2;
		m_diskcache_playlist_num = 10;
		m_is_clear_url2_cache = 1;
		m_urgent_win=2;  //紧急窗，抢动作，随机1
		m_smooth_win=6;	 //平稳窗，随机1； 应该为紧急窗的2倍以上为佳
		m_random_win=5; //随机选分块任务
		m_http_pause_win=0; //允许http暂停的窗口大小，0表示不暂停
		m_http_no_keep_alive=0;

		//limit
		m_download_speed = 0;
		m_downloadi_cnns = 50;
		m_downloadhttpi_cnns = 1;
		m_share_speed = 0;
		m_share_cnns = 500;
		m_sharefiles_max_timer_sec = 120;
		m_download_maxnum = 3;
		m_downloadlist_maxnum = 10;
		m_downloadlist_active_partnum = 1;
		m_min_httpi_speedB = 0;

	}
	ini.close();

	if(m_menu & MENU_VIP)
		m_is_use_super = 1;
	else 
		m_is_use_super = 0;

	unsigned int new_menu = m_http_port;
	new_menu = new_menu << 16;
	m_menu = new_menu | (m_menu&0x00ff);
}
#endif
*/

void Setting::check_set_cache_path(const string& path)
{
	string tmppath = path;
	string str;
	if(!tmppath.empty())
	{
		//检查目录是否可用
		Util::my_create_directory(tmppath);
		char c = tmppath.at(tmppath.length()-1);
		if('/' != c && '\\' != c)
			tmppath += "/";
		str = tmppath + "test.vkk!";

		FILE *fp = fopen(str.c_str(),"wb+");
		if(fp)
		{
			fclose(fp);
			Util::file_delete(str);
			printf("#cache path(%s)	OK! \n",tmppath.c_str());
		}
		else
		{
			printf("#*** cache path(%s) unused! \n",tmppath.c_str());
			tmppath = "";
		}
	}

//#ifdef _WIN32
//	if(tmppath.empty())
//	{
//		//取最大的磁盘空间
//		list<string> ls;
//		Util::get_all_volumes(ls);
//		string max_dir;
//		ULONGLONG max_size=0;
//		ULONGLONG total,used,free;
//		for(list<string>::iterator it=ls.begin(); it!=ls.end(); ++it)
//		{
//			str = *it;
//			total=used=free=0;
//			if(Util::get_volume_size(str,total,used,free))
//			{
//				if(free>max_size)
//				{
//					max_size=free;
//					max_dir=str;
//				}
//			}
//		}
//
//		tmppath = max_dir + "VKKCache";
//		::CreateDirectoryA(tmppath.c_str(),NULL);
//		BOOL bret = SetFileAttributesA(tmppath.c_str(),FILE_ATTRIBUTE_HIDDEN);
//		tmppath += "\\cache\\";
//		Util::my_create_directory(tmppath);
//	}
//#endif

	if(tmppath.empty()) 
	{
		tmppath = m_conf_path + "data/cache/";
		Util::my_create_directory(tmppath);
	}
	if(tmppath.at(tmppath.length()-1)!='/' && tmppath.at(tmppath.length()-1)!='\\')
		tmppath += "/";
	set_cache_path(tmppath);
	printf("#Setting::cache_path = %s \n",tmppath.c_str());

}
