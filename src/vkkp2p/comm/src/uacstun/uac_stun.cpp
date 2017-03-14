#include "uac_stun.h"
#include "uac_Setting.h"
#include "uac_UDPStunServer.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace UAC;

int socket_init()
{
#ifdef _WIN32
	WSADATA wsaData;
	if(0!=WSAStartup(0x202,&wsaData))
	{
		perror("#***WSAStartup false! : ");
		return -1;
	}
	return 0;
#endif
	return 0;
}
void socket_fini()
{
#ifdef _WIN32
	WSACleanup();
#endif
}


bool g_is_init = false;
int stunsvr_init()
{
	if(g_is_init)
	{
		assert(false);
		return 0;
	}
	g_is_init = true;
	socket_init();

	SettingSngl::instance();

	SettingSngl::instance()->init();
	if(0!=StunServer::stun_open(SettingSngl::instance()->get_stunsvr_config()))
	{
		return -1;
	}
	SchedulerSngl::instance()->run();
	return 0;
}

void stunsvr_fini()
{
	if(!g_is_init)
		return;
	g_is_init = false;

	SchedulerSngl::instance()->end();

	StunServer::stun_close();
	SettingSngl::instance()->fini();

	SchedulerSngl::destroy();
	SettingSngl::destroy();
	
	socket_fini();
}

bool check_config()
{
	PTL_STUN_RspStunsvrConfig_t local_config,remote_config;
	if(0!=StunServer::stun_check_config(local_config,remote_config))
	{
		printf("*** check config failed! *** \n");
		return false;
	}
	else
	{
		//stunA为提供给客户的自己信息，必须为stunB所见的一样
		//stunB为对方绑定的
		if((local_config.accept_port1==remote_config.eyePort || local_config.accept_port2==remote_config.eyePort) &&
			local_config.stunB_port1 == remote_config.accept_port1 && local_config.stunB_port2 == remote_config.accept_port2)
		{
			printf(" check config ok ( 与 stunB 握手成功) \n");
			return true;
		}
		printf("*** check config failed! 与 stunB 握手失败) *** \n");
		return false;
	}
}


int max_count = 1000;
int loop_count= 0;
int main(int argc,char** argv)
{
	printf("uac stunsvr monitor start...\n");
	while( true ) {
		pid_t ch = fork();
		loop_count = 0;

		if ( ch == (pid_t)-1 ) {
			printf("can not fork ...\n");
			exit(-1);
		}
		if( ch == 0 ) {
			break;
		}
		int status;
		waitpid( ch, &status, 0 );
		printf("uas stunsrv services end ,and restart soon ...\n");
		Sleep(1000);
	}


	printf("uac stunsvr(20160323) start...\n");
	socket_init();
	if(0!=stunsvr_init())
	{
		stunsvr_fini();
		socket_fini();
		return -1;
	}

	Sleep(2000);
	check_config();

/*
	char c = 0;
	while(c!='q' )
		c = getchar();
*/
	while( loop_count < max_count ) {
		Sleep(3000);
	}
	printf("uac loop %d \n", loop_count);

	stunsvr_fini();
	socket_fini();
	printf("uac stunsvr stop! \n");
	return 0;
}

//*************************************************
Scheduler::Scheduler(void)
:m_brun(false)
{
}

Scheduler::~Scheduler(void)
{
}
int Scheduler::run()
{
	if(m_brun)
		return 1;
	m_brun = true;
	this->activate();
	return 0;
}
void Scheduler::end()
{
	if(!m_brun)
		return;
	m_brun = false;
	wait();
}

unsigned long last_tick = GetTickCount();
int Scheduler::work(int e)
{
	int ret;
	while(m_brun)
	{
		ret = StunServer::handle_root(0);
		if(-1==ret)
			Sleep(8);
		else 
			loop_count++;

		unsigned long tick = GetTickCount();
		if(last_tick+5000 < tick)
		{
			last_tick = tick;
//			UACLOG("# stun thread ok! \n");
		}
	}
	return 0;
}


