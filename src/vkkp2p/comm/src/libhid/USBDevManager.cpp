
#include "USBDevManager.h"
#include "ChrConverter.h"

//****************************
//获取HID硬件的能力,如一次接收包大小等
HIDP_CAPS g_hidCapabilities;
int hid_get_capabilities(HANDLE h)
{
	int ret = -1;
	PHIDP_PREPARSED_DATA	PreparsedData;
	if(HidD_GetPreparsedData(h, &PreparsedData))
	{
		if(HIDP_STATUS_SUCCESS==HidP_GetCaps(PreparsedData, &g_hidCapabilities))
		{
			ret = 0;
		}
		HidD_FreePreparsedData(PreparsedData);
	}
	return ret;
}
//****************************
string hid_get_path(USHORT vid,USHORT pid)
{
	string strRet = "";
	GUID guidHID;
	HidD_GetHidGuid(&guidHID);

	HDEVINFO hDevInfo = SetupDiGetClassDevs(&guidHID,NULL,0,
		DIGCF_PRESENT|DIGCF_DEVICEINTERFACE );
	
	if(hDevInfo==INVALID_HANDLE_VALUE)
	{
		return strRet;
	}

	SP_DEVICE_INTERFACE_DATA strtInterfaceData;
	strtInterfaceData.cbSize=sizeof(SP_DEVICE_INTERFACE_DATA);
	BOOL bSuccess ;
	DWORD index=0;
	string temp;
	int i=0;
	bool bfind = false;
	//test:
	while(!bfind && ++i<100)
	{
		bSuccess= SetupDiEnumDeviceInterfaces(hDevInfo,NULL,&guidHID,index,&strtInterfaceData);
		++index;
		if (!bSuccess)
		{	
			break;
		}
		else
		{
			if(strtInterfaceData.Flags==SPINT_ACTIVE )
			{
				//ShowMore(hDevInfo,strtInterfaceData);
				PSP_DEVICE_INTERFACE_DETAIL_DATA_A strtDetailData;

				DWORD strSzie=0,requiesize=0;
				SetupDiGetDeviceInterfaceDetailA(hDevInfo,&strtInterfaceData,NULL,0,&strSzie,NULL);

				requiesize=strSzie;
				strtDetailData=(PSP_DEVICE_INTERFACE_DETAIL_DATA_A)malloc(requiesize);
				strtDetailData->cbSize=sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

				SP_DEVINFO_DATA infodata;
				infodata.cbSize=sizeof(SP_DEVINFO_DATA);
				if (SetupDiGetDeviceInterfaceDetailA(hDevInfo,&strtInterfaceData,strtDetailData,strSzie,&requiesize,NULL))
				{
					//CString strFind = strtDetailData->DevicePath;
					//test:
					//if (strFind.Find(_T("&0002")) != -1 )
					{
						HANDLE hCom = CreateFileA (
						  strtDetailData->DevicePath,
						  GENERIC_READ | GENERIC_WRITE,
						  FILE_SHARE_READ | FILE_SHARE_WRITE,
						  NULL, 
						  OPEN_EXISTING, 0, 
						  NULL);
						if (hCom != INVALID_HANDLE_VALUE)
						{
							HIDD_ATTRIBUTES strtAttrib;
							if (HidD_GetAttributes(hCom,&strtAttrib))
							{
								if(vid==strtAttrib.VendorID && pid==strtAttrib.ProductID)
								{
									bfind = true;
									strRet = strtDetailData->DevicePath;

									hid_get_capabilities(hCom);

									//BOOLEAN rc = 0;
									//PHIDP_PREPARSED_DATA	PreparsedData;
									//rc = HidD_GetPreparsedData(hCom, &PreparsedData);
									//HIDP_CAPS	Capabilities;
									//HidP_GetCaps(PreparsedData, &Capabilities);
									//if(Capabilities.NumberInputValueCaps==1)
									//{
									//	bfind = true;
									//	strRet = strtDetailData->DevicePath;
									//}
								}
							}
							CloseHandle(hCom);
						}
					}
				}
				free(strtDetailData);
			}
		}
	}
	SetupDiDestroyDeviceInfoList(hDevInfo);
	return strRet;
}

