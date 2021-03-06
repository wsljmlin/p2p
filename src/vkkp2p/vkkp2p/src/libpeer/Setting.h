#pragma once
#include "commons.h"

#define MENU_SHARE   0x01
#define MENU_SUPPORT_TCP 0x02
#define MENU_SUPPORT_UDP 0x04

class Setting
{
public:
	Setting(void);
	~Setting(void);
	
	void load_arg(int argc,char** argv);
	int init();
	void fini();

	int update_user_id(puid_t uid);
	void set_memcache_only();
private:
	void check_arg();
	void load_peerconf();
	void check_set_cache_path(const string& path);
protected:
	map<string,string> m_args;
protected:
	//confpath
	GETSET(string,m_conf_path,_conf_path)
	GETSET(string,m_confini_path,_confini_path)
	GETSET(string,m_log_path,_log_path)
	GETSET(string,m_data_path,_data_path)
	GETSET(string,m_path_localm3u8,_path_localm3u8)

	//peer_config.ini
	//comm
	GETSET(string,m_tracker_ip,_tracker_ip)
	GETSET(unsigned short,m_tracker_port,_tracker_port)
	GETSET(string,m_stun_ip,_stun_ip)
	GETSET(unsigned short,m_stun_port,_stun_port)
	GETSET(string ,m_accept_ip,_accept_ip)
	GETSET(unsigned short ,m_accept_port,_accept_port) //tcp 跟 udp 一样端口
	GETSET(unsigned short ,m_http_port,_http_port) //tcp 跟 udp 一样端口
	GETSET(string ,m_message_ip,_message_ip)
	GETSET(unsigned short ,m_message_port,_message_port) //udp 广播动态信息（如开始下载playlist,就往此端口发一个广播消息）

	//local
	GETSET(string,m_user_id,_user_id)
	GETSET(char,m_user_type,_user_type)
	GETSET(char,m_nat_type,_nat_type)
	GETSET(unsigned int,m_user_ip,_user_ip) //对有于多个外网IP的服务器，支持通过填写指定外网用哪个IP,nat信息中使用
	GETSET(unsigned int,m_menu,_menu) //菜单功能位:低0位共享支持,低1位TCP支持,低2位UDP支持
	GETSET(char,m_is_minor_version,_is_minor_version) //次版本不写ready，major version写
	GETSET(unsigned int,m_block_size,_block_size) //默认应该为102400，最小为10240
	GETSET(unsigned int,m_sn_multinum,_sn_multinum) //默认为1,指同一个源建立多个连接
	GETSET(char,m_is_use_super,_is_use_super) //是否使用super源，0:不使用；VIP用户使用super源
	GETSET(unsigned int,m_assign_num,_assign_num) //默认为2,指一次分配多少下载片段

	//cache
	GETSET(string,m_cache_path,_cache_path)
	GETSET(string,m_download_path,_download_path)
	GETSET(unsigned int,m_cache_sizeMB,_cache_sizeMB)
	GETSET(unsigned int,m_disk_min_free_spaceMB,_disk_min_free_spaceMB) //保持最少磁盘剩余空间
	GETSET(int,m_cache_filetype,_cache_filetype) //
	GETSET(char,m_cache_flag,_cache_flag)
	GETSET(char,m_cache_flag_vod,_cache_flag_vod)
	GETSET(unsigned int,m_memcache_size,_memcache_size)
	GETSET(int,m_memcache_win,_memcache_win)
	GETSET(int,m_memcache_playlist_win,_memcache_playlist_win)
	GETSET(char,m_diskcache_playlist_num,_diskcache_playlist_num)
	GETSET(int,m_playlist_http_pause_win_down,_playlist_http_pause_win_down)
	GETSET(int,m_playlist_http_pause_win_up,_playlist_http_pause_win_up)
	GETSET(char,m_is_clear_url2_cache,_is_clear_url2_cache)
	GETSET(int,m_urgent_win,_urgent_win)  //紧急窗，抢动作，随机1
	GETSET(int,m_smooth_win,_smoot_win)	 //平稳窗，随机1； 应该为紧急窗的2倍以上为佳
	GETSET(int,m_random_win,_random_win) //随机选分块任务
	GETSET(int,m_http_pause_win,_http_pause_win) //允许http暂停的窗口大小，0表示不暂停
	GETSET(char,m_http_no_keep_alive,_http_no_keep_alive) //=1时表示不使用keep-alive功能

