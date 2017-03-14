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
	EnumPortsA(NULL,2,pBite,0,&pcbNeeded,&pcReturned); //��ִ�з���false�����õ���С
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

		//1.���ô���״̬
		DCB dcb;
		if(!GetCommState(m_hFile,&dcb))
		{
			throw(2);
		}
		//
		dcb.BaudRate = BaudRate; //CBR_9600
		dcb.ByteSize = ByteSize;
		dcb.StopBits = StopBits; //ONESTOPBIT=0(1λ),ONE5STOPBITS=1(1.5λ),TWOSTOPBITS=2(2λ)
		dcb.Parity = Parity;//NOPARITY=0(��У��), ODDPARITY=1 ��У��,EVENPARITY=2 żУ��,MARKPARITY=3 ���У�� ; 
		dcb.fParity = Parity?1:0; //У��ʹ��λ��1��ʾ֧�ּ���
		if(!SetCommState(m_hFile,&dcb))
		{
			throw(3);
		}

		//2.���ý��ջ�����,1024 Byte
		if(!SetupComm(m_hFile,1024,1024))
		{
			throw(4);
		}

		//3.���ó�ʱ
		COMMTIMEOUTS cto;
		if(!GetCommTimeouts(m_hFile,&cto))
		{
			throw(5);
		}
		printf("open path=%s,rin=%d,rtm=%d,rtc=%d,wtm=%d,wtc=%d \n",szDevice,to_RInterval,to_RTMultiplier,to_RTConstant,to_WTMultiplier,to_WTConstant);
		cto.ReadIntervalTimeout = to_RInterval; //����2�ֽ�֮������������һ���ֽڶ�û���յ����ò������
		cto.ReadTotalTimeoutMultiplier = to_RTMultiplier; //��ÿ���ֽ�ƽ����ʱϵͳ,
		cto.ReadTotalTimeoutConstant = to_RTConstant; //�ܳ�ʱ����
		//д��ʱ:��дn�ֽڣ���ʱΪn*50+1000,10�ֽ�Ϊ1500���볬ʱ
		cto.WriteTotalTimeoutMultiplier = to_WTMultiplier; //д��ʱϵͳ
		cto.WriteTotalTimeoutMultiplier = to_WTConstant; //����
		if(!SetCommTimeouts(m_hFile,&cto))
		{
			throw(6);
		}

		//4.��ջ�����
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
		PurgeComm(m_hFile,PURGE_TXABORT|PURGE_RXABORT); //�ж����ж�/д�������������أ���ʹ��/д������û����ɡ�
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
bool SerialPortFile::kill_read() //���߳�ʱ,�˵����ö����߳�����ֹͣ�����أ�ͬʱ��ջ���
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
