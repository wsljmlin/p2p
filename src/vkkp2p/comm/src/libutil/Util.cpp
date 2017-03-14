#include "Util.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include "RFDelete.h"

#ifdef _WIN32
	#include <winsock2.h>
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
	#include <windows.h>
	#include <crtdbg.h>
	#include <Iphlpapi.h>
	//#pragma comment(lib,"Iphlpapi.lib")
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <net/if_arp.h>  
	#include <net/if.h>
	//#include <stropts.h>
	#include <sys/ioctl.h> 
	#include <linux/hdreg.h>
	#include <dirent.h>
	#include <stdarg.h>
#ifdef _OS
#include <sys/param.h>
#include <sys/mount.h>
#else
	#include <sys/vfs.h>
#endif
	#include <pthread.h>
	#include <sys/file.h>
#endif

#include "DIPCache.h"
int Util::socket_init()
{
#ifdef _WIN32
	WSADATA wsaData;
	if(0!=WSAStartup(0x202,&wsaData))
	{
		perror("WSAStartup false! : ");
		return -1;
	}
	return 0;
#endif
	return 0;
}

void Util::socket_fini()
{
#ifdef _WIN32
	WSACleanup();
#endif
}
void Util::debug_memleak()
{
#ifdef _WIN32
	//输出内存泄漏
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tmpFlag );
#endif
}

int Util::chdir(const char* path)
{
#ifdef _WIN32
	return TRUE==::SetCurrentDirectoryA(path);
#else
	return ::chdir(path);
#endif
}

int _GetModuleFileName(const char* lpszModuleName,char* lpszModulePath, int cbModule)
{
	int len = 0;
#ifdef _WIN32
	len = ::GetModuleFileNameA(::GetModuleHandleA(lpszModuleName), lpszModulePath, cbModule);
#else
#ifndef MAX_PATH
#define MAX_PATH 256
#endif //MAX_PATH

	//取得当前路径
	char szCurPath[MAX_PATH] = {0};
	getcwd(szCurPath, MAX_PATH);

	char szCmdPath[MAX_PATH] = {0};
	char szCmdLineFile[MAX_PATH] = {0};
	sprintf(szCmdLineFile, "/proc/%d/cmdline", getpid());
	FILE* f = fopen(szCmdLineFile, "r");
	if(f != 0)
	{
		len = fread(szCmdPath, 1, MAX_PATH,  f);
		fclose(f);

	}

	if(szCmdPath[0] != '/' && szCurPath[0] == '/')
	{
		char szTmp[MAX_PATH] = {0};
		if(szCmdPath[0] == '.' && szCmdPath[1] == '/')
		{
			//当前快捷路径 需要加上当前完全路径 wwjs 2006-02-06
			//通常直接命令行是这样的
			sprintf(szTmp, "%s/%s", szCurPath, &szCmdPath[2]);
		}
		else
		{
			//某些系统的Apache配置, cmcLine 中不包含路径，需要加上 wwjs 2006-01-23
			sprintf(szTmp, "%s/%s", szCurPath, szCmdPath);
		}

		strcpy(szCmdPath, szTmp);
	}


	switch(szCmdPath[0])
	{
	case '.':
		sprintf(lpszModulePath, "%s%s", szCurPath, &szCmdPath[1]);
		break;
	default:
		strcpy(lpszModulePath, szCmdPath);
		break;
	}


	len = strlen(lpszModulePath);
#endif

	//printf("cur::%s,cmd:%sm\npath=%s\n", szCurPath, szCmdPath, lpszModulePath);
	//去掉最后可能存在的 点 .
	if(lpszModulePath[0] == '/' && lpszModulePath[len] == '.')
	{
		lpszModulePath[len] = 0;
		len--;
	}
	return len;

}
string Util::get_module_path()
{
	char buf[256]={0,};
	_GetModuleFileName(NULL,buf,256);
	return buf;
}
string Util::get_module_dir()
{
	char buf[256]={0,};
	if(_GetModuleFileName(NULL,buf,256))
	{
		char *p = ::strrchr(buf,'/');
		if(!p)
			p = ::strrchr(buf,'\\');
		if(p)
			*(p+1)='\0';
	}
	//DEBUGMSG("module path = %s \n",buf);
	return buf;
}
string Util::get_module_name()
{
	char buf[256]={0,};
	if(_GetModuleFileName(NULL,buf,256))
	{
		char *p = ::strrchr(buf,'/');
		if(!p)
			p = ::strrchr(buf,'\\');
		if(p)
			return (p+1);
	}
	//DEBUGMSG("module path = %s \n",buf);
	return buf;
}
PSL_HANDLE Util::lockname_create(const char* name)
{
#ifdef _WIN32
	HANDLE hMutex = NULL;
	SECURITY_DESCRIPTOR sd; 
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION); 
	SetSecurityDescriptorDacl(&sd, TRUE, (PACL) NULL, FALSE); 

	SECURITY_ATTRIBUTES sa; 
	sa.nLength = sizeof(sa); 
	sa.lpSecurityDescriptor = &sd; 
	sa.bInheritHandle = TRUE; 
	hMutex = ::CreateMutexA(&sa,TRUE, name);  //创建互斥量
	int err = GetLastError();
	if(hMutex)
	{
		if(err == ERROR_ALREADY_EXISTS)   
		{   
			//如果已有互斥量存在则释放句柄并复位互斥量
			CloseHandle(hMutex);
			hMutex = NULL;
		}
	}
	return hMutex;
#else
	//这里假定open()不会返回0的句柄
	int fd = open(name,O_RDONLY,0666);
	if(-1==fd)
	{
		fd = open(name,O_CREAT|O_RDWR,0666);
		if(-1==fd)
		{
			perror("open():");
			return 0;
		}
	}
	int ret = flock(fd,LOCK_EX|LOCK_NB);
	if(-1==ret)
	{
		perror("flock():");
		close(fd);
		return 0;
	}
	return fd;
