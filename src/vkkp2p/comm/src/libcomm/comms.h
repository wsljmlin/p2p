#pragma once

#if !defined(ANDROID)
//FD_SETSIZE 决定 fd_set 的数组大小，即最大select数目，windows在winsock2.h中定义
#ifdef FD_SETSIZE
	#undef FD_SETSIZE
#endif
#define FD_SETSIZE 1024    
#endif

#ifdef _WIN32
// 4482 4267 4018 4800 4311 4312 4102
#pragma warning(disable:4996)
	#include <winsock2.h>
	typedef int socklen_t;
#else
	#include <unistd.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/socket.h>
	#include <sys/select.h>
#ifndef NO_EPOLL
	#include <sys/epoll.h>
#endif
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <net/if_arp.h>  
	#include <net/if.h>
	//#include <stropts.h>
	#include <sys/ioctl.h> 
	#include <dirent.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

#include "basetypes.h"
#include "Singleton.h"
#include "Speaker.h"
#include "Thread.h"

