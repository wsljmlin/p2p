#include "Initializers.h"
#include "Scheduler.h"
#include "MemBlockManager.h"
#include "StatManager.h"
#include "Setting.h"
#include "PeerManager.h"
//#include "UDPServer.h"

typedef Singleton<Scheduler> SchedulerSngl;

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

int stunsvr_init()
{
	int ret = 0;
	socket_init();
	ret |= SettingSngl::instance()->init();
#ifdef _WIN32
	IOReactorSngl::instance(new SelectReactor());
#else
	IOReactorSngl::instance(new ProReactor(PEER_MAX_SIZE));
#endif
	MemBlockPoolSngl::instance(new MemBlockManager());
	StatManagerSngl::instance();
	ret |= MemBlockPoolSngl::instance()->init();
	ret |= PeerManagerSngl::instance()->init();
	//ret |= instance_udps1()->open(SettingSngl::instance()->get_accept_port(),SettingSngl::instance()->get_accept_ip().c_str(),IOReactorSngl::instance());
	//ret |= instance_udps2()->open(SettingSngl::instance()->get_accept_port2(),SettingSngl::instance()->get_accept_ip().c_str(),IOReactorSngl::instance());
	ret |= SchedulerSngl::instance()->run();
	return ret;
}
void stunsvr_fini()
{
	SchedulerSngl::instance()->end();
	//instance_udps1()->close();
	//instance_udps2()->close();
	PeerManagerSngl::instance()->fini();
	MemBlockPoolSngl::instance()->fini();

	//destroy_udps1();
	//destroy_udps2();
	SchedulerSngl::destroy();
	PeerManagerSngl::destroy();
	MemBlockPoolSngl::destroy();
	StatManagerSngl::destroy();
	IOReactorSngl::destroy();
	SettingSngl::destroy();
	
	socket_fini();
}

