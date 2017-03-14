#pragma once
#include "commons.h"
#include "DBInfo.h"
#include "PacketList.h"
#include "DB.h"


class DBManager : public Thread
{
public:
	DBManager(void);
	~DBManager(void);

	int run();
	int end();
	virtual int work(int e);

	bool put_packet(const Packet& pack)
	{
		return m_packl.put_packet(pack);//如果已经fini，会失败。因此在此不必要判断是否已经fini
	}
private:
	int root_cmd();//处理命令包线程
	void handle_command();
	void free_data(Packet &pki);
	void handle_timer();

	void on_userlogin(DBUserLogin *inf){}
	void on_userloginout(DBUserLoginOut *inf){}
	void on_errorinfo(DBErrorInfo *inf){}
	void on_startfile(DBStartFile *inf){}
	void on_speedinfo(DBSpeedInfo *inf){}
	void on_downloadinfo(DBDownloadInfo *inf){}
	void on_realtimeinfo(DBRealTimeInfo *inf){}

	int on_trackerconf(){return 0;}
private:
	bool m_bopen;
	PacketList m_packl;
	time_t m_curr_sec;
	time_t m_last_ping_sec;
	bool m_bwrite_trackerconf;

	char buf[4096];
	//SqlQuery query;
};
typedef Singleton<DBManager> DBManagerSngl;
