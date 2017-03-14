#pragma once

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#include <windows.h>

// Port availability
typedef enum
{
	EPortUnknownError = -1,		// Unknown error occurred
	EPortAvailable    =  0,		// Port is available
	EPortNotAvailable =  1,		// Port is not present
	EPortInUse        =  2		// Port is in use

} EPort;

typedef struct tagSerialDevice
{
	int count;
	char paths[100][10];
}SerialDevice_t;


class SerialPortFile
{
public:
	SerialPortFile(void);
	~SerialPortFile(void);
	
public:
	// ö�����п��ö˿�
	static int enum_port(SerialDevice_t* psd); //count ָ�����ö����������ʵ�ʸ���
	// ���com���Ƿ���ã� szDevice = "COM1"
	static EPort check_port(char* szDevice);
	
public:

	//-1����0�ɹ���1�Ѿ���
	int open(const char* szDevice,int BaudRate=CBR_9600,unsigned char ByteSize=8,unsigned char StopBits=ONESTOPBIT,unsigned char Parity=NOPARITY
		,unsigned int to_RInterval=300,unsigned int to_RTMultiplier=20,unsigned int to_RTConstant=500,unsigned int to_WTMultiplier=50,unsigned int to_WTConstant=500);
	void close();
	bool is_open() const 
	{ 
		return (INVALID_HANDLE_VALUE!=m_hFile);
	}

	int read(char* buf,int size);
	int write(const char* buf,int size);

	bool kill_read(); //���߳�ʱ,�˵����ö����߳�����ֹͣ�����أ�ͬʱ��ջ���
	bool kill_write();
	bool clear_io();
private:
	int m_errno;
	HANDLE m_hFile;
};

#endif

