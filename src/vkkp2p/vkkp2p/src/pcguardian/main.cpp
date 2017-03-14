
#include "ProcessInfo.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int _GetModuleFileName(const char* lpszModuleName, char* lpszModulePath, int cbModule);

int main(int argc,char **argv)
{
	//���� SIGCLD�ź�,��ʾ�����ӽ�����ʬ
	//signal(SIGCLD, SIG_IGN); //ʹ�ú��ʹsystem()ʧ��
	
	char buf[256];
	char *name = NULL;
	if(_GetModuleFileName(NULL,buf,256))
	{
		char *p = ::strrchr(buf,'/');
		if(!p)
			p = ::strrchr(buf,'\\');
		if(p)
		{
			name = p+1;
			*p='\0';
		}
		printf("module path = %s \n",buf);
#ifdef _WIN32
		::SetCurrentDirectoryA(buf);
#else
		chdir(buf);
#endif
	}
#ifndef _WIN32
	ProcessInfo pif;
	if(argc>1 && 0==strcmp("stop",argv[1]))
	{
		pif.do_stop(name);
	}
	else
	{
		while(1)
		{
			//printf("keeplive\n");
			pif.do_keeplive();
			sleep(60);
		}
	}
#endif
	return 0;
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

	//ȡ�õ�ǰ·��
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
			//��ǰ���·�� ��Ҫ���ϵ�ǰ��ȫ·�� wwjs 2006-02-06
			//ͨ��ֱ����������������
			sprintf(szTmp, "%s/%s", szCurPath, &szCmdPath[2]);
		}
		else
		{
			//ĳЩϵͳ��Apache����, cmcLine �в�����·������Ҫ���� wwjs 2006-01-23
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
	//ȥ�������ܴ��ڵ� �� .
	if(lpszModulePath[0] == '/' && lpszModulePath[len] == '.')
	{
		lpszModulePath[len] = 0;
		len--;
	}
	return len;

}
