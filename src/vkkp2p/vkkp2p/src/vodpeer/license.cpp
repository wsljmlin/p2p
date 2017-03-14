#include "license.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "md5.h"
#include "basetypes.h"

#include "Util.h"
#include "Httpc.h"

extern bool g_is_exit;

#ifdef _WIN32
#define LICENSE_ID "windows"
#define LICENSE_KEY "184589045vaMDyAnCgsyNzY4MjAzMA=="
#elif defined _ANDROID
#define LICENSE_ID "android"
#define LICENSE_KEY "542983943kv5qruLtkYyNTk1MTE2Nw=="
#else
#define LICENSE_ID "linux"
#define LICENSE_KEY "59369668+nioFImUQLI3NTIwMjU3Mg=="
#endif

bool check_license_ok()
{
	printf("# check license...\n");
	int rnd = 0;
	char buf[256];
	char md5key[64];
	char rsp_md5key[64];
	memset(md5key,0,64);
	memset(rsp_md5key,0,64);
	srand((unsigned int)time(NULL));
	rnd = rand()+10000;

// step 1, get token from server
//
	char sret[10240];
	int res = Httpc::http_get( "http://media.tvata.com/services/license/token.php", sret, 10240 );
	if ( res == -1 ) {
		printf("can not get token!\n");
		return false;
	}

	char *p = sret;
	while( *p != '\0' ) {
		if ( *p == '\n' || *p == '\r' || *p == ' ' ) {
			*p = 0;
			break;
		}
		p++;
	}

	string token = sret;
	//printf("get token %s!\n", token.c_str());

	string mac = Util::get_mac();

	sprintf(buf,"%s,%d", token.c_str(), rnd);
	md5_str_sum(buf,md5key,64);
	//printf("r %d line %s md5 %s\n", rnd, buf, md5key );

	sprintf(buf,"http://media.tvata.com/services/license/chk.php?t=%s&r=%d&c=%s&m=%s", LICENSE_ID, rnd, md5key, mac.c_str());

	res = Httpc::http_get( buf, sret, 10240 );

	if ( res == -1 ) {
		printf("http response error!\n");
		return false;
	}

	//printf("send request %s\nget resp %s\n\n", buf, sret);

	//sprintf(buf,"%s,%d,%s,%s",LICENSE_ID,rnd,LICENSE_KEY,mac);
	sprintf(buf,"%s,%d,%s",LICENSE_ID,rnd,LICENSE_KEY);
	md5_str_sum(buf,md5key,64);
	//printf("# md5key:%s \n",md5key);
	//printf("read md5key: %s \n",rsp_md5key);
	//printf("send request %s\nget resp %s\nchksum %s\n", buf, sret, md5key);

	if ( strstr(sret, md5key ) == NULL )
		return false;
	printf("# license check ok \n" );
	return true;
}

int License::run()
{
	if(m_brun)
		return 0;
	m_brun = true;
	this->activate(1);
	return 0;
}
void License::end()
{
	if(m_brun)
	{
		m_brun = false;
		wait();
	}
}
int License::work(int e)
{
	bool ok = false;
	for(int i=0;i<5&&m_brun;++i)
	{
		if(check_license_ok())
		{
			ok = true;
			break;
		}
		Sleep(1000);
	}
	if(!ok)
	{
		printf("#*** check license failed \n");
		g_is_exit = true;
	}
	return 0;
}
