#pragma once
#include "User.h"
#include "PacketList.h"
#include "TrackerMessage.h"

class UserManager
{
public:
	UserManager(void);
	~UserManager(void);
public:
	int init();
	void fini();
	void handle_root();//处理收发线程
	bool put_packet(const Packet& pack)
	{
		return m_packs.put_packet(pack);//如果已经fini，会失败。因此在此不必要判断是否已经fini
	}

	int get_nat_type(int arr[]);
	int get_user_num() { return m_user_num;}
	int get_max_user_num_and_reset(){
		int n=m_max_user_num;
		m_max_user_num = m_user_num;
		return n;
	}
	int get_max_pack_num_and_reset() { 
		int n = m_max_pack_num;
		m_max_pack_num = 0;
		return n; 
	}
	int get_server_list(UserInfo *arr[],int max_count);
private:
	void handle_command();
	void handle_timer();

	void on_ptl_request_login                 (const PTL_Head& head,PTL_P2T_RequestLogin& req,Packet& inf);
	void on_ptl_report_nat                    (const PTL_Head& head,PTL_P2T_ReportNat& req,Packet& inf);
	void on_ptl_report_downloadlist_maxnum    (const PTL_Head& head,PTL_P2T_ReportDownloadListMaxnum& req,Packet& inf);
	void on_ptl_report_sharefile              (const PTL_Head& head,PTL_P2T_ReportShareFile& req,Packet& inf);
	void on_ptl_report_removefile             (const PTL_Head& head,PTL_P2T_ReportRemoveFile& req,Packet& inf);
	void on_ptl_report_startdownloadfile      (const PTL_Head& head,PTL_P2T_ReportStartDownloadFile& req,Packet& inf);
	void on_ptl_report_startdownloadlist      (const PTL_Head& head,PTL_P2T_ReportStartDownloadList& req,Packet& inf);
	void on_ptl_report_stopdownloadfile       (const PTL_Head& head,PTL_P2T_ReportStopDownloadFile& req,Packet& inf);
	void on_ptl_request_filesource            (const PTL_Head& head,PTL_P2T_RequestFileSource& req,Packet& inf);
	void on_ptl_report_downloadwrong          (const PTL_Head& head,PTL_P2T_ReportDownloadWrong& req,Packet& inf);
	void on_ptl_report_downloadfilespeed      (const PTL_Head& head,PTL_P2T_ReportDownloadFileSpeed& req,Packet& inf);
	void on_ptl_report_stat                   (const PTL_Head& head,PTL_P2T_ReportStat& req,Packet& inf);
	void on_ptl_report_error                  (const PTL_Head& head,PTL_P2T_ReportError& req,Packet& inf);
	void on_ptl_request_connturn              (const PTL_Head& head,PTL_P2T_RequestConnTurn& req,Packet& inf);
	void on_ptl_request_keeplive              (const PTL_Head& head,/*PTL_P2T_RequestKeeplive& req,*/Packet& inf);
	void on_ptl_request_serverlist            (const PTL_Head& head,PTL_P2T_RequestServerList& req,Packet& inf);
	void on_ptl_report_downloadfileinfo       (const PTL_Head& head,PTL_P2T_ReportDownloadFileInfo& req,Packet& inf);
	void logout  (int sid,bool tell=false);

public:
	int ptl_request_startdownloadlist(UserInfo* user,const hash_t& hash,const char* url);
	int ptl_request_stopdownloadlist(UserInfo* user,const hash_t& hash);
	int get_msg_server_info(MsgServerInfo& msg);
private:
	bool m_binit;
	UserInfo *m_userl;
	PacketList m_packs;
	unsigned int  m_userl_size, m_user_num;
	unsigned int m_max_user_num;
	int m_max_pack_num;
	int m_nat_type_num[7];

	time_t m_curr_sec;
	time_t m_last_clear_dead_sec;
	time_t m_stat_last_sec;
	time_t m_src_last_sec;

	//临时变量
	int             sid;
	hash_t          tmp_hash;
	MemBlock*       tmp_block;
	Packet          tmp_pack;
	UserInfo*       tmp_user[1200];
	PTLStream       tmp_ss;
	PTL_Head		cmd_head;

	PTL_P2T_RequestLogin              request_login;
	PTL_P2T_ReportNat                 report_nat;
	PTL_P2T_ReportDownloadListMaxnum	report_downloadlist_maxnum;
	PTL_P2T_ReportShareFile           report_sharefile;
	PTL_P2T_ReportRemoveFile          report_removefile;
	PTL_P2T_ReportStartDownloadFile   report_startdownloadfile;
	PTL_P2T_ReportStartDownloadList  report_startdownloadlist;
	PTL_P2T_ReportStopDownloadFile    report_stopdownloadfile;
	PTL_P2T_RequestFileSource         request_filesource;
	PTL_P2T_ReportDownloadWrong       report_downloadwrong;
	PTL_P2T_ReportDownloadFileSpeed   report_downloadfilespeed;
	PTL_P2T_ReportStat                report_stat;
	PTL_P2T_ReportError               report_error;
	PTL_P2T_RequestConnTurn           request_connturn;
	//PTL_P2T_RequestKeeplive           request_keeplive;
	PTL_P2T_RequestServerList         request_serverlist;
	PTL_P2T_RequestStartDownloadList	request_startdownloadlist;
	PTL_P2T_RequestStopDownloadList		request_stopdownloadlist;

	PTL_P2T_ResponseLogin              response_login;
	PTL_P2T_ResponseFileSource         response_filesource;
	PTL_P2T_ResponseConnTurn           response_connturn;
	//PTL_P2T_ResponseKeeplive           response_keeplive;
	PTL_P2T_ResponseServerList         response_serverlist;
	PTL_P2T_ReportDownloadFileInfo     report_downloadfileinfo;
};
typedef Singleton<UserManager> UserManagerSngl;

