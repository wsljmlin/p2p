#include "ubnservice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basetime.h"

#ifdef _WIN32
#pragma warning(disable:4996)
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>
#include <winsock2.h>

typedef int socklen_t;

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef int SOCKET;
#define closesocket close
#define INVALID_SOCKET -1
#define SOCKET_ERROR (-1)

#endif

typedef struct tag_unb_server_handle
{
	bool bthreadroot;
	bool bexit;
#ifdef _WIN32
	HANDLE thread;
#else
	pthread_t thread;
#endif
	SOCKET fd;
	char name[64];
	int namelen;
}unb_server_handle_t;

#ifdef _WIN32
DWORD WINAPI _ubn_server_root_T(LPVOID p)
#else
void* _ubn_server_root_T(void* p)
#endif
{
	ubn_server_root(p);
	return 0;
}

UBN_SERVER_HANDLE ubn_server_open(unsigned short port,const char* ubnname,bool bthreadroot)
{
	SOCKET fd;
#ifdef _WIN32
	bool isbroadcast = true;
#else
	int isbroadcast = 1;
#endif
	sockaddr_in addr;
	
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = 0;
	addr.sin_port = htons(port);

	fd = socket(AF_INET,SOCK_DGRAM,0);
	if(INVALID_SOCKET==fd)
		return NULL;
	if(0!=setsockopt(fd,SOL_SOCKET,SO_BROADCAST,(const char*)&isbroadcast,sizeof(isbroadcast)))
	{
		perror("*** setsockopt(SO_BROADCAST): ");
		closesocket(fd);
		return NULL;
	}

	if(SOCKET_ERROR == bind(fd,(const sockaddr*)& addr,sizeof(addr)))
	{
		perror("*** bind(): ");
		closesocket(fd);
		return NULL;
	}

	unb_server_handle_t *h = new unb_server_handle_t();
	h->bthreadroot = bthreadroot;
	h->bexit = false;
	h->fd = fd;
	h->namelen = (int)strlen(ubnname);
	if(h->namelen>63) h->namelen=63;
	memcpy(h->name,ubnname,h->namelen);
	h->name[63] = '\0';
	
	if(h->bthreadroot)
	{
#ifdef _WIN32
	DWORD threadId=0;
	h->thread = CreateThread(NULL,0,_ubn_server_root_T,(void*)h,0,&threadId);
	if(INVALID_HANDLE_VALUE == h->thread)
	{
		delete h;
		closesocket(fd);
		return NULL;
	}
#else
	if(0 != pthread_create(&h->thread,NULL,_ubn_server_root_T,(void*)h))
	{
		delete h;
		closesocket(fd);
		return NULL;
	}
#endif
	}

	printf("#ubn server open( bind port:%d )... \n",port);
	return (UBN_SERVER_HANDLE)h;
}

void ubn_server_root(UBN_SERVER_HANDLE ubnh)
{
	unb_server_handle_t* h = (unb_server_handle_t*) ubnh;

	sockaddr_in from_addr;
	socklen_t from_len;

	char rsp_name[1024];
	int rsp_len;
	char buf[1024];
	int n;
	sprintf(rsp_name,"RSP-UBNNAME:%s",h->name);
	rsp_len = (int)strlen(rsp_name);
	while(!h->bexit)
	{
		memset(&from_addr,0,sizeof(from_addr));
		from_len = sizeof(from_addr);
		n = recvfrom(h->fd,buf,1023,0,(sockaddr*)&from_addr,&from_len);
		if(n>0)
		{
			buf[n] = '\0';
			if(0==strcmp(buf,"REQ-UBNNAME:"))
			{
				n = sendto(h->fd,rsp_name,rsp_len,0,(const sockaddr*)&from_addr,from_len);
				printf("rsp : %s(%s:%d) => \"%s\" \n",buf,inet_ntoa(from_addr.sin_addr),ntohs(from_addr.sin_port),h->name);
			}
		}
	}
}