#endif
}
PSL_HANDLE Util::process_single_lockname_create()
{
	string markname;
#ifdef _WIN32
	markname = Util::get_module_name();
#else
	markname = Util::get_module_path();
#endif
	return Util::lockname_create(markname.c_str());
}
void Util::process_single_lockname_close(PSL_HANDLE h)
{
#ifdef _WIN32
	if(h)
		CloseHandle(h);
#else
	if(0!=h && -1!=h)
		close(h);
#endif
}
int Util::get_system_version()
{
	int ver=0;
#ifdef _WIN32
	OSVERSIONINFO *osvi = new OSVERSIONINFO();
	if(NULL == osvi)
		return 0;

	memset(osvi,0, sizeof(OSVERSIONINFO));
	osvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(osvi);
	switch(osvi->dwPlatformId)
	{
	case VER_PLATFORM_WIN32s:
		{
			ver = 1; //win 3.1
		}
		break;
	case VER_PLATFORM_WIN32_WINDOWS:
		{
			ver = 11;    //win 95
			if(osvi->dwMajorVersion == 4L && osvi->dwMinorVersion == 10L)
				ver = 12; //win 98
			else if(osvi->dwMajorVersion == 4L && osvi->dwMinorVersion == 90L)
				ver = 13; //win ME
		}
		break;
	case VER_PLATFORM_WIN32_NT:
		{
			ver = 21;
			if(osvi->dwMajorVersion <= 4L)
				ver = 21; //win NT4
			else if(osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 0L)
				ver = 22; //win 2000
			else if(osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 1L)
				ver = 23; //win XP
			else if( osvi->dwMajorVersion == 5L && osvi->dwMinorVersion == 2L )
                ver = 24; //Microsoft Windows Server 2003 family
			else
				ver = 25; //WindowsVista;
		}
		break;
	default:
		break;
	}

	delete osvi;
#else
	ver = 50;
#endif
	return ver;
}
#ifdef _WIN32
int Util::get_all_volumes(list<string>& ls)
{
	char *ptr;
	int len = 0;
	len = GetLogicalDriveStringsA(0,NULL);
	char *buf = new char[len+10];
	memset(buf,0,len+10);
	len = GetLogicalDriveStringsA(len+10,buf);
	ptr = buf;
	ls.clear();
	while(*ptr)
	{
		if(DRIVE_FIXED == GetDriveTypeA(ptr))
			ls.push_back(ptr);
		while(*ptr++);
	}

	delete[] buf;
	return 0;
}
#else
int Util::get_all_volumes(list<string>&/* ls*/)
{
	assert(0);
	return 0;
}
#endif
bool Util::get_volume_size(const string& volume,ULONGLONG &total,ULONGLONG &used,ULONGLONG &free)
{
#ifdef _WIN32
	if(volume.length()<2)
		return false;
	string Driver;
	if(volume.at(1)!=':')
	{
		char buf[MAX_PATH];
		GetModuleFileNameA(NULL,buf,MAX_PATH);
		buf[3]='\0';
		Driver = buf;
	}
	else
		Driver = volume.substr(0,3);
	if(!Driver.empty())
	{
		//如果选中文件夹,则计算盘空间可用大小
		//获得磁盘空间信息
		ULARGE_INTEGER FreeAv,TotalBytes,FreeBytes;
		if(GetDiskFreeSpaceExA(Driver.c_str(),&FreeAv,&TotalBytes,&FreeBytes))
		{
			//
			total = TotalBytes.QuadPart;
			free = FreeBytes.QuadPart;
			used = total - free;
			return true;
		}
	}
#else
	struct statfs buf;
	if(0==statfs(volume.c_str(), &buf))
	{
		total = (ULONGLONG)buf.f_blocks * buf.f_bsize;
		free = (ULONGLONG)buf.f_bavail * buf.f_bsize;
		used = total - free;
	}
	else
	{
		printf("#: ***statfs(%s,statfs) failed: \n",volume.c_str());
		perror("****** statfs() failed:");
		total = (ULONGLONG)1000000000 * 10;//10G
		used = (ULONGLONG)0;
		free = total-used;
	}
	return true;
#endif
	return false;
}

int Util::my_create_directory(const string& path)
{
	if(path.empty())
		return -1;
	string str = path;
	size_t pos = str.find("\\");
	while(pos != string::npos)
	{
		str.replace(pos,1,"/",1);
		pos = str.find("\\");
	}
	if(str.substr(str.length()-1) == "/")
		str = str.substr(0,str.length()-1);

	pos = str.rfind('/');
	string lstr;
	if(pos!=string::npos)
		lstr = str.substr(0,pos);
	else
		lstr = "";
	my_create_directory(lstr);
#ifdef _WIN32
	return CreateDirectoryA(str.c_str(),NULL)?0:-1;
#else
	return mkdir(str.c_str(),0777);
#endif
}

int Util::create_directory_by_filepath(const string& filepath)
{
	//含文件名的全路径
	int pos1,pos2;
	pos1 = (int)filepath.rfind('\\');
	pos2 = (int)filepath.rfind('/');
	if(pos1==-1 && pos2==-1)
		return -1;
	if(pos1>pos2)
		return my_create_directory(filepath.substr(0,pos1));
	else
		return my_create_directory(filepath.substr(0,pos2));
}

bool Util::file_exist(const string& path)
{
	
	FILE *fp = fopen(path.c_str(),"rb");
	if(fp)
	{
		fclose(fp);
		return true;
	}
	return false;
}
int Util::file_delete(const string& path)
{
	return rf_delete_(path.c_str());
}
int Util::file_rename(const string& from,const string& to)
{
	return rename(from.c_str(),to.c_str());
}

bool Util::get_stringlist_from_file(const string& path,list<string>& ls)
{
	char buf[2048];
	char *ptr = NULL;
	FILE *fp = fopen(path.c_str(),"rb");
	if(!fp)
		return false;
	ls.clear();
	while(!feof(fp))
	{
		memset(buf,0,2048);
		if(NULL==fgets(buf,2048,fp))
			continue;
		if((ptr=strstr(buf,"\r")))
			*ptr = '\0';
		if((ptr=strstr(buf,"\n")))
			*ptr = '\0';

		ls.push_back(buf);
	}
	fclose(fp);
	return true;
}
bool Util::put_stringlist_to_file(const string& path,list<string>& ls)
{
	string tmppath = (string&)path + ".tmp";
	FILE *fp = fopen(tmppath.c_str(),"wb+");
	if(!fp)
		return false;
	for(list<string>::iterator it=ls.begin();it!=ls.end(); ++it)
	{
		fwrite((*it).c_str(),(*it).length(),1,fp);
		fwrite("\r\n",2,1,fp);
	}
	fflush(fp);
	fclose(fp);
	Util::file_delete(path);
	Sleep(0);
	Util::file_rename(tmppath,path);
	return true;
}
int Util::get_file_modifytime(const char* path)
{
#ifdef _WIN32
	WIN32_FIND_DATAA ffd ;
	HANDLE hFind = FindFirstFileA(path,&ffd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		return ffd.ftLastWriteTime.dwLowDateTime;
	}
#else
	struct stat statbuf;
	if(0==lstat(path,&statbuf))
	{
		return statbuf.st_mtime;
	}
#endif
	return 0;
}
int Util::get_filetimes(const char* path,time_t& ctime,time_t& mtime,time_t& atime)
{
#ifdef _WIN32
	//32位windows: 8=sizeof(time_t);
	//windows下的ctime为文件创建时间
#define FILETIME_TO_TIMET(ft) (((((ULONGLONG)ft.dwHighDateTime) << 32) + ft.dwLowDateTime - 116444736000000000)/10000000)
	WIN32_FIND_DATAA ffd ;
	HANDLE hFind = FindFirstFileA(path,&ffd);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		ctime = (time_t)FILETIME_TO_TIMET(ffd.ftCreationTime);
		mtime = (time_t)FILETIME_TO_TIMET(ffd.ftLastWriteTime);
		atime = (time_t)FILETIME_TO_TIMET(ffd.ftLastAccessTime);
		return 0;
	}
