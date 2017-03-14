#include "DIPCache.h"

namespace DIPCache
{
	typedef struct tagDIPInfo
	{
		string domain;
		string ip;
		unsigned int beg_tick;
		string ips[10]; // i am not sure if dynamic array is ok for struct, it is too C-style!
		int    size;
		int    count;
	}DIPInfo_t;
	typedef list<DIPInfo_t> DIPList;
	typedef DIPList::iterator DIPListIter;
	DIPList g_dips;

	DIPInfo_t* findinfo(const char* domain)
	{
		for(DIPListIter it=g_dips.begin();it!=g_dips.end();++it)
		{
			if((*it).domain == domain)
				return &(*it);
		}
		return NULL;
	}
	string findip(const char* domain)
	{
		DIPInfo_t* inf=NULL;
		if((inf=findinfo(domain))) 
		{
			if ( inf->size <= 1 )
				return inf->ip;
			int i = (inf->count++) % inf->size;
			printf("# DIPCache findip ( %s : %d %s ) \n",domain, i, inf->ips[i].c_str() );
			return inf->ips[i];
		}
		return "";
	}
	void addip(const char* domain,const string& iplist)
	{
		if(iplist.empty() || NULL==domain)
			return;

		string ip = iplist;
		string ips[10];
		
		char * start = (char *) ip.c_str();
		char * p;
		int size = 0;
		while( size < 10 && (p=(char *)strstr(start, ",")) != NULL ) 
		{
			if ( p - start > 7 )
			{
				*p = 0;
				ips[size++] = start;
				printf("# DIPCache addip list ( %s : %d %s ) \n",domain, size-1, start );
			}
			start = p+1;
		}
		if ( size > 0 )
			ip = ips[0];
		
		DIPInfo_t* inf=NULL;
		if((inf=findinfo(domain)))
		{
			if(inf->ip != ip)
			{
				inf->ip = ip;
				inf->size = size;
				for(int i=0;i<size;i++)
					inf->ips[i] = ips[i];
				inf->count =0;
				inf->beg_tick = GetTickCount();
			}
			return;
		}
		DIPInfo_t inf2;
		inf2.beg_tick = GetTickCount();
		inf2.domain = domain;
		inf2.ip = ip;
		inf2.size = size;
		for(int i=0;i<size;i++)
			inf2.ips[i] = ips[i];
		inf2.count = 0;
		g_dips.push_back(inf2);
		printf("# DIPCache cache( %s : %s ) \n",domain,ip.c_str());
	}

	void load_hosts_file(const string &hosts_file )
	{
		if ( hosts_file.empty() == true )
			return;

		FILE * hfile = fopen( hosts_file.c_str(), "r" );
		if ( hfile == NULL )
			return;
		char domain[1024];
		while( fgets(domain, 1024, hfile) != NULL )
		{
			char *p = strstr(domain, ":");
			if ( p != NULL )
			{
				*p = 0;
				char *iplist = p+1;
				printf("DIPCache init: %s ---> %s\n", domain, iplist );
				addip(domain, iplist);
			}
		}
		fclose( hfile);
	}

}

