
#include "dcc.h"
#include "Util.h"
#include <winable.h>
#include <shellapi.h>

//ÎÞconsole´°¿Ú
#ifndef _DEBUG
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
#endif
//*****************************************************
//main:
int main(int argc,char** argv)
{
	if(!Util::check_process_single())
	{
		printf(" ****** exe is running!!! (%s) ****** \n",argv[0]);
		return 0;
	}
	Util::debug_memleak();
	Util::socket_init();
	Util::chdir(Util::get_module_dir().c_str());

	dcc_init("./dcc_config.xml");

	char c=0;
	while(c!='q')
	{
#ifndef _DEBUG
		Sleep(10000);
#else
		//c = getchar();
#endif
	}

	dcc_fini();
	Util::socket_fini();
	return 0;
}
//*****************************************************

