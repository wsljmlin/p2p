#include "uac_Util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#include <winsock2.h>
#include <Iphlpapi.h>
//#pragma comment(lib,"Iphlpapi.lib")
#else
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/socket.h>
	#include <sys/select.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <netdb.h>
	#include <net/if_arp.h>  
	#include <net/if.h>
	//#include <stropts.h>
	#include <sys/ioctl.h> 
	#include <dirent.h>
	#include <stdarg.h>
	//#include <sys/vfs.h>
	#include <pthread.h>
	#include <sys/file.h>
#endif

#include "uac_basetypes.h"

namespace UAC
{

//**********************************************
string Util::ip_explain(const char* s)
{
	string ip="";
	if(NULL==s || 0==strlen(s))
		return ip;
	if(INADDR_NONE != inet_addr(s))
	{
		return s;
	}
	else
	{
		in_addr sin_addr;
		hostent* host = gethostbyname(s);
		if (host == NULL) 
		{
			printf("gethostbyname return null\n");
#ifdef _WIN32

#else
			printf("----Util::ip_explain: use ping to explain (%s) ----\n",s);
			char cmd[1024];
			sprintf( cmd, "ping -c 1 %s", s );
			FILE * pd = popen( cmd , "r");
			if ( pd != NULL )
			{
				if( fgets( cmd, 1000, pd ) != NULL )
				{
					char * s1 = strtok( cmd, "(" );
					if ( s1 != NULL )
					{
						char * ip_e = strtok(NULL,")");
						if ( ip_e != NULL )
						{
							printf("read ip from ping: %s\n", ip_e );
							ip = ip_e;
						}
					}
				}
				pclose( pd );
			}
#endif
			//return s;
			return "";
		}
		else
		{
			sin_addr.s_addr = *((unsigned long*)host->h_addr);
			ip = inet_ntoa(sin_addr);
		}

		return ip;
	}
}
typedef struct tagIPFD
{
	string dns;
	string ip;
	int is_set;
	int del;
	tagIPFD(void)
	{
		is_set = 0;
		del = 0;
	}
}IPFD;

#ifdef _WIN32
DWORD WINAPI ip_explain_T(void *p)
#else
void *ip_explain_T(void *p)
#endif
{
	IPFD *ipf = (IPFD*)p;
	ipf->ip = Util::ip_explain(ipf->dns.c_str());
	ipf->is_set = 1;
	while(!ipf->del) 
		Sleep(100);
	delete ipf;

#ifdef _WIN32
	return 0;
#else
	return (void*)0;
#endif
}
string Util::ip_explain_ex(const char* s,int maxTick/*=5000*/)
{
	string ip;
	if(INADDR_NONE != inet_addr(s))
	{
		return s;//就是ip
	}
	else
	{
		IPFD *ipf = new IPFD();
		if(!ipf)
			return s;
		ipf->dns = s;
#ifdef _WIN32
		DWORD thid=0;
		HANDLE h = CreateThread(NULL,0,ip_explain_T,(void*)ipf,0,&thid);
		if(INVALID_HANDLE_VALUE==h)
			return s;
		else
			CloseHandle(h);
#else
		pthread_t hthread;
		if(0==pthread_create(&hthread,NULL,ip_explain_T,(void*)ipf))
			pthread_detach(hthread);
		else
			return s;
#endif
		Sleep(10);
		int i=maxTick/100;
		
		while(1)
		{
			if(ipf->is_set)
			{
				ip = ipf->ip;
				ipf->del = 1; 
				return ip;
			}
			if(i<=0)
				break;
			Sleep(100);
			i--;
		}
		ipf->del = 1;
		//return s;
		return "";
		//inet_addr("")=0,inet_addr(NULL)=-1,inet_addr("asdfs.s322.dassadf")=-1;
		//
	}
}

//****************************************************
bool Util::is_ip(const char* ip)
{
	unsigned int ip_n[4];
	if(ip && ip[0]!='\0' && 4==sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
		return true;
	return false;
}
bool Util::is_dev(const char* ip)
{
	if(ip && ip[0]!='\0' && !is_ip(ip))
		return true;
	return false;
}

//****************************************************

char* Util::ip_htoa(unsigned int ip)
{
	//非线程安全
	static char buf[32];
	unsigned char ip_n[4];
	ip_n[0] = ip >> 24;
	ip_n[1] = ip >> 16;
	ip_n[2] = ip >> 8;
	ip_n[3] = ip;
	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
	return buf;
}
char* Util::ip_ntoa(unsigned int nip)
{
	//非线程安全
	//inet_addr()使用同一块内存，如果调用一个函数里面有两个参数执行了ip_ntoa，则传入结果是同一个
	static char buf[32];
	unsigned char *ip_n = (unsigned char*)&nip;
	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
	return buf;
}
unsigned int Util::ip_atoh(const char* ip)
{
	//atonl:inet_addr("")=0,inet_addr(NULL)=-1,inet_addr("asdfs.s322.dassadf")=-1
	//sscanf返回：EOF=-1为错误，其它表示成功输入参数的个数,失败返回0或-1
	unsigned int ip_n[4]={0,0,0,0};
	if(4!=sscanf(ip,"%d.%d.%d.%d",&ip_n[0],&ip_n[1],&ip_n[2],&ip_n[3]))
		return 0;
	unsigned int iip;
	iip = ip_n[3];
	iip += (ip_n[2] << 8);
	iip += (ip_n[1] << 16);
	iip += (ip_n[0] << 24);
	return iip;
}

int Util::get_mtu()
{
	//取所有网卡中最小的mtu
#ifdef _WIN32
	PIP_ADAPTER_ADDRESSES pad = NULL;
	ULONG padlen = 0;
	DWORD mtu = 1500;
	DWORD ret = 0;
	GetAdaptersAddresses(AF_UNSPEC,0, NULL, pad,&padlen);
	pad = (PIP_ADAPTER_ADDRESSES) malloc(padlen);
	if(NO_ERROR==(ret=GetAdaptersAddresses(AF_INET,GAA_FLAG_SKIP_ANYCAST,0,pad,&padlen)))
	{
		mtu = pad->Mtu;
		PIP_ADAPTER_ADDRESSES p = pad->Next;
		while(p)
		{
			if(p->Mtu>0 && mtu > p->Mtu)
				mtu = p->Mtu;
			p = p->Next;
		};
	}

	free(pad);
	if(mtu<=0) mtu = 1500;
	return (int)mtu;
#else
	struct ifreq *ifr;
	struct ifconf conf;
	int fd,mtu,n,i;

	conf.ifc_len = 0;
	conf.ifc_buf = NULL;
	mtu = 1500;

	if((fd = socket(AF_INET,SOCK_DGRAM,0))<=0)
		goto fail;

	if(0!=ioctl(fd,SIOCGIFCONF,&conf))
		goto fail;
	conf.ifc_buf = (char*)malloc(conf.ifc_len);
	if(0!=ioctl(fd,SIOCGIFCONF,&conf))
		goto fail;
	
	n = conf.ifc_len / sizeof(struct ifreq);
	//printf("ifc_len = %d, n = %d, sizeof(ifreq)=%d \n",conf.ifc_len,n,sizeof(struct ifreq));
	if(n>0)
	{
		ifr = conf.ifc_req;
		if(0==ioctl(fd, SIOCGIFMTU, (void*)ifr))
		{
			mtu = ifr->ifr_mtu;
			//printf("ifr: %s , mtu=%d \n",ifr->ifr_name,ifr->ifr_mtu);
			for(i=1;i<n;++i)
			{
				ifr++;
				if(0==ioctl(fd, SIOCGIFMTU, (void*)ifr))
				{
					if(ifr->ifr_mtu>0 && mtu>ifr->ifr_mtu)
						mtu = ifr->ifr_mtu;
					//printf("ifr: %s , mtu=%d \n",ifr->ifr_name,ifr->ifr_mtu);
				}
			}
		}
	}

fail:
	if(fd>0)
		close(fd);
	if(conf.ifc_buf)
		free(conf.ifc_buf);
	if(mtu<=0) mtu = 1500;
	return mtu;

#endif
}

//*******************************************************
bool Util::is_write_debug_log()
{
	static int s_writelog = 2;
	if(0==s_writelog)
	{
		return false;
	}
	else if(2==s_writelog)
	{
		FILE *fp = fopen("./debug.log","rb");
		if(fp)
		{
			fclose(fp);
			s_writelog = 1;
		}
		else
		{
			s_writelog = 0;
			return false;
		}
	}
	return true;
}
int Util::write_debug_log(const char *strline,const char *path)
{
	if(!is_write_debug_log())
		return -1;
	return write_log(strline,path);
}
int Util::write_log(const char *strline,const char *path)
{
	char *buf=new char[strlen(strline)+128];
	if(!buf)
		return -1;
	time_t tt = time(0);
	tm *t = localtime(&tt);
	sprintf(buf,"[%d-%d-%d %02d:%02d:%02d] %s \r\n",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec,strline);

	FILE *fp = fopen(path,"ab+");
	if(fp)
	{
		fwrite(buf,strlen(buf),1,fp);
		fclose(fp);
	}
	delete[] buf;
	return 0;
}

}
