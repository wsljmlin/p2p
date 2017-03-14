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
//�û���½ʱд���ݿ��¼
//
//*********************************
typedef struct tag_DBUserLogin{
	puid_t uid;
	int peer_type;
	unsigned int ip;        //������IP
	unsigned int prog_ver;  //����汾
	unsigned int os_ver;    //ϵͳ�汾
}DBUserLogin;


//*********************************
//
//�û��˳�ʱд���ݿ��¼
//
//*********************************
typedef struct tag_DBUserLoginOut{
	puid_t uid;
	int dump_num;      //�쳣�˳��ζࣨ��ʵ���Ϊ1��
	int file_num;     //�����ļ���
	unsigned int download_flow;  //MB
	unsigned int upload_flow;    //MB
}DBUserLoginOut;

//*********************************
//
//����ͳ����,ÿ10����һ����¼�ۼ���10���ӵ�������.
//
//*********************************
typedef struct tag_DBErrorInfo{
	//unsigned int server_id;       //tracker_id;
	unsigned int miss_file_num;   //���������ļ�Դ���ļ���
	unsigned int only_server_num; //����Դֻ�����ķ�����Դ������,������˵��������Ŀ��û�з������ٷ�������
	unsigned int program_except_num; //�����쳣����
	unsigned int hash_check_fail_num; //���ش�����
}DBErrorInfo;

//*********************************
//
//�û�ÿ��һ����Ŀ��һ�μ�¼.
//ע�⣺32λϵͳ
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
//����Ŀ�ٶ������¼,�û�ÿ����һ����Ŀ��һ�μ�¼.
//ע�⣺32λϵͳ
//*********************************
typedef struct tag_DBSpeedInfo{
	puid_t uid;
	unsigned int ip;
	unsigned char hash_type;
	char str_hash[48];
	unsigned long long size;
	int speed;              //KB/s
	int wait_num;           //�����
	int download_duration;  //����ʱ��
}DBSpeedInfo;

//*********************************
//
//ÿ����Ŀ������Ϣ,�û�ÿ��20���ӱ�һ�Σ������һ�Ρ�
//ע�⣺32λϵͳ
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
//�ļ�������/������/������ͳ������ ÿ10����һ����¼�ۼ���10���ӵ�������.
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
	unsigned int tcp_connect_success;  //tcp ֱ��
	unsigned int tcp_connect_fail;
	unsigned int tcp_accept_success;   //tcp ����
	unsigned int tcp_accept_fail;
	unsigned int udp_connect_success;  //udp ֱ��
	unsigned int udp_connect_fail;
	unsigned int udp_receive_success;  //udp ����
	unsigned int udp_receive_fail;
	unsigned int udp_nat_success;      //udp nat
	unsigned int udp_nat_fail;
}DBDownloadInfo;

//*********************************
//
//ʵʱ���� ÿ10����һ����¼.
//
//*********************************
typedef struct tag_DBRealTimeInfo{
	unsigned int online_users;     //��������
	unsigned int edge_servers;     //��Ե��������
	unsigned int center_servers;   //���ķ�������
	unsigned int super_servers;    //���������������ר��Ϊ��ͷ���϶�ʱ��ʱ�����
	unsigned int online_progs;     //���߽�Ŀ��
	unsigned int online_sources;    //���߽�ĿԴ��
	int nat_0;
	int nat_1;
	int nat_2;
	int nat_3;
	int nat_4;
	int nat_5;
}DBRealTimeInfo;

