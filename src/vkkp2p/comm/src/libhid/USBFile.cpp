
#include "USBFile.h"
#include <assert.h>
#pragma warning(disable:4996)

USBFile::USBFile(void)
	:m_h(INVALID_HANDLE_VALUE)
	,m_packetlen(0)
	,m_reportid(0)
{
}


USBFile::~USBFile(void)
{
	close();
}

int USBFile::open(const char* path,int packetlen,unsigned char reportid)
{
	if(packetlen<2)
		return -1;
	close();
	m_h = CreateFileA(
		path,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, 
		OPEN_EXISTING,/*0*/FILE_FLAG_OVERLAPPED, 
		NULL);
	if(INVALID_HANDLE_VALUE==m_h)
		return -1;
	m_packetlen = packetlen;
	m_reportid = reportid;
	m_sendbuf[0] = m_reportid;


	//m_read_overlapped.hEvent = CreateEventA(NULL,TRUE,TRUE,"");
	//m_write_overlapped.hEvent = CreateEventA(NULL,TRUE,TRUE,"");
	m_read_overlapped.hEvent = CreateEventA(NULL,FALSE,FALSE,"");
	m_write_overlapped.hEvent = CreateEventA(NULL,FALSE,FALSE,"");

	m_read_overlapped.Offset = 0;
	m_read_overlapped.OffsetHigh = 0;
	m_write_overlapped.Offset = 0;
	m_write_overlapped.OffsetHigh = 0;
	return 0;
}
void USBFile::close()
{
	if(INVALID_HANDLE_VALUE!=m_h)
	{
		CloseHandle(m_h);
		m_h = INVALID_HANDLE_VALUE;
		CloseHandle(m_read_overlapped.hEvent);
		CloseHandle(m_write_overlapped.hEvent);
	}

	m_packetlen = 0;
	m_reportid = 0;
}
int USBFile::write(const char* buf,int size)
{
	//WriteFile(); HidD_SetFeature(); HidD_SetOutputReport()
	if(INVALID_HANDLE_VALUE==m_h || size<=0)
		return -1;

	int pos = 0;
	int n = 0;
	int sendall = 0;
	while(pos!=size)
	{
		n = size-pos;
		if(n>(m_packetlen-1)) n=m_packetlen-1;
		memset(m_sendbuf+1,0,m_packetlen-1);
		memcpy(m_sendbuf+1,buf+pos,n);
		pos += n;
		if(m_packetlen==write_pack(m_sendbuf))
		{
			sendall += n;
		}
	}

	return sendall;
}
int USBFile::write_pack(const char* buf)
{
	DWORD n = 0;
	if(INVALID_HANDLE_VALUE==m_h)
		return -1;
	SetLastError(0);
	BOOL fState;
	fState = WriteFile(m_h,buf,m_packetlen,&n,/*NULL*/&m_write_overlapped);
	if(fState == false)
	{
		int lastcount = 0;
		BOOL  bResult = TRUE;	
		while(1)
		{
			DWORD dwError = GetLastError();
			if(dwError == ERROR_IO_PENDING || dwError == ERROR_IO_INCOMPLETE)
			{
				//WaitForSingleObject(m_write_overlapped.hEvent,2000/*INFINITE*/); //µÈ2Ãë
				bResult = GetOverlappedResult(m_h,	// Handle to COMM port 
												&m_write_overlapped,		// Overlapped structure
												&n,		// Stores number of bytes read
												FALSE); 			// Wait flag NOµÈ´ý
			}else{
				return 0;
			}

			if(bResult == TRUE)
			{
				assert(n==m_packetlen);
				return n;
			}
			else
			{			
				lastcount++;
				Sleep(100);
				if(lastcount > 5)
				{
					return 0;
				}
			}		
		}
	}
	//DWORD err = GetLastError();
	return -1;
}
int USBFile::read_packet(char* buf,int bufsize)
{
	//ReadFile(); HidD_GetFeature(); HidD_GetInputReport()
	if(INVALID_HANDLE_VALUE==m_h)
		return -1; 
	if(bufsize<m_packetlen)
		return -2;

	//read
	DWORD n=0;
	SetLastError(0);
	BOOL fState;
	fState = ReadFile(m_h,buf,m_packetlen,&n,/*NULL*/&m_read_overlapped);
	if(fState == false)
	{
		int lastcount = 0;
		BOOL  bResult = TRUE;	
		while(1)
		{
			DWORD dwError = GetLastError();
			if(dwError == ERROR_IO_PENDING || dwError == ERROR_IO_INCOMPLETE)
			{
				//WaitForSingleObject(m_read_overlapped.hEvent,200/*INFINITE*/);
				bResult = GetOverlappedResult(m_h,	// Handle to COMM port 
												&m_read_overlapped,		// Overlapped structure
												&n,		// Stores number of bytes read
												FALSE); 			// Wait flag NOµÈ´ý
			}
			else
			{
				return 0;
			}

			if(bResult == TRUE)
			{
				assert(n==m_packetlen);
				return n;
			}
			else
			{			
				lastcount++;
				Sleep(100);
				if(lastcount > 5)
				{
					return 0;
				}
			}		
		}
	}
	//DWORD err = GetLastError();
	return 0;
}


