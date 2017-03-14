#pragma once
#include "ntypes.h"
#include "Protocol.h"

const int DBC_BASE                                 = 100;
const int DBC_USER_LOGIN                           = DBC_BASE + 1;
const int DBC_USER_LOGIN_OUT                       = DBC_BASE + 2;
const int DBC_ERROR_INFO                           = DBC_BASE + 3;
const int DBC_START_FILE                           = DBC_BASE + 4;
const int DBC_SPEED_INFO                           = DBC_BASE + 5;
const int DBC_DOWNLOADFILE_INFO                    = DBC_BASE + 6;
const int DBC_DOWNLOAD_INFO                        = DBC_BASE + 7;
const int DBC_REALTIME_INFO                        = DBC_BASE + 8;


//*********************************
//
//用户登陆时写数据库记录
//
//*********************************
typedef struct tag_DBUserLogin{
	puid_t uid;
	int peer_type;
	unsigned int ip;        //本机序IP
	unsigned int prog_ver;  //软件版本
	unsigned int os_ver;    //系统版本
}DBUserLogin;


//*********************************
//
//用户退出时写数据库记录
//
//*********************************
typedef struct tag_DBUserLoginOut{
	puid_t uid;
	int dump_num;      //异常退出次多（其实最多为1）
	int file_num;     //请求文件数
	unsigned int download_flow;  //MB
	unsigned int upload_flow;    //MB
}DBUserLoginOut;

//*********************************
//
//错误统计量,每10分钟一条记录累计这10分钟的数据量.
//
//*********************************
typedef struct tag_DBErrorInfo{
	//unsigned int server_id;       //tracker_id;
	unsigned int miss_file_num;   //搜索不到文件源的文件数
	unsigned int only_server_num; //搜索源只有中心服务器源的数量,此数据说明发布节目并没有发到加速服务器上
	unsigned int program_except_num; //程序异常数据
	unsigned int hash_check_fail_num; //下载错误量
}DBErrorInfo;

//*********************************
//
//用户每看一个节目发一次记录.
//注意：32位系统
//*********************************
typedef struct tag_DBStartFile{
	puid_t uid;
	unsigned int user_type;
	unsigned int ip;
	unsigned char hash_type;
	char str_hash[48];
#ifdef SM_VOD
	int filetype;
#endif
}DBStartFile;


//*********************************
//
//看节目速度体验记录,用户每看完一个节目发一次记录.
//注意：32位系统
//*********************************
typedef struct tag_DBSpeedInfo{
	puid_t uid;
	unsigned int ip;
	unsigned char hash_type;
	char str_hash[48];
	unsigned long long size;
	int speed;              //KB/s
	int wait_num;           //缓冲次
	int download_duration;  //下载时长
}DBSpeedInfo;

//*********************************
//
//每个节目下载信息,用户每看20分钟报一次，看完后报一次。
//注意：32位系统
//*********************************
typedef struct tag_DBDownloadFileInfo{
	puid_t uid;
	unsigned int ip;
	PTL_P2T_ReportDownloadFileInfo df;
#ifdef SM_MODIFY
	int nattype;
#endif

}DBDownloadFileInfo;


//*********************************
//
//文件请求量/下载量/连接量统计数据 每10分钟一条记录累计这10分钟的数据量.
//
//*********************************
typedef struct tag_DBDownloadInfo{
	int request_files;       
	unsigned int tcp_download_flow;    //MB
	unsigned int udp_download_flow;
	unsigned int http_download_flow;
	unsigned int edge_download_flow;
	unsigned int client_download_flow;
	unsigned int center_download_flow;
	unsigned int super_download_flow;
	unsigned int tcp_connect_success;  //tcp 直连
	unsigned int tcp_connect_fail;
	unsigned int tcp_accept_success;   //tcp 反连
	unsigned int tcp_accept_fail;
	unsigned int udp_connect_success;  //udp 直连
	unsigned int udp_connect_fail;
	unsigned int udp_receive_success;  //udp 反连
	unsigned int udp_receive_fail;
	unsigned int udp_nat_success;      //udp nat
	unsigned int udp_nat_fail;
}DBDownloadInfo;

//*********************************
//
//实时数据 每10分钟一条记录.
//
//*********************************
typedef struct tag_DBRealTimeInfo{
	unsigned int online_users;     //总在线数
	unsigned int edge_servers;     //边缘服务器数
	unsigned int center_servers;   //中心服务器数
	unsigned int super_servers;    //超级缓冲服务器，专门为开头和拖动时短时间服务
	unsigned int online_progs;     //在线节目数
	unsigned int online_sources;    //在线节目源次
	int nat_0;
	int nat_1;
	int nat_2;
	int nat_3;
	int nat_4;
	int nat_5;
}DBRealTimeInfo;

