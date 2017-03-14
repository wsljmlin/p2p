#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <map>
#include <list>
using namespace std;

class ProcessInfo
{
public:
	ProcessInfo(void);
	~ProcessInfo(void);

	typedef struct proc_info{
		string cwd;
		string path;
		string info;
	}proc_info_t;
	typedef struct tag_proc_idname{
		string name;
		int id;
	}proc_idname_t;
	
public:
	int do_stop(const char* name);
	int do_keeplive();
	int stop_process_by_name(const char* myname);
	int update_curr_user_process();
	int find_process_id(const char* name) const;

	static void *open_process_thread(void *arg);
	static int exec(const char *path,const char *cwd,char *const argv[]);

private:
	//map<string,int> m_psmap;
	list<proc_idname_t> m_ls_idname;
};
