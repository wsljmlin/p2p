#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "md5.h"

#ifdef _WIN32
#include <windows.h>
#define sleep(i) Sleep(i*1000)
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
#pragma warning(disable:4996)
#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define LICENSE_ID "2011092395"
#define LICENSE_KEY "184589045vaMDyAnCgsyNzY4MjAzMA=="
#define LICENSE_SUFFIX ""
#elif defined _LICENSE_C2
#define LICENSE_ID "2011092393"
#define LICENSE_KEY "82388215q8btccyDiKoyMTgxMDA4NA=="
#define LICENSE_SUFFIX "_c2"
#elif defined _LICENSE_ANDROID
#define LICENSE_ID "2011092391"
#define LICENSE_KEY "542983943kv5qruLtkYyNTk1MTE2Nw=="
#define LICENSE_SUFFIX "_android"
#elif defined _LICENSE_TELECHIPS
#define LICENSE_ID "2011092392"
#define LICENSE_KEY "23377969QisWjgI80ec3OTMzMTYyNQ=="
#define LICENSE_SUFFIX "_telechips"
#else
#define LICENSE_ID "2011092394"
#define LICENSE_KEY "59369668+nioFImUQLI3NTIwMjU3Mg=="
#define LICENSE_SUFFIX ""
#endif


//g++ -o license_p2p license_p2p.cpp md5.cpp
int main(int argc,char** argv)
{
	sleep(2);
	if(argc<2)
	{
		printf("***no rnd!\n");
		return 0;
	}
	char buf[128];
	char md5key[64];
	memset(md5key,0,64);
	sprintf(buf,"%s,%s,%s",LICENSE_ID,argv[1],LICENSE_KEY);
	md5_str_sum(buf,md5key,64);
	printf("%s",md5key);
	return 0;
}



