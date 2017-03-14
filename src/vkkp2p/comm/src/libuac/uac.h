#pragma once

//���֧��������
#define UAC_FD_SIZE 2048
#define UAC_SOCK_RECVBUF 204800
#define UAC_SOCK_SENDBUF 204800

typedef int UAC_SOCKET;
typedef void (*UAC_CALLBACK_ONNATOK)(int ); //(int nattype)
typedef void (*UAC_CALLBACK_ONIPPORTCHANGED)(unsigned int ,unsigned short ); //(unsigned int ip,unsigned short port)

#ifdef __cplusplus
extern "C"
{
#endif

//UAC_sockaddrʹ�������򣬲�ʹ��������
typedef struct tag_UAC_sockaddr
{
	unsigned int	ip;
	unsigned short	port;
	char			nattype; //0~6
}UAC_sockaddr;

//**********************************
//nattype(0~5): nat���Ϳ�����ͨ��ƥ��
//
//			nat0	nat1	nat2	nat3	nat4
// nat0		1		1		1		1		1
// nat1		1	   	1		1		1		1
// nat2		1		1		1		1		1
// nat3		1		1		1		1		0
// nat4		1		1		1		0		0
//
//**********************************


typedef struct tag_UAC_fd_set
{
	int				fd_count;
	UAC_SOCKET		fd_array[UAC_FD_SIZE];
}UAC_fd_set;

//setting
void uac_setcallback_onnatok(UAC_CALLBACK_ONNATOK fun);
void uac_setcallback_onipportchanged(UAC_CALLBACK_ONIPPORTCHANGED fun);
//get
int uac_get_nattype();
//UAC ֻ��һ������˿ڣ����������ڴ˶˿ڻ�����ģ�����
int uac_init(unsigned short bindport,const char* stunsvr,unsigned short stunport); //ָ��UDP�˿�
int uac_fini();

//UAC socket �ӿ�
UAC_SOCKET uac_accept(UAC_sockaddr* sa_client);
UAC_SOCKET uac_connect(const UAC_sockaddr* sa_client);
int uac_closesocket(UAC_SOCKET fd);
int uac_setsendbuf(UAC_SOCKET fd,int size);
int uac_setrecvbuf(UAC_SOCKET fd,int size);
bool uac_is_read(UAC_SOCKET fd);
bool uac_is_write(UAC_SOCKET fd);
int uac_select(UAC_fd_set* rset,UAC_fd_set* wset);
int uac_send(UAC_SOCKET fd,const char* buf,int len);//����:-1 �ر�
int uac_recv(UAC_SOCKET fd,char* buf,int len); //����:-1 �ر�


#ifdef __cplusplus
}
#endif