int hid_get_id(const string& path,USHORT& vid,USHORT& pid)
{
	BOOL ret = -1;
	HANDLE hCom = CreateFileA (
		path.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, 
		OPEN_EXISTING, 0, 
		NULL);
	if (hCom != INVALID_HANDLE_VALUE)
	{
		HIDD_ATTRIBUTES strtAttrib;
		if (HidD_GetAttributes(hCom,&strtAttrib))
		{
			ret = 0;
			vid = strtAttrib.VendorID;
			pid = strtAttrib.ProductID;
		}
		CloseHandle(hCom);
	}
	return ret;
}
void hid_test(const string& path)
{
	if(path.empty())
		return;
	HANDLE hCom = CreateFileA(
		path.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, 
		OPEN_EXISTING, 0, 
		NULL);
	if (hCom == INVALID_HANDLE_VALUE)
	{
		return;
	}

	//write
	char buf[1024];
	DWORD n = 0;
	BOOL b = FALSE;
	memset(buf,0,1024);
	//if((b=WriteFile(hCom,buf,128,&n,NULL)))
	//{
	//}

	//read
	b = FALSE;
	if((b=ReadFile(hCom,buf,1024,&n,NULL)))
	{
	}

	CloseHandle(hCom);;
}
//********************************************************************

HDEVNOTIFY hid_register_WM_DEVICECHANGE_message(HWND hwnd)
{
	GUID guidHID;
	HidD_GetHidGuid(&guidHID);

	HDEVNOTIFY hDevNotify;
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

	NotificationFilter.dbcc_classguid = guidHID;
	hDevNotify = RegisterDeviceNotification(hwnd, &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if(NULL==hDevNotify)
	{
		return NULL;
	}
	return hDevNotify;
}
int hid_unregister_WM_DEVICECHANGE_message(HWND hwnd,HDEVNOTIFY h)
{
	UnregisterDeviceNotification(h);
	return 0;
}
LRESULT hid_on_WM_DEVICECHANGE_message_sample(WPARAM wParam, LPARAM lParam)
{
	if ( DBT_DEVICEARRIVAL == wParam || DBT_DEVICEREMOVECOMPLETE == wParam ) 
	{
		PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)lParam;
		PDEV_BROADCAST_DEVICEINTERFACE pDevInf;
		PDEV_BROADCAST_HANDLE pDevHnd;
		PDEV_BROADCAST_OEM pDevOem;
		PDEV_BROADCAST_PORT pDevPort;
		PDEV_BROADCAST_VOLUME pDevVolume;
		USHORT vid,pid;
		switch( pHdr->dbch_devicetype ) 
		{
			case DBT_DEVTYP_DEVICEINTERFACE:
				{
					pDevInf = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
					//pDevInf->dbcc_name
					if(DBT_DEVICEARRIVAL == wParam)
					{
						//插入硬件，根据PATH读VID/PID
						string path;
#ifdef UNICODE
						char spath[1024]={0,};
						_W2A(spath,1024,pDevInf->dbcc_name);
#else
						path = pDevInf->dbcc_name;
#endif
						hid_get_id(path,vid,pid);
					}
					else
					{
						//拔出硬件，对比路径
					}
				}
				break;

			case DBT_DEVTYP_HANDLE:
				pDevHnd = (PDEV_BROADCAST_HANDLE)pHdr;
				break;

			case DBT_DEVTYP_OEM:
				pDevOem = (PDEV_BROADCAST_OEM)pHdr;
				break;

			case DBT_DEVTYP_PORT:
				pDevPort = (PDEV_BROADCAST_PORT)pHdr;
				break;

			case DBT_DEVTYP_VOLUME:
				pDevVolume = (PDEV_BROADCAST_VOLUME)pHdr;
				break;
		}
	}
	return S_OK;
}