int ubn_server_close(UBN_SERVER_HANDLE ubnh)
{
	unb_server_handle_t* h = (unb_server_handle_t*) ubnh;
	h->bexit = true;
	closesocket(h->fd);

	if(h->bthreadroot)
	{
#ifdef _WIN32
	WaitForSingleObject(h->thread,INFINITE);
	CloseHandle(h->thread);
#else
	pthread_join(h->thread,NULL);
#endif
	}

	delete h;
	return 0;
}

int ubn_find_server(unsigned short port,const char* ubnname,ubn_server_t* us,unsigned int timeo/*=3000*/) //count 指定最多查找个数，返回实际查找个数
{
	int count;
	unsigned int ip;
	unsigned int begtick;
	char name[64];

	SOCKET fd;
#ifdef _WIN32
	bool isbroadcast = true;
#else
	int isbroadcast = 1;
#endif
	sockaddr_in addr,from_addr;
	socklen_t from_len;
	char buf[1024];
	char req_name[1024];
	int n;
	
	count = us->count;
	us->count = 0;
	sprintf(req_name,"REQ-UBNNAME:");
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_BROADCAST;
	addr.sin_port = htons(port);

	fd = socket(AF_INET,SOCK_DGRAM,0);
	if(INVALID_SOCKET==fd)
		return -1;
	if(0!=setsockopt(fd,SOL_SOCKET,SO_BROADCAST,(const char*)&isbroadcast,sizeof(isbroadcast)))
	{
		perror("*** setsockopt(SO_BROADCAST): ");
		closesocket(fd);
		return -1;
	}

	//设置超时:
#ifdef _WIN32
	int x = 100;
#else
	struct timeval x;  
	x.tv_sec = 0;
	x.tv_usec = 100000;
#endif
	if(-1==setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&x,sizeof(x)))
		perror("setsockopt SO_RCVTIMEO");
	if(-1==setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(char*)&x,sizeof(x)))
		perror("setsockopt SO_SNDTIMEO");

	//if(SOCKET_ERROR == bind(fd,(const sockaddr*)& addr,sizeof(addr)))
	//{
	//	perror("*** bind(): ");
	//	closesocket(fd);
	//	return -1;
	//}

	begtick = GetTickCount();
	while(1)
	{
		if(sendto(fd,req_name,(int)strlen(req_name),0,(const sockaddr*)&addr,sizeof(addr))>0)
		{
			//printf("ubnserver find ubnname in port(%d): \n",port);
			while(1)
			{
				memset(&from_addr,0,sizeof(from_addr));
				from_len = sizeof(from_addr);
				n = recvfrom(fd,buf,1024,0,(sockaddr*)&from_addr,&from_len);
				if(n>0)
				{
					buf[n] = '\0';
					if(strstr(buf,"RSP-UBNNAME:"))
					{
						ip = ntohl(from_addr.sin_addr.s_addr);
						strcpy(name,buf+12);
						if(ubnname && 0!=strcmp(name,ubnname))
							continue;

						bool bfind = false;
						for(int i=0;i<us->count;++i)
						{
							if(ip==us->server[i].ip && 0==strcmp(us->server[i].name,name))
							{
								bfind = true;
								break;
							}
						}
						if(!bfind)
						{
							//printf("#R:find ubnname(%s:%d) => \"%s\" \n",inet_ntoa(from_addr.sin_addr),ntohs(from_addr.sin_port),buf);
							us->server[us->count].ip = ntohl(from_addr.sin_addr.s_addr);
							strcpy(us->server[us->count].name,name);
							us->count++;
							if(us->count >= count)
								break;
						}
					}
				}
				else
				{
					break;
				}
			}
			if(us->count >= count || begtick + timeo < GetTickCount())
				break;

		}
		else
		{
			perror("***sendto(req_name):");
			break;
		}
	}
	

	closesocket(fd);
	return us->count;
}

