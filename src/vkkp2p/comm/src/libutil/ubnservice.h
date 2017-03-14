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

//ubnname ���ܳ���64
UBN_SERVER_HANDLE ubn_server_open(unsigned short port,const char* ubnname,bool bthreadroot);
void ubn_server_root(UBN_SERVER_HANDLE ubnh);
int ubn_server_close(UBN_SERVER_HANDLE ubnh);

//count ָ�������Ҹ���������ʵ�ʲ��Ҹ���,ī������5�볬ʱ
//ubnname ΪNULLʱ�����˽��
int ubn_find_server(unsigned short port,const char* ubnname,ubn_server_t* us,unsigned int timeo=3000); 

