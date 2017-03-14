#include "SerialPortFile.h"

#ifdef _WIN32
#include <winspool.h>
#include <stdio.h>
#pragma warning(disable:4996)


SerialPortFile::SerialPortFile(void)
: m_errno(0)
, m_hFile(INVALID_HANDLE_VALUE)
{
}

SerialPortFile::~SerialPortFile(void)
{
	close();
}

int SerialPortFile::enum_port(SerialDevice_t* psd)
{
	if(!psd || psd->count<=0) return 0;
	int count = psd->count;
	if(count>100) count=100;
	LPBYTE pBite = NULL;
	DWORD pcbNeeded = 0;
	DWORD pcReturned = 0;
	PORT_INFO_2A *pPort;

	psd->count = 0;
	EnumPortsA(NULL,2,pBite,0,&pcbNeeded,&pcReturned); //此执行返回false，但得到大小
	if(pcbNeeded>0)
	{
		pBite = new BYTE[pcbNeeded];
		if(EnumPortsA(NULL,2,pBite,pcbNeeded,&pcbNeeded,&pcReturned))
		{
			pPort = (PORT_INFO_2A*)pBite;
			for(int i=0;i<(int)pcReturned;++i)
			{
				if(pPort[i].pPortName == strstr(pPort[i].pPortName,"COM") && strlen(pPort[i].pPortName)<10)
				{
					strcpy(psd->paths[psd->count++],pPort[i].pPortName);
				}
			}
		}
		delete[] pBite;
	}
	return psd->count;
}
EPort SerialPortFile::check_port(char* szDevice)
{
	HANDLE hFile = ::CreateFileA(szDevice, 
						   GENERIC_READ|GENERIC_WRITE, 
						   0, 
						   0, 
						   OPEN_EXISTING, 
						   0,
						   0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		switch (::GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:
			// The specified COM-port does not exist
			return EPortNotAvailable;
		case ERROR_ACCESS_DENIED:
			// The specified COM-port is in use
			return EPortInUse;
		default:
			// Something else is wrong
			return EPortUnknownError;
		}
	}
	::CloseHandle(hFile);
	// Port is available
	return EPortAvailable;
}


//*********************************************************************
int SerialPortFile::open(const char* szDevice,int BaudRate/*=CBR_9600*/,unsigned char ByteSize/*=8*/,unsigned char StopBits/*=ONESTOPBIT*/,unsigned char Parity/*=NOPARITY*/
		,unsigned int to_RInterval/*=300*/,unsigned int to_RTMultiplier/*=20*/,unsigned int to_RTConstant/*=500*/,unsigned int to_WTMultiplier/*=50*/,unsigned int to_WTConstant/*=500*/)
{
	m_errno = 0;
	if(INVALID_HANDLE_VALUE!=m_hFile)
		return 1;
	try
	{
		m_hFile = CreateFileA(szDevice, 
							   GENERIC_READ|GENERIC_WRITE, 
							   0, 
							   0, 
							   OPEN_EXISTING, 
							   0,
							   0);
		if(INVALID_HANDLE_VALUE==m_hFile)
		{
			throw(1);
		}

		//1.设置串口状态
		DCB dcb;
		if(!GetCommState(m_hFile,&dcb))
		{
			throw(2);
		}
		//
		dcb.BaudRate = BaudRate; //CBR_9600
		dcb.ByteSize = ByteSize;
		dcb.StopBits = StopBits; //ONESTOPBIT=0(1位),ONE5STOPBITS=1(1.5位),TWOSTOPBITS=2(2位)
		dcb.Parity = Parity;//NOPARITY=0(无校验), ODDPARITY=1 奇校验,EVENPARITY=2 偶校验,MARKPARITY=3 标记校验 ; 
		dcb.fParity = Parity?1:0; //校验使能位，1表示支持检验
		if(!SetCommState(m_hFile,&dcb))
		{
			throw(3);
		}

		//2.设置接收缓冲区,1024 Byte
		if(!SetupComm(m_hFile,1024,1024))
		{
			throw(4);
		}

		//3.设置超时
		COMMTIMEOUTS cto;
		if(!GetCommTimeouts(m_hFile,&cto))
		{
			throw(5);
		}
		printf("open path=%s,rin=%d,rtm=%d,rtc=%d,wtm=%d,wtc=%d \n",szDevice,to_RInterval,to_RTMultiplier,to_RTConstant,to_WTMultiplier,to_WTConstant);
		cto.ReadIntervalTimeout = to_RInterval; //接收2字节之间最大间隔，如果一个字节都没有收到就用不到这个
		cto.ReadTotalTimeoutMultiplier = to_RTMultiplier; //读每个字节平均超时系统,
		cto.ReadTotalTimeoutConstant = to_RTConstant; //总超时常量
		//写超时:如写n字节，则超时为n*50+1000,10字节为1500毫秒超时
		cto.WriteTotalTimeoutMultiplier = to_WTMultiplier; //写超时系统
		cto.WriteTotalTimeoutMultiplier = to_WTConstant; //常量
		if(!SetCommTimeouts(m_hFile,&cto))
		{
			throw(6);
		}

		//4.清空缓冲区
		if(!PurgeComm(m_hFile,PURGE_TXCLEAR|PURGE_RXCLEAR))
		{
			throw(7);
		}

	}catch(...)
	{
		m_errno = GetLastError();
		if(0==m_errno) m_errno=-1;
		return -1;
	}
	return 0;
}
void SerialPortFile::close()
{
	m_errno = 0;
	if(INVALID_HANDLE_VALUE!=m_hFile)
	{
		PurgeComm(m_hFile,PURGE_TXABORT|PURGE_RXABORT); //中断所有读/写操作并立即返回，即使读/写操作还没有完成。
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

int SerialPortFile::read(char* buf,int size)
{
	if(INVALID_HANDLE_VALUE==m_hFile) return -1;
	DWORD realsize = 0;
	if(ReadFile(m_hFile,buf,size,&realsize,NULL))
		return realsize;
	//GetLastError()
	return -1;
}
int SerialPortFile::write(const char* buf,int size)
{
	if(INVALID_HANDLE_VALUE==m_hFile) return -1;
	DWORD realsize = 0;
	if(WriteFile(m_hFile,buf,size,&realsize,NULL))
		return realsize;
	//GetLastError()
	return -1;
}
bool SerialPortFile::kill_read() //多线程时,此调用让读的线程立即停止并返回，同时清空缓冲
{
	if(INVALID_HANDLE_VALUE==m_hFile) return false;
	if(PurgeComm(m_hFile,PURGE_RXABORT|PURGE_RXCLEAR))
		return true;
	return false;
}
bool SerialPortFile::kill_write()
{
	if(INVALID_HANDLE_VALUE==m_hFile) return false;
	if(PurgeComm(m_hFile,PURGE_TXABORT|PURGE_TXCLEAR))
		return true;
	return false;
}
bool SerialPortFile::clear_io()
{
	if(INVALID_HANDLE_VALUE==m_hFile) return false;
	if(PurgeComm(m_hFile,PURGE_RXCLEAR|PURGE_TXCLEAR))
		return true;
	return false;
}

#endif
