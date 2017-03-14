#include "ProcessInfo.h"
#include "Util.h"
#ifdef _WIN32
#else

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <wait.h>

//
//将多个连续的空格保留成一个空格
string& trim_space(string& src)
{
	if(src.empty())
		return src;

	int n = 0;
	int pos = 0,pos2=0;
	int count = 0;

	while(1)
	{
		pos = src.find(' ',n);
		if(pos<0)
			break;
		pos2 = pos;
		for(int i=pos+1;i<(int)src.length();++i)
		{
			if(src.at(i)!=' ')
				break;
			else
				pos2 = i;
		}
		count = pos2-pos;
		if(count>0)
			src.erase(pos,count);
		n = pos+1;
	}

	//去掉首尾空格
	if(!src.empty())
	{
		if(src.at(0)==' ')
			src.erase(0,1);
	}
	if(!src.empty())
	{
		if(src.at(src.length()-1)==' ')
			src.erase(src.length()-1,1);
	}
	return src;
}
/////////////////////////////////////////
ProcessInfo::ProcessInfo(void)
{
}

ProcessInfo::~ProcessInfo(void)
{
	//m_psmap.clear();
	m_ls_idname.clear();
}
int ProcessInfo::do_stop(const char* myname)
{
	//
	printf("#check stop !!!\n");
	if(0 != update_curr_user_process())
		return -1;
	list<string> ls;
	list<string>::iterator it;
	Util::get_stringlist_from_file("keeplive.conf",ls);
	//路径|参数1|参数2|...
	string str,name,path,cwd;
	//int n=0;

	//char **argv;
	int pos;
	for(it=ls.begin();it!=ls.end();++it)
	{
		str=*it;
		if(str.empty())
			continue;
		//n = Util::get_string_index_count(str,"|");
		path=Util::get_string_index(str,0,"|");
		pos=path.rfind('/');
		if(pos>=0)
		{
			name=path.substr(pos+1);
			cwd=path.substr(0,pos);
		}
		else
		{
			name=path;
			cwd="";
		}
		stop_process_by_name(name.c_str());
	}
	
	stop_process_by_name(myname);
	return 0;
}
int ProcessInfo::stop_process_by_name(const char* name)
{
	if(NULL==name)
		return 0;
	char cmd[512];
	pid_t myid = getpid();
	for(list<proc_idname_t>::iterator it=m_ls_idname.begin();it!=m_ls_idname.end();)
	{
		if((*it).name == name && (*it).id!=myid)
		{
			sprintf(cmd,"kill -9 %d",(*it).id);
			system(cmd);
			if(errno!=0)
			{
				perror("system() failed:");
				Util::write_log("system(\"kill -9\") failed!","keeplive.log");
				printf("%s -- %s -- faild*** \n",cmd,(*it).name.c_str());
			}
			else
			{
				printf("%s -- %s -- ok \n",cmd,(*it).name.c_str());
			}

			m_ls_idname.erase(it++);
		}
		else
		{
			if((*it).name == name && (*it).id==myid)
			{
				printf(" do not close myself id:%d \n",(int)myid);
			}
			++it;
		}
	}
	return 0;
}
int ProcessInfo::do_keeplive()
{
	//
	printf("#check keeplive !!!\n");
	if(0 != update_curr_user_process())
		return -1;

	list<string> ls;
	list<string>::iterator it;
	Util::get_stringlist_from_file("keeplive.conf",ls);
	//路径|参数1|参数2|...
	string str,name,path,cwd;
	//int n=0;

	//char **argv;
	int pos;
	for(it=ls.begin();it!=ls.end();++it)
	{
		str=*it;
		if(str.empty())
			continue;
		//n = Util::get_string_index_count(str,"|");
		path=Util::get_string_index(str,0,"|");
		pos=path.rfind('/');
		if(pos>=0)
		{
			name=path.substr(pos+1);
			cwd=path.substr(0,pos);
		}
		else
		{
			name=path;
			cwd="";
		}
		if(!find_process_id(name.c_str()))
		{
			//argv=(char **)new char[n*sizeof(char*)];
			//for(i=0;i<(n-1);++i)
			//	argv[i] = new char[256];
			//for(i=1;i<n;++i)
			//	strcpy(argv[i-1],Util::get_string_index(str,i,"|").c_str());
			//argv[n-1]=NULL;

			//exec(path.c_str(),cwd.c_str(),argv);

			//for(i=0;i<(n-1);++i)
			//	delete[] argv[i];
			//delete[] argv;
			proc_info_t *inf=new proc_info_t();
			inf->cwd = cwd;
			inf->path = path;
			inf->info = str;
			pthread_t id;
			if(0==pthread_create(&id,NULL,open_process_thread,(void*)inf))
			{
				pthread_detach(id); 
			}
		}
	}
	
	return 0;
}
int ProcessInfo::update_curr_user_process()
{
	errno=0;
	system("ps -A >ps.txt.tmp");
	if(errno!=0)
	{
		perror("system() failed:");
		Util::write_log("system(\"ps -A >ps.txt.tmp\") failed!","keeplive.log");
		return -1;
	}
	//m_psmap.clear();
	m_ls_idname.clear();
	list<string> ls;
	list<string>::iterator it;
	Util::get_stringlist_from_file("ps.txt.tmp",ls);

	if(ls.size()<1)
		return -1;

	string str,name;
	str = ls.front();
	ls.pop_front();

	trim_space(str);
	int n=-1;
	int count = Util::get_string_index_count(str," ");
	for(int i=0;i<count;++i)
	{
		if(Util::get_string_index(str,i," ")=="CMD")
		{
			n = i;
			break;
		}
	}

	if(-1==n)
	{
		//Util::write_log("\"ps >ps.txt.tmp\" 命令输出格式不对不含[CMD] ","keeplive.log");
		return -1;
	}
	
	//注意：keeplive程序算法不支持带有空格的文件名
	proc_idname_t in;
	for(it=ls.begin();it!=ls.end();++it)
	{
		str=*it;
		trim_space(str);
		if(str.empty())
			continue;
		name = Util::get_string_index(str,n," ");
		in.id = atoi(Util::get_string_index(str,0," ").c_str());
		in.name = name;
		//map<exename,pid>
		//m_psmap[name]=atoi(Util::get_string_index(str,0," ").c_str());
		m_ls_idname.push_back(in);
	}
	ls.clear();

	//remove("ps.txt.tmp");
	return 0;
}

