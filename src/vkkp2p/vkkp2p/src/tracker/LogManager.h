#pragma once
#include "commons.h"
#include "DBInfo.h"

class LogManager
{
	friend class Singleton<LogManager>;
private:
	LogManager(void);
	~LogManager(void);
public:
	static string format_curr_day();
	static string format_curr_time();
public:
	int init();
	void fini();
	void update_fd();

	void on_userlogin(DBUserLogin *inf);
	void on_userloginout(DBUserLoginOut *inf);
	void on_startfile(DBStartFile *inf);
	void on_downloadfileinfo(DBDownloadFileInfo *inf);
	void on_downloadinfo(DBDownloadInfo *inf);
	void on_realtimeinfo(DBRealTimeInfo *inf);
#ifdef SM_MODIFY	
private:
	void update_userloginfd();
	void update_userloginoutfd();
	void update_startfilefd();
	void update_downloadfileinfofd();
	void update_downloadinfofd();
	void update_realtimeinfofd();
	void update_fd(FILE* &fd, string &time, const char* cprename, const char *cbuf );
#endif

private:
	bool m_binit;
	string m_curr_date;
	FILE *fd_login,*fd_loginout;
	FILE *fd_start_file,*fd_start_playlist;
	FILE *fd_speed_file,*fd_speed_playlist;
	FILE *fd_downinfo,*fd_realtimeinfo;
	char _tmpbuf[4096];
	hash_t _tmphash;
	char _tmpstrhash[HASHLEN];
};

typedef Singleton<LogManager> LogManagerSngl;

