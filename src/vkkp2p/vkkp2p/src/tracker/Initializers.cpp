#include "Initializers.h"
#include "Scheduler.h"
#include "MemBlockManager.h"
#include "StatManager.h"
#include "Setting.h"
#include "UserManager.h"
#include "PeerManager.h"
#include "AutoFileManager.h"

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

int tracker_init()
{
	int ret = 0;
	socket_init();
	ret |= SettingSngl::instance()->init();
	MemBlockPoolSngl::instance(new MemBlockManager());
	StatManagerSngl::instance();
	ret |= MemBlockPoolSngl::instance()->init();
	ret |= AutoFileManagerSngl::instance()->init();
	ret |= UserManagerSngl::instance()->init();
	ret |= PeerManagerSngl::instance()->init();
	ret |= SchedulerSngl::instance()->run();
	return ret;
}
void tracker_fini()
{
	SchedulerSngl::instance()->end();
	PeerManagerSngl::instance()->fini();
	UserManagerSngl::instance()->fini();
	AutoFileManagerSngl::instance()->fini();
	MemBlockPoolSngl::instance()->fini();

	SchedulerSngl::destroy();
	PeerManagerSngl::destroy();
	UserManagerSngl::destroy();
	AutoFileManagerSngl::destroy();
	MemBlockPoolSngl::destroy();
	StatManagerSngl::destroy();
	SettingSngl::destroy();
	socket_fini();
}

