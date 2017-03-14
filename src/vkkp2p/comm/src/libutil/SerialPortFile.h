#pragma once

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
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
	// 枚举所有可用端口
	static int enum_port(SerialDevice_t* psd); //count 指定最大枚举数，返回实际个数
	// 检查com口是否可用， szDevice = "COM1"
	static EPort check_port(char* szDevice);
	
public:

	//-1错误，0成功，1已经打开
	int open(const char* szDevice,int BaudRate=CBR_9600,unsigned char ByteSize=8,unsigned char StopBits=ONESTOPBIT,unsigned char Parity=NOPARITY
		,unsigned int to_RInterval=300,unsigned int to_RTMultiplier=20,unsigned int to_RTConstant=500,unsigned int to_WTMultiplier=50,unsigned int to_WTConstant=500);
	void close();
	bool is_open() const 
	{ 
		return (INVALID_HANDLE_VALUE!=m_hFile);
	}

	int read(char* buf,int size);
	int write(const char* buf,int size);

	bool kill_read(); //多线程时,此调用让读的线程立即停止并返回，同时清空缓冲
	bool kill_write();
	bool clear_io();
private:
	int m_errno;
	HANDLE m_hFile;
};

#endif

