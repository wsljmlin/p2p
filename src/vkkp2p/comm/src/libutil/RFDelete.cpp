#include "RFDelete.h"


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>
#include <stdio.h>
#pragma warning(disable:4996)

//递归删除目录和文件

int rf_delete_dir(const char* path)
{
	char dir[MAX_PATH];
	char dir_[MAX_PATH];
	char file_path[MAX_PATH];
	strcpy(dir,path);
	char c=dir[strlen(path) - 1];
	if(c != '/' && c!='\\')
		strcat(dir, "\\");
	strcpy(dir_,dir);
	strcat(dir_, "*.*");

	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA(dir_,&fd);
	if(hFind!=INVALID_HANDLE_VALUE)
	{
		do{
			if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if(0!=strcmp(fd.cFileName,".") && 0!=strcmp(fd.cFileName,".."))
				{
					sprintf(file_path,"%s%s",dir,fd.cFileName);
					rf_delete_dir(file_path);// 递归调用删除目录下的文件
				}
			}
			else
			{
				sprintf(file_path,"%s%s",dir,fd.cFileName);
				::DeleteFileA(file_path);
				//printf("--file-del: %s \n",file_path);
			}
		}while(FindNextFileA(hFind,&fd));
		::FindClose(hFind);
	}
	//printf("--dir-del: %s \n",path);
	if(::RemoveDirectoryA(path))
		return 0;
	return -1;
}
int rf_delete_(const char* path)
{
	WIN32_FIND_DATAA fd;
	HANDLE hFind = ::FindFirstFileA(path,&fd);
	int ret = -1;
	if(hFind!=INVALID_HANDLE_VALUE)
	{
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(0!=strcmp(fd.cFileName,".") && 0!=strcmp(fd.cFileName,".."))
			{
				ret = rf_delete_dir(path);// 递归调用删除目录下的文件
			}
		}
		else
		{
			//printf("--file-del: %s \n",path);
			if(::DeleteFileA(path))
				ret = 0;
		}
		::FindClose(hFind);
	}
	return ret;
}
#else
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

//递归删除目录和文件
int rf_delete_(const char* path)
{
	struct stat st;
	if(lstat(path, &st)==0)
	{
		if(S_ISREG(st.st_mode))// 删除文件
		{
			//printf("--delete-file : %s \n",path);
			if(0==unlink(path) || (errno == ENOENT))
				return 0;
			else
				return -1;
		}
		else if(S_ISDIR(st.st_mode))//删除目录
		{
			char file_path[512] = {0};
			struct dirent *dir_ret = NULL;
			union 
			{
				struct dirent d;
				char b[offsetof(struct dirent, d_name) + 512 + 1]; 
			}u;
			DIR* dir = opendir(path);
			if(dir != NULL) 
			{
				while((0==readdir_r(dir,&u.d,&dir_ret)) && (NULL!=dir_ret))
				{
					if(0==strncmp(dir_ret->d_name, ".", 2) || 0==strncmp(dir_ret->d_name, "..", 3)) 
						continue;
					else 
					{
						strcpy(file_path, path);
						if(file_path[strlen(path) - 1] != '/')
							strcat(file_path, "/");
						strcat(file_path, dir_ret->d_name);
						rf_delete_(file_path);// 递归调用删除目录下的文件
					}
				}
				closedir(dir);
				//printf("--delete-dir : %s \n",path);
				if(0==rmdir(path))
					return 0;
				else
					return -2;
			}
			else
				return -3;
		}
	}
	else
	{
		unlink(path);//文件不存在时有可能是符合链接指向的文件不存在, 也要删除
		//printf("-- try-delete : %s \n",path);
	}
	return 0;
} 
#endif