int ProcessInfo::find_process_id(const char* name) const
{
	if(!name)
		return 0;

	//由于存在进程队列保存的文件名不支持空格，如果有空格，被截断取前面部分，所以我们的名字如果有空格也比较空格前部，惹相等也算存在
	string str = name;
	trim_space(str);
	//str = Util::get_string_index(str,0," ");
	//map<string,int>::const_iterator it=m_psmap.find(str);
	//if(it!=m_psmap.end())
	//	return it->second;
	for(list<proc_idname_t>::const_iterator it=m_ls_idname.begin();it!=m_ls_idname.end();++it)
	{
		if((*it).name == str)
			return (*it).id;
	}
	return 0;
}
void *ProcessInfo::open_process_thread(void *arg)
{
	proc_info_t *inf=(proc_info_t*)arg;

	int n = Util::get_string_index_count(inf->info,"|");
	char **argv;
	int i;
	argv=(char **)new char[n*sizeof(char*)];
	for(i=0;i<(n-1);++i)
		argv[i] = new char[256];
	for(i=1;i<n;++i)
		strcpy(argv[i-1],Util::get_string_index(inf->info,i,"|").c_str());
	argv[n-1]=NULL;

	exec(inf->path.c_str(),inf->cwd.c_str(),argv);

	for(i=0;i<(n-1);++i)
		delete[] argv[i];
	delete[] argv;

	delete inf;
	
	pthread_exit(NULL);
	return (void*)0;
}

int ProcessInfo::exec(const char *path,const char *cwd,char *const argv[])
{
	int pid;
	pid=fork();
	switch(pid)
	{
	case -1:
		{
			perror("fork faild");
			Util::write_log("fork() failed","keeplive.log");
			return -1;
		}
		break;
	case 0:
		{
			//再次创建子进程,让中间进程结束,第三层进程(应用进程)的父进程将被init接管,此时的进程即使远程操作断开,也不会被kill掉了
			pid = fork();
			if(pid < 0 || pid>0)
			{
				//sleep(2);
				printf("# -- 中间进程退出.. \n");
				exit(0); 
			}

			//由于改变进程的工作目录,如果原来的path是相对目录,先变成绝对目录
			char old_cwd[1024];
			char buf[1024];
			getcwd(old_cwd,1024);
			if(path[0] != '/')
			{
				if(old_cwd[strlen(old_cwd)-1] != '/')
					sprintf(buf,"%s/%s",old_cwd,path);
				else
					sprintf(buf,"%s%s",old_cwd,path);
			}
			else
				sprintf(buf,"%s",path);

			if(cwd&&strlen(cwd))
				chdir(cwd);

			char log[2048];
			sprintf(log,"keeplive start [path=%s,execv_path=%s,cwd=%s]",path,buf,cwd);
			Util::write_log(log,"keeplive.log");

			execv(buf,argv);
			
			sprintf(log,"***start [path=%s] failed!",path);
			Util::write_log(log,"keeplive.log");
			exit(0);
		}
		break;
	default:
		{
		}
		break;
	}
	printf("#-- keeplive waitpid-> %s \n",path);
	waitpid(pid,NULL,0);
	printf("#-- keeplive = %s finished \n",path);
	return 0;
}
#endif