#else
	//32位linux: 4=sizeof(time_t);
	//linux下的ctime为inode节点(状态)的最后修改时间
	struct stat statbuf;
	if(0==lstat(path,&statbuf))
	{
		//注意:下面将st_mtime给ctime当创建时间
		//ctime = statbuf.st_ctime;
		ctime = statbuf.st_mtime;
		mtime = statbuf.st_mtime;
		atime = statbuf.st_atime;
		return 0;
	}
#endif
	////time to string (ctime: 2010-7-30 15:39:39)
	//tm* t = localtime(&ctime);
	//sprintf(buf,"ctime: %d-%d-%d %d:%d:%d\r\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
	//printf("-%s",::ctime(&ctime));
	//printf("-%s",asctime(t));
	return -1;
}

//如果指定扩展名,则只取指定扩展名,如果继承,则递归子目录搜索.返回所取文件个数.
long Util::get_folder_files(const string& dir,
						list<string>& ls_path,
						list<int>& ls_ino,
						const string& suffix/* = ""*/,
						int enable_samefile/* = 0*/,
						bool inherit/* = true*/)
{
#ifdef _WIN32
	if(dir.empty())
		return 0;

	string str;
	list<string> ls_suffix;
	list<string>::iterator it;
	int n = get_string_index_count(suffix,"|");
	for(int i=0;i<n;++i)
	{
		str = get_string_index(suffix,i,"|");
		string_trim(str);
		if(!str.empty())
			ls_suffix.push_back(str);
	}

	bool bfind = false;
	long count = 0;
	string strdir = dir;
	string path,name;
	size_t iExLen = suffix.length();
	if(strdir.at(strdir.length()-1) != '\\' && strdir.at(strdir.length()-1) != '/')
		strdir += "\\";

	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA((strdir+"*.*").c_str(),&fd);
	if(hFind!=INVALID_HANDLE_VALUE)
	{
		do{
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(inherit && strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
				{
					//count += get_dirfiles(arrName,arrPath,strdir + fd.cFileName,strEx,bInherit);
					count += get_folder_files(strdir + fd.cFileName,ls_path,ls_ino,suffix,enable_samefile,inherit);
				}
			}
			else
			{
				if(strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
				{
					path = strdir + fd.cFileName;
					name = fd.cFileName;

					bfind = false;
					if(ls_suffix.empty())
					{
						bfind = true;
					}
					else
					{
						int pos1,pos2;
						pos1 = (int)name.rfind('.');
						pos2 = (int)name.rfind('\\');
						if(pos2>=pos1)
							str = "";
						else
							str = name.substr(pos1+1);
						if(!str.empty())
						{
							for(it=ls_suffix.begin();it!=ls_suffix.end();++it)
							{
								//printf("----------scan suffix:::: %s --- %s \n",str.c_str(),(*it).c_str());
								if(0==stricmp(str.c_str(),(*it).c_str()))
								{
									
									bfind = true;
									break;
								}
							}
						}
					}
					if(bfind)
					{
						ls_path.push_back(path);
						//arrName.push_back(name);
						count ++;
					}
				}
			}
		}while(FindNextFileA(hFind,&fd));
		FindClose(hFind);
	}
	return count;
#else
	string str,new_dir;
	int n = 0;

	new_dir = dir;
	if(new_dir.empty())
		return 0;
	if(new_dir.at(new_dir.length()-1)=='/')
		new_dir.erase(new_dir.length()-1);
	if(new_dir.empty())
		return 0;

	list<string> ls_suffix;
	list<string>::iterator it;
	int count = get_string_index_count(suffix,"|");
	for(int i=0;i<count;++i)
	{
		str = get_string_index(suffix,i,"|");
		string_trim(str);
		if(!str.empty())
			ls_suffix.push_back(str);
	}

	DIR *dp=NULL;
	struct dirent *entry=NULL;
	struct stat statbuf;

	dp = opendir(new_dir.c_str());
	if(NULL==dp)
	{
		printf("********** opendir faild : dir = %s \n",new_dir.c_str());
		return n;
	}

	char old_dir[1024]={0};
	getcwd(old_dir,1024);
	chdir(new_dir.c_str());

	bool bfind = false;
	while(NULL!=(entry=readdir(dp)))
	{
		if(0==strcmp(entry->d_name,".") || 0==strcmp(entry->d_name,".."))
			continue;
		//注意：使用lstat()的话，读到软连接只是读到连接的信息，不是读到连接所引用的文件的信息
		//if(0==lstat(entry->d_name,&statbuf))
		if(0==stat(entry->d_name,&statbuf))
		{
			//printf("------->d_ino=%d,  d_type=%d  path=%s/%s,  dev=%d,  ino=%d,  mode=%d,  nlink=%d,\n",(int)entry->d_ino,(int)entry->d_type,new_dir.c_str(),entry->d_name,
			//	(int)statbuf.st_dev,(int)statbuf.st_ino,(int)statbuf.st_mode,(int)statbuf.st_nlink);
			//////DT_LNK=10,DT_REG=8,DT_DIR=4
			//printf("d_ino_len =%d ,st_ino_len = %d \n",sizeof(entry->d_ino),sizeof(statbuf.st_ino));

			if(S_ISREG(statbuf.st_mode))
			{
				bfind = false;
				if(ls_suffix.empty())
				{
					bfind = true;
				}
				else
				{
					str = entry->d_name;
					int pos1,pos2;
					pos1 = str.rfind('.');
					pos2 = str.rfind('/');
					if(pos2>=pos1)
						str = "";
					else
						str = str.substr(pos1+1);

					if(!str.empty())
					{
						for(it=ls_suffix.begin();it!=ls_suffix.end();++it)
						{
							//printf("----------scan suffix:::: %s --- %s \n",str.c_str(),(*it).c_str());
							if(0==strcasecmp(str.c_str(),(*it).c_str()))
							{
								
								bfind = true;
								break;
							}
						}
					}
				}
				if(bfind)
				{
					//如果不允许重复的话检查是否已经存在
					if(!enable_samefile)
					{
						for(list<int>::iterator it=ls_ino.begin();it!=ls_ino.end();++it)
						{
							if((*it) == (int)statbuf.st_ino)
							{
								bfind = false;
							}
						}
					}
				}
				if(bfind)
				{
					ls_path.push_back(new_dir + "/" + entry->d_name);
					ls_ino.push_back((int)statbuf.st_ino);
					++n;
				}
			}
			else if(S_ISDIR(statbuf.st_mode))
			{
				if(inherit && 0!=strcmp(entry->d_name,".") && 0!=strcmp(entry->d_name,".."))
					n += get_folder_files(new_dir + "/" + entry->d_name,ls_path,ls_ino,suffix,enable_samefile,inherit);
			}
			else
			{
				printf("*******unkown path : %s \n",entry->d_name);
			}
		}
	}

	//printf("+++++++++++++old_dir=%s \n",old_dir);
	chdir(old_dir);
	closedir(dp);
	return n;
#endif
}

string Util::get_filename(const string& path)
{
	int pos1 = (int)path.rfind('\\');
	int pos2 = (int)path.rfind('/');
	if(pos1<pos2)
		return path.substr(pos2+1);
	else if(pos1>pos2)
		return path.substr(pos1+1);
	else
		return path;
}
string Util::get_filename_prename(const string& path)
{
	string name = get_filename(path);
	size_t pos = name.rfind('.');
	if(pos!=string::npos)
		return name.substr(0,pos);
	return name;
}
string Util::get_filename_extension(const string& path)
{
	string name = get_filename(path);
	size_t pos = name.rfind('.');
	if(pos!=string::npos)
		return name.substr(pos);
	return "";
}
string Util::get_filedir(const string& path)
{
	int pos1 = (int)path.rfind('\\');
	int pos2 = (int)path.rfind('/');
	if(pos1<pos2)
		return path.substr(0,pos2+1);
	else if(pos1>pos2)
		return path.substr(0,pos1+1);
	else
		return "";
}
void Util::filepath_split(const string& path,string& dir,string& prename,string& ext)
{
	int pos = (int)path.rfind('\\');
	int pos2 = (int)path.rfind('/');
	if(pos<pos2) pos = pos2;
	string name;
	if(pos>=0)
	{
		dir = path.substr(0,pos+1);
		name = path.substr(pos+1);
	}
	else
	{
		dir = "";
		name = path;
	}
	pos = (int)name.rfind('.');
	if(pos>=0)
	{
		prename = name.substr(0,pos);
		ext = name.substr(pos);
	}
	else
	{
		prename = name;
		ext = "";
	}
}

/////////////////////////////////////////////////////////////
long long Util::atoll(const char* _Str)
{
	long long i=0;
	if(NULL==_Str)
		return 0;
	if(1!=sscanf(_Str,"%lld",&i))
		return 0;
	return i;
}
/////////////////////////////////////////////////////////////
//*****************************************
string Util::get_string_index(const string& source,int index,const string& sp)
{
	if(sp.empty() || index<0)
		return source;

	int splen = (int)sp.length();
	int pos1=0-splen,pos2=0-splen;

	int i=0;
	for(i=0;i<(index+1);i++)
	{
		pos1 = pos2+splen;
		pos2 = (int)source.find(sp,pos1);
		if(pos2<0)
			break;
	}
	if(i<index)
		return "";

	pos2 = pos2<pos1?(int)source.length():pos2;
	return source.substr(pos1,pos2-pos1);

}
int Util::get_string_index_pos(const string& source,int index,const string& sp)
{
	if(index < 0 || sp.empty())
		return -1;
	int splen = (int)sp.length();
	int pos = 0 - splen;
	for(int i=0;(pos=(int)source.find(sp,pos+splen))>=0 && i<index;i++);
	return pos;
}
int Util::get_string_index_count(const string& source,const string& sp)
{
	if(sp.empty() || source.empty())
		return 0;
	int i=1,pos=0,splen = (int)sp.length();
	pos = 0-splen;
	for(i=1;(pos=(int)source.find(sp,pos+splen))>=0;i++);
	return i;

}
string Util::set_string_index(string &source,int index,const string& val, const string& sp)
{
	if(index<0 || sp.empty())
		return source;

	int splen = (int)sp.length();
	int i=0,pos1,pos2=0;

	pos1=0-splen,pos2=0-splen;
	for(i=0;i<(index+1);i++)
	{
		pos1 = pos2+splen;
		pos2 = (int)source.find(sp,pos1);
		if(pos2<0)
		{
			if(i==index)
				break;
			else
			{
				source += sp;//补充;
				pos2  = (int)source.find(sp,pos1);//这次一定找到
			}
		}
	}

	if(pos2<0)
	{
		assert(i == index);
		source = source.substr(0,pos1) + val;
	}
	else
	{
		source = source.substr(0,pos1) + val + source.substr(pos2);
	}
	return source;

}
bool Util::get_stringlist_from_string(const string& source,list<string>& ls)
{
	ls.clear();
	int n = get_string_index_count(source,"\n");
	string str;
	size_t len;
	for(int i=0;i<n;++i)
	{
		str = get_string_index(source,i,"\n");
		if(str.empty())
			continue;
		len = str.length();
		if(str.at(len-1) == '\r')
		{
			if(1==len)
				continue;
			str.erase(len-1);
		}
		ls.push_back(str);
	}
	return true;
}
char* Util::string_trim(char* sz,char c/*=' '*/)
{
	if(NULL==sz) return sz;
	int pos1=0,pos2=(int)strlen(sz)-1;
	for(; pos1<=pos2 && c==sz[pos1];++pos1);//全空的话超越边界
	for(; pos2>pos1 && c==sz[pos2];--pos2);//上面已经判断过=,此处不再
	if( pos1>pos2)
		sz[0]='\0';
	else
	{
		if(pos1>0)
			memmove(sz,sz+pos1,pos2-pos1+1);
		sz[pos2-pos1+1] = '\0';
	}
	return sz;
}
string& Util::string_trim(string& str,char c/*=' '*/)
{
	int pos1=0,pos2=(int)str.length()-1;
	for(; pos1<=pos2 && c==str.at(pos1);++pos1);//全空的话超越边界
	for(; pos2>pos1 && c==str.at(pos2);--pos2);//上面已经判断过=,此处不再
	if( pos1>pos2)
		str = "";
	else
		str = str.substr(pos1,pos2-pos1+1);
	return str;
}

string& Util::string_trim_endline(string& str)
{
	int pos1=0,pos2=(int)str.length()-1;
	for(; pos1<=pos2 && ('\r'==str.at(pos1)||'\n'==str.at(pos1));++pos1);//全空的话超越边界
	for(; pos2> pos1 && ('\r'==str.at(pos2)||'\n'==str.at(pos2));--pos2);//上面已经判断过=,此处不再
	if( pos1>pos2)
		str = "";
	else
		str = str.substr(pos1,pos2-pos1+1);
	return str;
}
string& Util::str_replace(string& str,const string& str_old,const string& str_new)
{
	if(str_old == str_new)
		return str;
	int pos = (int)str.find(str_old);
	while(pos > -1)
	{
		str.replace(pos,str_old.length(),str_new.c_str());
		pos = (int)str.find(str_old);
	}
	return str;
}

//**********************************************

////*************************************************
////umac参数必须大于6个字节
//int get_mac_by_ifname(const char *ifname,unsigned char umac[])
//{
//#ifdef _WIN32
//	return -1;
//#else
//	int sock,ret;
//	struct ifreq ifr;
//	sock = socket(AF_INET,SOCK_STREAM,0);
//	if(sock<0)
//	{
//		perror("socket()");
//		return -1;
//	}
//	memset(&ifr,0,sizeof(ifr));
//	strcpy(ifr.ifr_name,ifname);
//	ret = ioctl(sock,SIOCGIFHWADDR,&ifr,sizeof(ifr));
//	close(sock);
//	if(0==ret)
//	{
//		memcpy(umac,ifr.ifr_hwaddr.sa_data,6);
//	}
//	else
//	{
//		perror("ioctl()");
//	}
//	return ret;
//#endif
//}
string Util::get_disksn(const char* path) 
{     
	string sn = "";
#ifndef _WIN32
	int fd;
	struct hd_driveid hid;
	char buf[1024];
	fd = open(path, O_RDONLY); 
	if(fd>=0)
	{
		if(-1!=ioctl(fd, HDIO_GET_IDENTITY, &hid))
		{
			memcpy(buf,hid.serial_no,sizeof(hid.serial_no));
			buf[sizeof(hid.serial_no)] = '\0';
			sn = buf;
			string_trim(sn);
		}
		close (fd);
	}
	if(sn.empty())
	{
		//通过/opt/suexec /sbin/hdparm -i /dev/sda;
		sprintf(buf,"/opt/suexec /sbin/hdparm -i %s",path);
		FILE *fp = popen(buf,"r");
		if(fp)
		{
			int n = fread(buf,1,1023,fp);
			pclose(fp);
			if(n>0)
			{
				buf[n] = '\0';
				//DEBUGMSG("#=======get disk sn:\n%s\n========\n",buf);
				char* p1 = strstr(buf,"SerialNo=");
				if(p1)
				{
					p1 += 9;
					char* p2 = strstr(p1,"\n");
					if(p2)
					{
						p2[0] = '\0';
					}
					sn = p1;
					string_trim(sn);
				}
			}
		}
	}
#endif
	//DEBUGMSG("# get_disk sn = %s \n",sn.c_str());
	return sn; 
} 
int Util::get_umac(unsigned char umac[])
{
	unsigned int iumac[6];
	string mac = get_mac();
	sscanf(mac.c_str(),"%02X%02X%02X%02X%02X%02X", 
		&iumac[0], &iumac[1], &iumac[2], &iumac[3], &iumac[4], &iumac[5]);
	for(int i=0;i<6;++i)
		umac[i] = (unsigned char) iumac[i];
	return 0;
}

string Util::get_mac()
{
	ifinfo_s inf;
	inf.ifcount = 10;
	int i=0;
	if(get_macall(&inf)>0)
	{
#ifndef _WIN32
		//返回eth0
		for(i=0;i<inf.ifcount;++i)
		{
			if(0==strcmp("eth0",inf.ifs[i].name))
			{
				return inf.ifs[i].mac;
			}
		}
#endif
		//返回在工作 的 && !lookback
		for(i=0;i<inf.ifcount;++i)
		{
			if(inf.ifs[i].isup && !inf.ifs[i].isloopback)
			{
				return inf.ifs[i].mac;
			}
		}
		//返回!lookback
		for(i=0;i<inf.ifcount;++i)
		{
			if(!inf.ifs[i].isloopback)
			{
				return inf.ifs[i].mac;
			}
		}
		return inf.ifs[0].mac;
	}
	return "000000000000";
}
int Util::get_macall(ifinfo_s *inf)
{
#ifdef _WIN32
	int i = 0;
	int n = inf->ifcount;
	inf->ifcount = 0;
	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information 
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen);
	if(dwStatus != ERROR_SUCCESS)
		return -1;
	//memcpy(umac,AdapterInfo->Address,6);
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
	do
	{
		strcpy(inf->ifs[i].name,pAdapterInfo->AdapterName);
		sprintf(inf->ifs[i].mac,"%02X%02X%02X%02X%02X%02X", 
			pAdapterInfo->Address[0], 
			pAdapterInfo->Address[1], 
			pAdapterInfo->Address[2], 
			pAdapterInfo->Address[3], 
			pAdapterInfo->Address[4], 
			pAdapterInfo->Address[5]); 
		inf->ifs[i].isup = true;
		if(MIB_IF_TYPE_LOOPBACK==AdapterInfo->Type)
			inf->ifs[i].isloopback = true;
		else
			inf->ifs[i].isloopback = false;
		strcpy(inf->ifs[i].ip,pAdapterInfo->IpAddressList.IpAddress.String);
		strcpy(inf->ifs[i].mask,pAdapterInfo->IpAddressList.IpMask.String);


		i = ++inf->ifcount;
		if(inf->ifcount>=n) break;
	}while(NULL!=(pAdapterInfo=pAdapterInfo->Next));
	return inf->ifcount;
#else
	int fd;
	struct ifconf conf;
	struct ifreq *ifr;
	sockaddr_in *sin;
	int i;
	char buff[2048];

	struct ifreq ifr2;
	

	conf.ifc_len = 2048;
	conf.ifc_buf = buff;
	fd = socket(AF_INET,SOCK_DGRAM,0);
	ioctl(fd,SIOCGIFCONF,&conf);

	inf->ifcount = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
	if(inf->ifcount>10) inf->ifcount = 10;
	for(i=0;i < inf->ifcount;++i)
	{
		sin = (struct sockaddr_in*)(&ifr->ifr_addr);
		ioctl(fd,SIOCGIFFLAGS,ifr);
		strcpy(inf->ifs[i].name,ifr->ifr_name);
		strcpy(inf->ifs[i].ip,inet_ntoa(sin->sin_addr));
		if(ifr->ifr_flags & IFF_LOOPBACK) 
			inf->ifs[i].isloopback = true;
		else
			inf->ifs[i].isloopback = false;
		if(ifr->ifr_flags & IFF_UP)
			inf->ifs[i].isup = true;
		else
			inf->ifs[i].isup = false;
		ifr++;
	}

	for(i=0;i < inf->ifcount;++i)
	{
		memset(inf->ifs[i].mac,0,16);
		memset(&ifr2,0,sizeof(ifr2));
		strcpy(ifr2.ifr_name,inf->ifs[i].name);
#ifndef _OS
		if(0==ioctl(fd,SIOCGIFHWADDR,&ifr2,sizeof(struct ifreq)))
		{
			unsigned char* umac = (unsigned char*)ifr2.ifr_hwaddr.sa_data;
			sprintf(inf->ifs[i].mac,"%02X%02X%02X%02X%02X%02X", umac[0], umac[1], umac[2], umac[3], umac[4], umac[5]);
		}
#endif
	}
	close(fd);
	return inf->ifcount;
#endif
}
int Util::get_mtu()
{
	//取所有网卡中最小的mtu
#ifdef _WIN32
	PIP_ADAPTER_ADDRESSES pad = NULL;
	ULONG padlen = 0;
	DWORD mtu = 1500;
	DWORD ret = 0;
	GetAdaptersAddresses(AF_UNSPEC,0, NULL, pad,&padlen);
	pad = (PIP_ADAPTER_ADDRESSES) malloc(padlen);
	if(NO_ERROR==(ret=GetAdaptersAddresses(AF_INET,GAA_FLAG_SKIP_ANYCAST,0,pad,&padlen)))
	{
		mtu = pad->Mtu;
		PIP_ADAPTER_ADDRESSES p = pad->Next;
		while(p)
		{
			if(p->Mtu>0 && mtu > p->Mtu)
				mtu = p->Mtu;
			p = p->Next;
		};
	}

	free(pad);
	return (int)mtu;
#else
	struct ifreq *ifr;
	struct ifconf conf;
	int fd,mtu,n,i;

	conf.ifc_len = 0;
	conf.ifc_buf = NULL;
	mtu = 1500;

	if((fd = socket(AF_INET,SOCK_DGRAM,0))<=0)
		goto fail;

	if(0!=ioctl(fd,SIOCGIFCONF,&conf))
		goto fail;
	conf.ifc_buf = (char*)malloc(conf.ifc_len);
	if(0!=ioctl(fd,SIOCGIFCONF,&conf))
		goto fail;
	
	n = conf.ifc_len / sizeof(struct ifreq);
	//printf("ifc_len = %d, n = %d, sizeof(ifreq)=%d \n",conf.ifc_len,n,sizeof(struct ifreq));
	if(n>0)
	{
		ifr = conf.ifc_req;
		if(0==ioctl(fd, SIOCGIFMTU, (void*)ifr))
		{
			mtu = ifr->ifr_mtu;
			//printf("ifr: %s , mtu=%d \n",ifr->ifr_name,ifr->ifr_mtu);
			for(i=1;i<n;++i)
			{
				ifr++;
				if(0==ioctl(fd, SIOCGIFMTU, (void*)ifr))
				{
					if(ifr->ifr_mtu>0 && mtu>ifr->ifr_mtu)
						mtu = ifr->ifr_mtu;
					//printf("ifr: %s , mtu=%d \n",ifr->ifr_name,ifr->ifr_mtu);
				}
			}
		}
	}

fail:
	if(fd>0)
		close(fd);
	if(conf.ifc_buf)
		free(conf.ifc_buf);
	return mtu;

#endif
}

