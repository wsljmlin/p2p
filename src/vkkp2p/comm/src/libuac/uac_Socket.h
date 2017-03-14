#pragma once
#include "uac.h"


#define UAC_READ 0x01
#define UAC_WRITE 0x02

enum { UAC_DISCONNECTED,UAC_CONNECTING,UAC_CONNECTED };


//UAC_Socket Ϊ ����ģʽsocket����Ҫ���زſ�����
class UAC_Socket
{
public:
	UAC_Socket(void);
	virtual ~UAC_Socket(void);

public:
	int uac_attach(UAC_SOCKET uac_fd,const UAC_sockaddr& uac_addr);
	int uac_connect(unsigned int ip,unsigned short port,int nattype=0); //Ĭ��ֱ��
	int uac_disconnect();
	int uac_send(const char* buf,int size);
	int uac_recv(char* buf,int size);
	
	int uac_setsendbuf(int size);
	int uac_setrecvbuf(int size);

	UAC_SOCKET uac_get_fd()const {return m_uac_fd;}
	int uac_get_state()const {return m_uac_state;}
	bool uac_is_write()const {return ::uac_is_write(m_uac_fd);}

	//UAC_SocketSelector��Ӧ����
	virtual int uac_on_read(){return 1;} //����1��ʾ�´��ٽ���(����)
	virtual void uac_on_write();
	virtual void uac_on_connected(); //����֪ͨ�ϲ�
protected:
	UAC_SOCKET			m_uac_fd;
	UAC_sockaddr		m_uac_addr;
	int					m_uac_state;
	bool				m_uac_blastsend,m_uac_bregsend;
};
