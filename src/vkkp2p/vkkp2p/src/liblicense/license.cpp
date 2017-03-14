#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <stdlib.h>
#include <string.h>
//#include <cstdlib>
#include "mini_md5.h"
#include "random.h"
#include "license.h"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
	// 4482 4267 4018 4800 4311 4312 4102
	#pragma warning(disable:4996)
#endif



/*
    buf:
    
    ranstring ..... key value..........ranstring.....offset
                     40 BYTE
*/
int CmnLicense::GenerateLicense(const char* ip, const char* enddate, char** pbufout, int* buflen)
{
    if(strlen(enddate) != 8)
        return -1;
        
    char bufout[16], randbuf[16];
    miniutil::Random::rand_stringbuf16(inet_addr(ip),randbuf);
    //cmutil_genranstring(inet_addr(ip.c_str()),randbuf);
 
    miniutil::MD5 amd5;
    amd5.update((unsigned char*)ip, (int)strlen(ip));
    amd5.update((unsigned char*)randbuf, 16);
    amd5.finalize();
    memcpy(bufout, amd5.raw_digest(), 16);
	
	int len = 16+16+4+1 + 160;
	char *pbuf = new char[len];
	memset(pbuf, 0x00, len);
	
	int beforerannum = (int)(((rand()-1)/(double)RAND_MAX)*10);
	int offset = 0;
	for(int i=0; i< beforerannum; i++)
	{
        miniutil::Random::rand_stringbuf16(rand(),pbuf+offset);
        //cmutil_genranstring(rand(),pbuf+offset);
        offset += 16;
	}
	
	memcpy(pbuf + offset, bufout, 16);
	offset += 16;
	memcpy(pbuf+ offset, randbuf, 16);
	offset += 16;
	
	int datevalue = htonl(atoi(enddate));
    //memcpy(pbuf, enddate.c_str(), 8);
    *((int*)(pbuf + offset)) = datevalue;
    
    offset += 4;
    
    for(int i=0; i< 10 - beforerannum; i++)
    {
        miniutil::Random::rand_stringbuf16(rand(),pbuf+offset);
        //cmutil_genranstring(rand(),pbuf+offset);
        offset += 16;
   }
   
   pbuf[len-1] = (char)(beforerannum&0xFF);

   *pbufout = pbuf;
   *buflen = len;
   return 0;
 
}


bool CmnLicense::CheckLicense(const char* ip, const char* enddate, char* buf, int buflen)
{
    int len = 16+16+4+1 + 160;
    if(buflen != len)
        return false;
       
    int offset = (unsigned int)(buf[len-1]);
    char *pbufout, *prandbuf;
    int datevalue;
    
    pbufout = buf+offset*16;
    prandbuf = buf+offset*16+16;
    datevalue = ntohl((*(int*)(buf+offset*16+32)));
    
    if(atoi(enddate) > datevalue)
        return false;
        
	char buf_out[16];

    miniutil::MD5 amd5;
    amd5.update((unsigned char*)ip, (int)strlen(ip));
    amd5.update((unsigned char*)prandbuf, 16);
    amd5.finalize();
    memcpy(buf_out, (char*)amd5.raw_digest(), 16);
        
	if(memcmp(buf_out, pbufout, 16))
	{
		//unlimited IP check
		char unlimitedip[32];
		strcpy(unlimitedip,"0.0.0.0");

        miniutil::MD5 bmd5;
        bmd5.update((unsigned char*)unlimitedip,(int)strlen(unlimitedip));
        bmd5.update((unsigned char*)prandbuf, 16);
        bmd5.finalize();
        memcpy(buf_out, (char*)bmd5.raw_digest(), 16);

		if(memcmp(buf_out, pbufout, 16))
			return false;
	}
        
    return true;
       
}

int CmnLicense::GenerateLicenseFile(const char* ip, const char* enddate, const char* filename)
{
	FILE* file = fopen(filename, "wb");
	if(!file)
		return -1;

	char *buf;
	int len;
	int ret = GenerateLicense(ip, enddate, &buf, &len);
	if(ret != 0)
		return -1;

	fwrite(buf, len, 1, file);
	fclose(file);

	delete buf;
	return 0;
}

bool CmnLicense::CheckLicenseFile(const char* ip, const char* enddate, const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if(!file)
		return false;

	char *buf;
	int len;

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	buf = new char[len];
	fseek(file, 0,SEEK_SET);
	fread(buf, len, 1, file);

	bool ret = CheckLicense(ip, enddate, buf, len);

	delete buf;
	return ret;
}
