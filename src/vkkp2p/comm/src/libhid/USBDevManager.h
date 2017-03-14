#pragma once

#include <Windows.h>
#include <setupapi.h>
#include <Dbt.h>
extern "C"
{
	#include "../../include/hidsdi.h"
	//#include <c:\WinDDK\7600.16385.1\inc\api\hidsdi.h>
}
#pragma comment(lib,"../../lib/hid.lib")
//#pragma comment(lib,"c:/WinDDK/7600.16385.1/lib/win7/i386/hid.lib")
#pragma comment(lib,"setupapi.lib")

#include "basetypes.h"

//#ifdef LIBHID_EXPORTS
//#define API_DECLSPEC    __declspec(dllexport)
//#else
//#define API_DECLSPEC    __declspec(dllimport)
//#endif
//#undef API_DECLSPEC

#ifndef API_DECLSPEC
	#define API_DECLSPEC
#endif


//����id��HIDӲ����path
API_DECLSPEC string hid_get_path(USHORT vid,USHORT pid);
//����HIDӲ����path��vid/pid
API_DECLSPEC int hid_get_id(const string& path,USHORT& vid,USHORT& pid);
API_DECLSPEC void hid_test(const string& path);

//ע����ӦHID�Ĳ��֪ͨ
//ע����Ȼ���봰�ھ����ע����󴰿ڴ���WM_DEVICECHANGE��Ϣ��
API_DECLSPEC HDEVNOTIFY hid_register_WM_DEVICECHANGE_message(HWND hwnd);
API_DECLSPEC int hid_unregister_WM_DEVICECHANGE_message(HWND hwnd,HDEVNOTIFY h);
API_DECLSPEC LRESULT hid_on_WM_DEVICECHANGE_message_sample(WPARAM wParam, LPARAM lParam);