	//limit
	GETSET(int ,m_download_speed,_download_speed) //总的下载速度
	GETSET(int ,m_downloadi_cnns,_downloadi_cnns) //每个download的最大连接数
	GETSET(int ,m_downloadhttpi_cnns,_downloadhttpi_cnns) //每个downloadhttp的最大连接数
	GETSET(int ,m_share_speed,_share_speed)
	GETSET(int ,m_share_cnns,_share_cnns)
	GETSET(int ,m_sharefiles_max_timer_sec,_sharefiles_max_timer_sec) //最多多少秒内共享完
	GETSET(int ,m_download_maxnum,_download_maxnum)    //manual_download 并发数
	GETSET(int ,m_downloadlist_maxnum,_downloadlist_maxnum)		//最多支持多少个downloadlist 并发
	GETSET(int ,m_downloadlist_active_partnum,_downloadlist_active_partnum)  //downloadlist的 片段下载并发数
	GETSET(int ,m_min_httpi_speedB,_min_httpi_speedB) //如果大于0，侧HTTP连接速度过小时断开重连

	GETSET(int,m_force_download,_force_download) //用于测试下载.本地即使已经有文件也强制创建下载.
#ifdef SM_VOD
	GETSET(int ,m_http_close_peercnt,_http_close_peercnt) //http will close when peer count exceed this
#endif
	
private:
	void init_default_value();
	void save_peerconf();
	
public:
	void load_peerconf(const char * content);

protected:
	//extra
	//
	// Add the attibutes for suitable
	GETSET(int, m_update_peerconf, _update_peerconf)
	// update_peerconf
	//   which mode to write local peerconf 

	GETSET(string, m_log_mode, _log_mode)
	// log_mode, set output mode
	// defoult : 
	// options:
	// 	"" : stdout
	// 	nobuf : stdout without buffer
	// 	file:filepath
	// 	...  todo:
	// 		none
	//		logcat:logcattag
	//		udp:udp_ip_and_port

	GETSET(string, m_preload_playlist, _preload_playlist)
	// set preload playlist file path
	// 	to load the urls in playlist file 
	// 	for p2p caching
	// 	default: playlist.inf on conf path
	
	GETSET(int, m_daemon_mode, _daemon_mode)
	// set vod peer is or is not on background
	// 	default: 0, not on deamon
	// 	when it is running on daemon mode, 
	// 	log_mode should not be stdout and flush stdout
	
	GETSET(string, m_peerconfig_name, _peerconfig_name)
	// peerconfig_name, 
	// 	default is "peerconfig.ini"
	
	GETSET(unsigned int, m_playlist_timeshift, _playlist_timeshift )
	// playlist_timeshift
	// 	load the segments when source's playlist has expired
	//
	
	GETSET(unsigned int, m_playlist_delaynumber, _playlist_delaynumber )
	// playlist_delaynumber
	// 	the last number of the segments in playlist which will not download
	//  the different delay make the different vodpeer has different content
	//  so it is better for p2p sharing.
	

	GETSET(int, m_playlist_size, _playlist_size )
	// playlist_size
	// 	output's playlist's size
	//
	
	GETSET(unsigned int, m_export_list, _export_list )
	// export_list
	// 	0 or 1
	// 	export or non-export the m3u8 list

	GETSET(string, m_export_filename, _export_filename )
	// export_filename
	//   change the filename of the export m3u8 file

	GETSET(string, m_export_prefix, _export_prefix )
	// export_prefix
	//   add the prefix on each segment in the export m3u8 list

	GETSET(string, m_export_logfile, _export_logfile )
	// export_logfile
	//   log the export status when update the export m3u8 list

	GETSET(unsigned int, m_export_timeshift, _export_timeshift )
	// export_timeshift
	//   another timeshift is pre-process the m3u8 list, so download is start 
	//   for more early segments.
	//   but i find it is not a good idea for export m3u8 file, i don't want
	//   it start for early segment, just i want it can give more early sengment
	//   when export!

	GETSET(string, m_hosts_file, _hosts_file )
	// hosts_file
	//  a hosts file which do local dns lookup !

	GETSET(int, m_localonly, _localonly )
	// localonly
	//   security setting, limit playlist services only for localhost access

	GETSET(int, m_strict_mode, _strict_mode )
	// strict_mode
	// don't read/write local peerconfig

	GETSET(int, m_auth_response, _auth_response )
	// auth_response , -1 means auth failed!
	
	GETSET(string, m_auth_message, _auth_message )
	// auth_message
	//   if auth failed, it is the auth fail message
	//   if auth success, it is the auth package infomation


	GETSET(string, m_m3u8_tailer, _m3u8_tailer )
	// m3u8 tailer, for remove the session based m3u8 playlist
	GETSET(string, m_tsseg_tailer  , _tsseg_tailer )
	// ts segment tailer, for remove the session based ts segementer url

	GETSET(string, m_ostype, _ostype )
	// ostype

/*
	GETSET(string, m_versionname, _versionname)
	// versionname
	// the string to tell which detail version to make client decides if it need update
	// and other action

	GETSET(string, m_update_url, _update_url )
	// update_url
	// the url which include the new version of vodpeer
*/

public: 
        map<string,string> m_plurls;
        // just for save orginial playlist url, 
        // so the different url with session random url can be p2plized!
        
};

typedef Singleton<Setting> SettingSngl;
