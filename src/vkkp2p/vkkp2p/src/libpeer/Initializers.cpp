#include "Initializers.h"
#include "Setting.h"
#include "MemBlockPoolFLB.h"
#include "FileStorage.h"
#include "FileAutoCache.h"
#include "DownloadManager.h"
#include "Tracker.h"
#include "HashManager.h"
#include "ShareService.h"
#include "Statistician.h"
#include "Scheduler.h"
#include "LocalService.h"
#include "NattypeMgr.h"
#include "DownloadListManager.h"
#include "DownManualManager.h"
#include "MessagePusher.h"
#include "uac_SocketSelector.h"
#include "LocalM3u8.h"

#ifndef _WIN32
#include <signal.h>
#endif

bool g_is_peerinit = false;
int init_acceptor();


bool is_peer_init()
{
	return g_is_peerinit;
}
int peer_init()
{
	if(g_is_peerinit)
	{
		assert(false);
		return 0;
	}
	g_is_peerinit = true;
	
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN); //忽略Broken pipe,否则socket对端关闭时，很容易写会出现Broken pipe（管道破裂）
#endif

	SettingSngl::instance();
	TimerSngl::instance();
	IOReactorSngl::instance(new SelectReactor());
	HashManagerSngl::instance();
	int as[]={1024*2,1024*8,1024*64};
	MemBlockPoolSngl::instance(new MemBlockPoolFLB(as,3));
	FileBlockPoolSngl::instance();
	MessagePusherSngl::instance();

	SettingSngl::instance()->init();
	//MemBlockPoolSngl::instance()->init();
	StatisticianSngl::instance()->init();
	FileStorageSngl::instance()->init();
	FileAutoCacheSngl::instance()->init();
	ShareServiceSngl::instance()->init();
	PeerManagerSngl::instance()->init(DownloadManagerSngl::instance(),ShareServiceSngl::instance());
	MessagePusherSngl::instance()->init();
	DownloadManagerSngl::instance()->init();
	init_acceptor();
	TrackerSngl::instance()->init();
	NattypeMgrSngl::instance()->init();
	DownloadListManagerSngl::instance()->init();
	LocalM3u8MgrSngl::instance()->init();

	HashManagerSngl::instance()->run();
	LocalServiceSngl::instance()->run();
	SchedulerSngl::instance()->run();
	DownManualManagerSngl::instance()->run();
	return 0;
}
void peer_fini()
{
	if(!g_is_peerinit)
		return;
	g_is_peerinit = false;
	DownManualManagerSngl::instance()->end();
	LocalServiceSngl::instance()->end();
	SchedulerSngl::instance()->end();
	HashManagerSngl::instance()->end();
	
	LocalM3u8MgrSngl::instance()->fini();
	DownloadListManagerSngl::instance()->fini();
	NattypeMgrSngl::instance()->fini();
	DownloadManagerSngl::instance()->fini();
	TrackerSngl::instance()->fini();
	ShareServiceSngl::instance()->fini();
	PeerManagerSngl::instance()->fini();
	MessagePusherSngl::instance()->fini();


	TCPAcceptorSngl::instance()->close();
	//uac
	UAC_SocketSelectorSngl::destroy();
	uac_fini();

	FileAutoCacheSngl::instance()->fini();
	FileStorageSngl::instance()->fini();
	StatisticianSngl::instance()->fini();
	//MemBlockPoolSngl::instance()->fini();
	SettingSngl::instance()->fini();

	LocalM3u8MgrSngl::destroy();
	DownManualManagerSngl::destroy();
	DownloadListManagerSngl::destroy();
	NattypeMgrSngl::destroy();
	LocalServiceSngl::destroy();
	TrackerSngl::destroy();
	SchedulerSngl::destroy();
	HashManagerSngl::destroy();
	TCPAcceptorSngl::destroy();
	MessagePusherSngl::destroy();
	DownloadManagerSngl::destroy();
	PeerManagerSngl::destroy();
	ShareServiceSngl::destroy();
	FileAutoCacheSngl::destroy();
	FileStorageSngl::destroy();
	StatisticianSngl::destroy();
	FileBlockPoolSngl::destroy();
	MemBlockPoolSngl::destroy();
	IOReactorSngl::destroy();
	TimerSngl::destroy();
	SettingSngl::destroy();
}

int init_acceptor()
{
	unsigned short port = SettingSngl::instance()->get_accept_port();
	string ip = SettingSngl::instance()->get_accept_ip();
	int i=0;
	while(i++<100)
	{
		if(0==TCPAcceptorSngl::instance()->open(port,ip.c_str(),PeerManagerSngl::instance(),IOReactorSngl::instance()))
		{
			g_netLiveInfo.tcpLocalPort = port;
			break;
		}
		else
			port++;
	}

	//uac 初始化
	uac_setcallback_onipportchanged(NattypeMgr::callback_udp_ipportchanged);
	//port = SettingSngl::instance()->get_accept_port();
	i=0;
	while(i++<200)
	{
		if(0==uac_init(port, SettingSngl::instance()->get_stun_ip().c_str(),SettingSngl::instance()->get_stun_port()))
		{
			g_netLiveInfo.udpLocalPort = port;
			break;
		}
		else
			port++;
	}
	UAC_SocketSelectorSngl::instance(new UAC_SocketSelector(static_cast<UAC_SocketFactory*>(PeerManagerSngl::instance())));
	
	return 0;
}

