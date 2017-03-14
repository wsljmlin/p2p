
#include "commons.h"
#include "Initializers.h"
#include "Util.h"
#include "license.h"
#include "HttpsvrHandler.h"
#include "Setting.h"

#ifdef _WIN32
#include <crtdbg.h>
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
#else
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#endif



bool check_license();
int main(int argc,char** argv)
{
#ifdef _WIN32
	//输出内存泄漏
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag( tmpFlag );
#else
	signal(SIGPIPE, SIG_IGN); //忽略Broken pipe,否则socket对端关闭时，很容易写会出现Broken pipe（管道破裂）
#endif
	printf("#tccsvr server start... \n");
	if(0!=stunsvr_init())
	{
		stunsvr_fini();
		return 0;
	}
	
	//if(!check_license())
	//{
	//	DEBUGMSG("license ERROR! exit\n");
	//	Sleep(2000);
	//	stunsvr_fini();
	//	return 0;
	//}
	httpsvr_start(SettingSngl::instance()->get_accept_http_port());

#ifdef _WIN32
	char c='\0';
	while(c!='q')
		c=::getchar();
#else
	char c='\0';
	while(c!='q')
		c=::getchar();
#endif

	httpsvr_stop();
	stunsvr_fini();
	return 0;
}

bool check_license()
{
	string licpath = Util::get_module_dir() + "stunsvrlic.dat";
	string ip = Util::get_local_private_ip();
	time_t now;
	time(&now);
	tm *ptm = localtime(&now);
	if(ptm != NULL)
	{
		char buf[12];
		buf[8] = 0;
		sprintf(buf,"%04d%02d%02d",ptm->tm_year+1900,ptm->tm_mon+1, ptm->tm_mday);
		return CmnLicense::CheckLicenseFile(ip.c_str(),buf,licpath.c_str());
	}
	return false;
}



