#pragma once
#include "commons.h"


/*
功能:
1. 每10秒加载一次 playlist.inf 中的 m3u8任务 (跟核心线程相同)
2. 每10秒执行一次文件共享，下载，删除的任务 （单独线程)
	sharefile.txt ---- 共享文件(执行完成后会把sharefile.txt文件删除)   [ path ]
	deletefile.txt ---- 删除共享的文件(执行完成后会把deletefile.txt文件删除)  [ sha1|flah ] --- flag=0(不删除源文件)/1（删除源文件)
	downloadfile.txt ---- 添加一个下载(执行完成后会把downloadfile.txt文件删除) [ sha1/url | path ] --- 可以是hash,也可以是url, PATH可以不指定
	downloadfile_test.txt ---- 以测试形式添加一个下载 （ 不会删除downloadfile_test.txt 文件 ）
*/
class LocalService : public Thread
	,public TimerHandler
{
public:
	LocalService(void);
	~LocalService(void);
	
public:
	int run();
	void end();
	virtual int work(int e);
	virtual void on_timer(int e);
	int get_pll(list<string>& ls);
private:
	void check_share_playlist();
	void check_open_playlist();

	void check();
	int share_files(const string& path);
	void delete_files(const string& path);
	void download_files(const string& path);

	int share_files_list(list<string>& ls);
private:
	bool m_brun;
	list<string> m_pll,m_pre_pll; //m_pll当前正在运行的所有playlist，m_pre_pll是load进来还未处理完的
	int m_pll_mtime;  //最后一次装载的连接
};
typedef Singleton<LocalService> LocalServiceSngl;
