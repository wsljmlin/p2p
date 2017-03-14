#pragma once

typedef struct tag_ubn_serveri
{
	unsigned int ip;
	char name[64];
}ubn_serveri_t;
typedef struct tag_ubn_server
{
	int count;
	ubn_serveri_t server[64];
}ubn_server_t;

typedef void* UBN_SERVER_HANDLE;

//ubnname 不能超过64
UBN_SERVER_HANDLE ubn_server_open(unsigned short port,const char* ubnname,bool bthreadroot);
void ubn_server_root(UBN_SERVER_HANDLE ubnh);
int ubn_server_close(UBN_SERVER_HANDLE ubnh);

//count 指定最多查找个数，返回实际查找个数,墨认最多查5秒超时
//ubnname 为NULL时不过滤结果
int ubn_find_server(unsigned short port,const char* ubnname,ubn_server_t* us,unsigned int timeo=3000); 