int Util::get_server_time(time_t *t) //从time-nw.nist.gov 37 中获取1970年以来的秒数
{
	string ip = Util::ip_explain_ex("time-nw.nist.gov");
	SOCKET sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock == INVALID_SOCKET)
		return -1;
	
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(37);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	if(SOCKET_ERROR == connect(sock,(sockaddr*)&addr,sizeof(addr)))
	{
		closesocket(sock);
		return -1;
	}
	unsigned char nTime[16];
	int n = recv(sock,(char*)nTime,16,0);
	if(n<=0)
	{
		closesocket(sock);
		return -1;
	}
	closesocket(sock);
	assert(4==n);
	unsigned int dwTime = 0;
	dwTime += nTime[0] << 24;		//整合数据	
	dwTime += nTime[1] << 16;
	dwTime += nTime[2] << 8;
	dwTime += nTime[3];
	dwTime -= (unsigned int)2208988800UL; //取得的是1970年后的时间，转成1900年时间（减70年）
	dwTime += 8*3600; //转为本地时间
	*t = (time_t)dwTime;
	return 0;
}
//**********************************************
bool Util::is_ip(const char* ip)
{
	unsigned int ip_n[4];
	if(NULL!=ip && 4==sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
		return true;
	return false;
}
string Util::ip_explain(const char* s)
{
	string ip="";
	if(NULL==s || 0==strlen(s))
		return ip;
	if(INADDR_NONE != inet_addr(s))
	{
		return s;
	}
	else
	{
		ip = DIPCache::findip(s);
		if(!ip.empty())
			return ip;

		in_addr sin_addr;
		hostent* host = gethostbyname(s);
		if (host == NULL) 
		{
			printf("gethostbyname return null\n");
#ifdef _WIN32

#else
			printf("----Util::ip_explain: use ping to explain (%s) ----\n",s);
			char cmd[1024];
			sprintf( cmd, "ping -c 1 %s", s );
			FILE * pd = popen( cmd , "r");
			if ( pd != NULL )
			{
				if( fgets( cmd, 1000, pd ) != NULL )
				{
					char * s1 = strtok( cmd, "(" );
					if ( s1 != NULL )
					{
						char * ip_e = strtok(NULL,")");
						if ( is_ip(ip_e) )
						{
							printf("read ip from ping: %s\n", ip_e );
							ip = ip_e;
						}
					}
				}
				pclose( pd );
			}
#endif
		}
		else
		{
			sin_addr.s_addr = *((unsigned long*)host->h_addr);
			ip = inet_ntoa(sin_addr);
		}
		if(!ip.empty() && is_ip(ip.c_str()))
			DIPCache::addip(s,ip);
		return ip;
	}
}
typedef struct tagIPFD
{
	string dns;
	string ip;
	int is_set;
	int del;
	tagIPFD(void)
	{
		is_set = 0;
		del = 0;
	}
}IPFD;

#ifdef _WIN32
DWORD WINAPI ip_explain_T(void *p)
#else
void *ip_explain_T(void *p)
#endif
{
	IPFD *ipf = (IPFD*)p;
	ipf->ip = Util::ip_explain(ipf->dns.c_str());
	ipf->is_set = 1;
	while(!ipf->del) 
		Sleep(100);
	delete ipf;

#ifdef _WIN32
	return 0;
#else
	return (void*)0;
#endif
}
string Util::ip_explain_ex(const char* s,int maxTick/*=5000*/)
{
	if(NULL==s || 0==strlen(s))
		return "";
	string ip;
	if(INADDR_NONE != inet_addr(s))
	{
		return s;//就是ip
	}
	else
	{
		ip = DIPCache::findip(s);
		if(!ip.empty())
			return ip;

		IPFD *ipf = new IPFD();
		if(!ipf)
			return s;
		ipf->dns = s;
#ifdef _WIN32
		DWORD thid=0;
		HANDLE h = CreateThread(NULL,0,ip_explain_T,(void*)ipf,0,&thid);
		if(INVALID_HANDLE_VALUE==h)
			return s;
		else
			CloseHandle(h);
#else
		pthread_t hthread;
		if(0==pthread_create(&hthread,NULL,ip_explain_T,(void*)ipf))
			pthread_detach(hthread);
		else
			return s;
#endif
		Sleep(10);
		int i=maxTick/100;
		
		while(1)
		{
			if(ipf->is_set)
			{
				ip = ipf->ip;
				ipf->del = 1; 
				return ip;
			}
			if(i<=0)
				break;
			Sleep(100);
			i--;
		}
		ipf->del = 1;
		//return s;
		return "";
		//inet_addr("")=0,inet_addr(NULL)=-1,inet_addr("asdfs.s322.dassadf")=-1;
		//
	}
}
//****************************************************

char* Util::ip_htoa(unsigned int ip)
{
	//非线程安全
	static char buf[32];
	unsigned char ip_n[4];
	ip_n[0] = ip >> 24;
	ip_n[1] = ip >> 16;
	ip_n[2] = ip >> 8;
	ip_n[3] = ip;
	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
	return buf;
}
char* Util::ip_ntoa(unsigned int nip)
{
	//非线程安全
	//inet_addr()使用同一块内存，如果调用一个函数里面有两个参数执行了ip_ntoa，则传入结果是同一个
	static char buf[32];
	unsigned char *ip_n = (unsigned char*)&nip;
	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
	return buf;
}
string Util::ip_htoas(unsigned int ip)
{
	return ip_htoa(ip);;
}
string Util::ip_ntoas(unsigned int nip)
{
	return ip_ntoa(nip);
}
unsigned int Util::ip_atoh(const char* ip)
{
	//atonl:inet_addr("")=0,inet_addr(NULL)=-1,inet_addr("asdfs.s322.dassadf")=-1
	//sscanf返回：EOF=-1为错误，其它表示成功输入参数的个数,失败返回0或-1
	unsigned int ip_n[4]={0,0,0,0};
	if(4!=sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
		return 0;
	unsigned int iip;
	iip = ip_n[3];
	iip += (ip_n[2] << 8);
	iip += (ip_n[1] << 16);
	iip += (ip_n[0] << 24);
	return iip;
}
unsigned int Util::ip_atoh_try_explain_ex(const char* s,int maxTick/*=5000*/)
{
	return  ip_atoh(ip_explain_ex(s,maxTick).c_str());
}

//*******************************************************
#ifdef _WIN32	
string Util::get_local_private_ip()
{
	string tmp;
	hostent* he = NULL;
	//he = gethostbyname(buf);
	//printf("$: gethostbyname(%s) \n",buf);
	////test:
	he = gethostbyname(NULL);
	if(he == NULL || he->h_addr_list[0] == 0)
	{
		return "";
	}

	in_addr addr;
	int i = 0;
	
	// We take the first ip as default, but if we can find a better one, use it instead...
	memcpy(&addr, he->h_addr_list[i++], sizeof addr);
	tmp = inet_ntoa(addr);
	if(strncmp(tmp.c_str(), "127", 3) == 0 || (!Util::is_private_ip(tmp) && strncmp(tmp.c_str(), "169", 3) != 0) )
	{
		while(he->h_addr_list[i]) 
		{
			memcpy(&addr, he->h_addr_list[i], sizeof addr);
			string tmp2 = inet_ntoa(addr);
			if(strncmp(tmp2.c_str(), "127", 3) !=0 && (Util::is_private_ip(tmp2) || strncmp(tmp2.c_str(), "169", 3) == 0) )
			{
				tmp = tmp2;
			}
			i++;
		}
	}
	printf("$: ====Local_ip:%s \n",tmp.c_str());

	return tmp;
}
#else
string Util::get_local_private_ip()
{
	register int fd, num,i;
    struct ifconf ifc;
    struct ifreq buf[32];
	struct ifreq *ifr;
	sockaddr_in *sin;
	string ip="0.0.0.0";
	string tmp;

	printf("$: //***********************//\n");
	printf("$: //**get local private ip : \n");
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
	{
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if(!ioctl(fd, SIOCGIFCONF, (char *) &ifc)) 
		{
            num = ifc.ifc_len / sizeof(struct ifreq);
			ifr = ifc.ifc_req;
            printf("$:interface num=%d\n", num);
			for(i=0;i<num;++i,ifr++)
			{
				sin = (struct sockaddr_in*)(&ifr->ifr_addr);
				if(0==ioctl(fd,SIOCGIFFLAGS,ifr))
				{
					if( !(ifr->ifr_flags & IFF_LOOPBACK) && ifr->ifr_flags & IFF_UP ) 
					{
						tmp = inet_ntoa(sin->sin_addr);
						if(strncmp(tmp.c_str(), "127", 3) !=0 && (is_private_ip(tmp) || strncmp(tmp.c_str(), "169", 3) == 0) )
						{
							ip = tmp;
							break;
						}
					}
				}

			}
		}
		close(fd);
	}
	else
        perror("cpm: socket()");
	printf("$: local private ip : %s \n",ip.c_str());
	printf("$: //***********************//\n");
	return ip;
}
#endif
/////////////////////////////////////////////////////////////
typedef struct tagGPIP
{
	string ip;
	int is_set;
	int del;
	tagGPIP(void)
	{
		is_set = 0;
		del = 0;
	}
}GPIP;
#ifdef _WIN32
DWORD WINAPI get_local_private_ip_ex_T(void *p)
#else
void *get_local_private_ip_ex_T(void *p)
#endif
{
	GPIP *ipf = (GPIP*)p;
	ipf->ip = Util::get_local_private_ip();
	ipf->is_set = 1;
	while(!ipf->del) 
		Sleep(100);
	delete ipf;
#ifdef _WIN32
	return 0;
#else
	return (void*)0;
#endif
}
string Util::get_local_private_ip_ex(int timeout_tick/*=5000*/)
{
	string tmp="0.0.0.0";

	GPIP *ipf = new GPIP();
		if(!ipf)
			return tmp;
#ifdef _WIN32
		DWORD thid=0;
		HANDLE h = CreateThread(NULL,0,get_local_private_ip_ex_T,(void*)ipf,0,&thid);
		if(INVALID_HANDLE_VALUE==h)
			return tmp;
		else
			CloseHandle(h);
#elif defined(_ECOS_8203)
		//TODO_ECOS: 暂时不独立线程创建
		delete ipf;
		tmp = get_local_private_ip();
		return tmp;
#else
		pthread_t hthread;
		if(0==pthread_create(&hthread,NULL,get_local_private_ip_ex_T,(void*)ipf))
			pthread_detach(hthread);
		else
			return tmp;
#endif
		Sleep(10);
		int i=timeout_tick/100;
		while(1)
		{
			if(ipf->is_set)
			{
				tmp = ipf->ip;
				ipf->del = 1; //总时让另外线程删除它最安全
				return tmp;
			}
			if(i<=0)
				break;
			Sleep(100);
			i--;
		}
		ipf->del = 1;
	return tmp;
}

bool Util::is_private_ip(string const& ip) 
{
	unsigned long naddr;

	naddr = inet_addr(ip.c_str());

	if (naddr != INADDR_NONE) {
		unsigned long haddr = ntohl(naddr);
		return ((haddr & 0xff000000) == 0x0a000000 || // 10.0.0.0/8
				(haddr & 0xff000000) == 0x7f000000 || // 127.0.0.0/8
				(haddr & 0xfff00000) == 0xac100000 || // 172.16.0.0/12
				(haddr & 0xffff0000) == 0xc0a80000);  // 192.168.0.0/16
	}
	return false;
}

//*******************************************************
int Util::url_element_split(const string& url,string& server,unsigned short& port,string& cgi)
{
	//解析url
	string str;
	int pos = 0,pos2 = 0, pos3 = 0;
	port = 80;
	server = "";
	cgi = "";

	pos = (int)url.find("://",0);
	if(pos >= 0)
		pos += 3;
	else
	{
		pos = 0;
	}

	pos2 = (int)url.find(":",pos);
	pos3 = (int)url.find("/",pos);

	if(pos3 > 0 && pos2 > pos3)
		pos2 = -1;

	if(pos3 > pos)
	{
		if(pos2>pos)
		{
			server = url.substr(pos,pos2-pos);
			str = url.substr(pos2+1,pos3-pos2-1);
			port = atoi(str.c_str());
		}
		else
		{
			server = url.substr(pos,pos3-pos);
		}
		cgi = url.substr(pos3);
	}
	else
	{
		if(pos2>pos)
		{
			server = url.substr(pos,pos2-pos);
			str = url.substr(pos2+1);
			port = atoi(str.c_str());
		}
		else
		{
			server = url;
		};
	}

	return 0;
}
string Util::url_get_name(const string& url)
{
	if(url.empty())
		return "";
	int n = 0;
	string str = url;
	n = (int)str.find("?");
	if(n>=0)
		str.erase(n); //后面参数可能带有/,所以先去掉参数部分
	n = (int)str.rfind('/');
	return str.substr(n+1);
}
string Util::url_get_cgi(const string& url)
{
	if(url.empty())
		return "";
	int n = 0;
	string str = url;
	n = (int)str.find("://");
	if(n>=0)
		str = str.substr(n+3);
	n = (int)str.find("/");
	assert(n>=0);
	if(n>0)
		str = str.substr(n);
	return str;
}
string Util::url_get_parameter(const string& url,const string& parameter)
{
	if(url.empty() || parameter.empty())
		return "";
	int pos;
	string str;
	str = parameter;
	str += "=";
	pos = (int)url.find(str);
	if(pos < 0)
		return "";
	str = url.substr(pos + parameter.length()+1);

	pos = (int)str.find("&");
	if(pos >= 0)
		str = str.substr(0,pos);
	return str;
}
//获取URL参数，假设url参数为cgi中的最后一个参数
string Util::url_get_last_parameter(const string& url,const string& parameter)
{
	string val;
	string str;
	str = parameter;
	str += "=";
	int pos = (int)url.find(str);
	if(pos>0)
	{
		val = url.substr(pos+4);
	}
	return val;
}
string Util::url_cgi_get_path(const string& cgi)
{
	int pos = (int)cgi.find("?");
	if(pos>=0)
		return cgi.substr(0,pos);
	return cgi;
}

//****************************************************
string Util::time_to_datetime_string(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec);
	return buf;
}
string Util::time_to_datetime_string2(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%d%02d%02d.%02d%02d%02d",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec);
	return buf;
}
string Util::time_to_date_string(const time_t& _Time)
{
	tm *t = localtime(&_Time);
	char buf[128];
	sprintf(buf,"%d-%02d-%02d",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday);
	return buf;
}

//*******************************************************
int Util::string_array_find(int argc,char** argv,const char* str)
{
	for(int i=0;i<argc;++i)
	{
		if(0==strcmp(argv[i],str))
			return i;
	}
	return -1;
}
//*******************************************************
bool Util::is_write_debug_log()
{
	static int s_writelog = 2;
	if(0==s_writelog)
	{
		return false;
	}
	else if(2==s_writelog)
	{
		if(Util::file_exist("./debug.log"))
			s_writelog = 1;
		else
		{
			s_writelog = 0;
			return false;
		}
	}
	return true;
}
int Util::write_debug_log(const char *strline,const char *path)
{
	if(!is_write_debug_log())
		return -1;
	return write_log(strline,path);
}
int Util::write_log(const char *strline,const char *path)
{
	char *buf=new char[strlen(strline)+128];
	if(!buf)
		return -1;
	time_t tt = time(0);
	tm *t = localtime(&tt);
	sprintf(buf,"[%d-%d-%d %02d:%02d:%02d] %s \r\n",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec,strline);

	FILE *fp = fopen(path,"ab+");
	if(fp)
	{
		fwrite(buf,strlen(buf),1,fp);
		fclose(fp);
	}
	delete[] buf;
	return 0;
}


