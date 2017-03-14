
#include "uactest.h"
#include <stdio.h>
#include "uac.h"

#ifdef _WIN32
#include <winsock2.h>
#include <crtdbg.h> 

#ifdef NDEBUG
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
#endif

#else
#include <signal.h>
#endif



#include "uac_mempool.h"
using namespace UAC;


int socket_init()
{
#ifdef _WIN32
	WSADATA wsaData;
	if(0!=WSAStartup(0x202,&wsaData))
	{
		perror("WSAStartup false! : ");
		return -1;
	}
#endif
	return 0;
}
void socket_fini()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

int uactest_main(int argc,char** argv)
{
	
#ifdef _WIN32
	//输出内存泄漏
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tmpFlag );
#else
	signal(SIGPIPE, SIG_IGN); //忽略Broken pipe,否则socket对端关闭时，很容易写会出现Broken pipe（管道破裂）
#endif

	socket_init();
	uac_init(6125,NULL,0);

	char c=0;
	while('q'!=c)
		c = getchar();

	uac_fini();
	socket_fini();
	return 0;
}

