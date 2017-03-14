#pragma once
#include "commons.h"

enum {DS_STOP=0,DS_START=1,DS_QUEUE=2};
typedef struct tagDownManualInfo
{
	string tth;
	hash_t hash;
	string url;
	string name;  //��Ϊ�����������ʱ������·�����ڼ�������ʱ��ѡ��·����������name���ݴ�·��
	string path;
	uint64 size;
	int state; 
	int last_state; //��ͣǰ��״̬
	int ftype; //�������� FTYPE_DOWNLOAD��FTYPE_VOD
	unsigned int createtime;// = time(NULL)
	int progress;//��ɵ�ǧ�ֱ�
	int speed;//�ٶ� B/S
	int downtick; //�����ۼ�ʱ��
	int zerospeedtick; //0�ٶ�ʱ���ۼ�
	tagDownManualInfo()
		:size(0)
		,state(0)
		,last_state(0)
		,ftype(0)
		,createtime(0)
		,progress(0)
		,speed(0)
		,downtick(0)
		,zerospeedtick(0)
	{}

}DownManualInfo;


class DownManualManager : public Thread
{
	friend class Singleton<DownManualManager>;
private:
	DownManualManager(void);
	~DownManualManager(void);
public:
	typedef CriticalSection Mutex;
	typedef list<DownManualInfo*>  FileList;
	typedef FileList::iterator FileIter;

	int run();
	void end();
	virtual int work(int e);
	int add_down(const hash_t& hash,const string& filename,const string& path,const string& url,int state,int ftype);
	int del_down(const hash_t& hash,bool isDelPhy);
	int set_state(const hash_t& hash,int state);

	int pause();
	int resume();

	unsigned int down_file_get_count();
	int down_file_get_info(unsigned int index,unsigned int n,FileList& ls);
	int down_file_set_status(unsigned int index,int state); //DS_STOP=0,DS_START=1,DS_QUEUE=2
	int down_file_set_priority(unsigned int oldIndex,unsigned int newIndex); //������ԭ�����ŵ�������λ��
	int down_file_stop_all();
	int down_file_queue_all_from_stop();
	int down_file_delete(unsigned int index);

	unsigned int downfini_file_get_count();
	int downfini_file_get_info(unsigned int index,unsigned int n,FileList& ls);
	int downfini_file_delete(unsigned int index);
private:
	void InitPath();
	bool StartDown(DownManualInfo* inf);
	bool StopDown(DownManualInfo* inf,int state);
	bool ExistDown(const hash_t& hash);
	bool DownChangState(DownManualInfo* inf,int state);
	void DownTryStart(hash_t* exclude_hash=0);
	void DownTryQueue(hash_t* exclude_hash=0);
	
	void Load();
	void LoadList(FileList& fls,const string& path);
	void Save();
	void SaveList(FileList& fls,const string& path);
	void clear();

	void DownUpdate();
	void DownFinished(const hash_t& hash);
	void DownInit();
	void DownFini();
private:
	Mutex m_mt;
	bool m_bInit;
	bool m_bdowninit;
	string m_path;
	bool m_isPausing;
	int m_activeNum;
	int m_maxActiveNum;
	FileList m_lsDown;
	FileList m_lsFinished;
};
typedef Singleton<DownManualManager> DownManualManagerSngl;

