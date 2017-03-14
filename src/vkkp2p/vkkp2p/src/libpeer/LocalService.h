#pragma once
#include "commons.h"


/*
����:
1. ÿ10�����һ�� playlist.inf �е� m3u8���� (�������߳���ͬ)
2. ÿ10��ִ��һ���ļ��������أ�ɾ�������� �������߳�)
	sharefile.txt ---- �����ļ�(ִ����ɺ���sharefile.txt�ļ�ɾ��)   [ path ]
	deletefile.txt ---- ɾ��������ļ�(ִ����ɺ���deletefile.txt�ļ�ɾ��)  [ sha1|flah ] --- flag=0(��ɾ��Դ�ļ�)/1��ɾ��Դ�ļ�)
	downloadfile.txt ---- ���һ������(ִ����ɺ���downloadfile.txt�ļ�ɾ��) [ sha1/url | path ] --- ������hash,Ҳ������url, PATH���Բ�ָ��
	downloadfile_test.txt ---- �Բ�����ʽ���һ������ �� ����ɾ��downloadfile_test.txt �ļ� ��
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
	list<string> m_pll,m_pre_pll; //m_pll��ǰ�������е�����playlist��m_pre_pll��load������δ�������
	int m_pll_mtime;  //���һ��װ�ص�����
};
typedef Singleton<LocalService> LocalServiceSngl;
