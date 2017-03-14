

/*
// update.exe pararm:
	--update [path ver url]  -------���url�汾,������°汾�����ر��浽path��. pathΪ���ص�·��,��reload�е�newfile
	--reload [newfile path params...]  ------ ��newfile���µ�path��,������path��ָ���� params...�����в���Ϊ����path�Ĳ���(��ȷ������,���Ա�����update�����������.

����verurl���ص�����Ϊ:
ver=...
path=...[��/��ͷ����ȫ·��.����/��ͷ�������verurl��·��.]
nd5=...[�����ļ���MD5ֵ)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Util.h"
#include "Httpc.h"
#include "sha1.h"

void show_help();
int update(const string& path,const string& ver,const string& verurl);
int reload(const string& newfile,const string& path,int argc,char** argv);
int main(int argc,char** argv)
{
	if((argc>1 && (0==strcmp(argv[1],"-h")||0==strcmp(argv[1],"--help"))))
	{
		show_help();
		return 0;
	}
	Util::debug_memleak();
	Util::socket_init();

	int i = 0;
	if(-1!=(i=Util::string_array_find(argc,argv,"--update")))
	{
		if(i+3<argc)
		{
			//--update [path ver url]
			return update(argv[i+1],argv[i+2],argv[i+3]);
		}
	}

	if(-1!=(i=Util::string_array_find(argc,argv,"--reload")))
	{
		if(i+2<argc)
		{
			//--reload [newfile path params...] 
			int new_argc = argc-(i+3);
			char** new_argv = NULL;
			if(new_argc>0)
				new_argv = &argv[i+3];
			return reload(argv[i+1],argv[i+2],new_argc,new_argv);
		}
	}

	Util::socket_fini();
	return 0;
}

void show_help()
{
	printf("update version: 1.0(20160320) \n");
	printf("update: cnvalid option: -h or --help \n");
	printf("usage update: \n");
	printf("--update [path ver url]	: ���url�汾,������°汾�����ر��浽path��. pathΪ���ص�·��,��reload�е�newfile \n");
	printf("--reload [newfile path params...] : ��newfile���µ�path��,������path��ָ���� params...�����в���Ϊ����path�Ĳ���(��ȷ������,���Ա�����update�����������. \n");
	printf("\n");
#ifdef _WIN32
	getchar();
#endif
}

/*
����: ���url�汾,������°汾�����ر��浽path��
*/

//ȡ=�ź����ֵ
string update_get_field(const string& body,const char* key)
{
	string strkey;
	size_t pos1,pos2;
	string ret;
	
	strkey = key;
	strkey += "=";
	pos1 = body.find(strkey);
	if(pos1==string::npos)
		return "";
	pos2 = body.find("\n",pos1);
	//pos2==string::npos Ҳ����
	if(string::npos==pos2)
		ret = body.substr(pos1+strkey.length());
	else
		ret = body.substr(pos1+strkey.length(),pos2-(pos1+strkey.length()));
	//ȥ��\r
	Util::string_trim(ret,'\r');
	return ret;
}
int update(const string& path,const string& ver,const string& verurl)
{
	string new_ver,new_path,new_url,new_sha1;
	char body[1024];
	string strbody;
	size_t pos;

	if(0!=Httpc::http_get(verurl,body,1024))
	{
		return -1;
	}
	strbody = body;
	new_ver = update_get_field(strbody,"ver");

	//�汾�Ƚ�
	printf("compare ver: local %s remote %s\n", ver.c_str(), new_ver.c_str());
	if(strcmp(ver.c_str(),new_ver.c_str())>=0)
		return 0; //��ǰ�汾�Ŵ��ڻ��ߵ�������汾

	new_sha1 = update_get_field(strbody,"sha1");

	//���URL
	new_path = update_get_field(strbody,"path");
	if(new_path.length()>7 && 0==strncmp(new_path.c_str(),"http://",7))
		new_url = new_path;
	else if(new_path.at(0)=='/')
	{
		pos = verurl.find("/",7);
		assert(pos!=string::npos);
		new_url = verurl.substr(0,pos) + new_path;
	}
	else
	{
		pos = verurl.rfind('/');
		assert(pos!=string::npos);
		new_url = verurl.substr(0,pos+1) + new_path;
	}
	
	//�����ļ�
	printf("try update file: %s\n",path.c_str());
	Util::file_delete(path);
	if(0!=Httpc::download_file(new_url,path))
		return -1;

	//sha1����
	if(!new_sha1.empty())
	{
		char ssha1[64];
		if(0==Sha1_BuildFile(path.c_str(),ssha1,NULL,-1,true))
		{
#ifdef SM_DBG
			printf("****new download file hash:%s\n",ssha1);
#endif
			if(0==strcmp(new_sha1.c_str(),ssha1))
			{
				printf("update file to %s and sha1 check ok! \n",path.c_str());
				return 1;
			}
		}
#ifdef SM_DBG
			printf("****check file failed, will delete it\n");
#endif
		Util::file_delete(path);
		return -2;
	}
	printf("update file to %s ok! \n",path.c_str());
	return 1;
}

int reload(const string& newfile,const string& path,int argc,char** argv)
{
	char cmd[1024];
	//�滻�ļ�����2����
	string bak = path;
	bak += ".bak";
	if(!Util::file_exist(newfile))
		return -1;
	Util::file_delete(bak);
	if(Util::file_exist(path))
	{
		//���ļ��Ƶ�bak
		int n = 0;
		while(0!=Util::file_rename(path,bak))
		{
			if(++n>20) break;
			Sleep(100);
		}
	}
	if(0!=Util::file_rename(newfile,path))
		return -1;

#ifndef _WIN32
	//�޸�ִ��Ȩ��
	sprintf(cmd,"chmod 775 %s",path.c_str());
	system(cmd);
#endif
	sprintf(cmd,"%s",path.c_str());
	for(int i=0;i<argc;++i)
		sprintf(cmd+strlen(cmd)," %s",argv[i]);
	return system(cmd);
}

