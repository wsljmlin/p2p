#include "test_util.h"
#include "Util.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#include <windows.h>
#include <Iphlpapi.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

int get_allmac(ifinfo_s *inf)
{
#ifdef _WIN32
	int i = 0;
	int n = inf->ifcount;
	inf->ifcount = 0;
	IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information 
	DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer
	DWORD dwStatus = GetAdaptersInfo(AdapterInfo,&dwBufLen);
	if(dwStatus != ERROR_SUCCESS)
		return -1;
	//memcpy(umac,AdapterInfo->Address,6);
	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
	do
	{
		strcpy(inf->ifs[i].name,pAdapterInfo->AdapterName);
		sprintf(inf->ifs[i].mac,"%02X%02X%02X%02X%02X%02X", 
			pAdapterInfo->Address[0], 
			pAdapterInfo->Address[1], 
			pAdapterInfo->Address[2], 
			pAdapterInfo->Address[3], 
			pAdapterInfo->Address[4], 
			pAdapterInfo->Address[5]); 
		inf->ifs[i].isup = true;
		if(MIB_IF_TYPE_LOOPBACK==AdapterInfo->Type)
			inf->ifs[i].isloopback = true;
		else
			inf->ifs[i].isloopback = false;
		strcpy(inf->ifs[i].ip,pAdapterInfo->IpAddressList.IpAddress.String);


		i = ++inf->ifcount;
		if(inf->ifcount>=n) break;
	}while(NULL!=(pAdapterInfo=pAdapterInfo->Next));
	return inf->ifcount;
#else
	int fd;
	struct ifconf conf;
	struct ifreq *ifr;
	sockaddr_in *sin;
	int i;
	char buff[2048];

	struct ifreq ifr2;
	unsigned char* umac;

	conf.ifc_len = 2048;
	conf.ifc_buf = buff;
	fd = socket(AF_INET,SOCK_DGRAM,0);
	ioctl(fd,SIOCGIFCONF,&conf);

	inf->ifcount = conf.ifc_len / sizeof(struct ifreq);
	ifr = conf.ifc_req;
	if(inf->ifcount>10) inf->ifcount = 10;
	for(i=0;i < inf->ifcount;++i)
	{
		sin = (struct sockaddr_in*)(&ifr->ifr_addr);
		ioctl(fd,SIOCGIFFLAGS,ifr);
		strcpy(inf->ifs[i].name,ifr->ifr_name);
		strcpy(inf->ifs[i].ip,inet_ntoa(sin->sin_addr));
		if(ifr->ifr_flags & IFF_LOOPBACK) 
			inf->ifs[i].isloopback = true;
		else
			inf->ifs[i].isloopback = false;
		if(ifr->ifr_flags & IFF_UP)
			inf->ifs[i].isup = true;
		else
			inf->ifs[i].isup = false;
		ifr++;
	}

	for(i=0;i < inf->ifcount;++i)
	{
		memset(inf->ifs[i].mac,0,16);
		memset(&ifr2,0,sizeof(ifr2));
		strcpy(ifr2.ifr_name,inf->ifs[i].name);
		if(0==ioctl(fd,SIOCGIFHWADDR,&ifr2,sizeof(struct ifreq)))
		{
			umac = (unsigned char*)ifr2.ifr_hwaddr.sa_data;
			sprintf(inf->ifs[i].mac,"%02X%02X%02X%02X%02X%02X", umac[0], umac[1], umac[2], umac[3], umac[4], umac[5]);
		}
	}
	close(fd);
	return inf->ifcount;
#endif
}


int test_util_main(int argc,char** argv)
{
	ifinfo_s inf;
	inf.ifcount = 10;
	Util::get_macall(&inf);
	for(int i=0;i<inf.ifcount;++i)
	{
		printf("%d -- %s (%s)(%s) -- %s %s \n",i,inf.ifs[i].mask,inf.ifs[i].ip,inf.ifs[i].mac,inf.ifs[i].isup?"up":"down",inf.ifs[i].isloopback?"[loopback]":"");
	}
	return 0;
}


