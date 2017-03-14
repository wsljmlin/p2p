#pragma once

#include <wchar.h>

#define _W2A(a,len,w) (wideToAcp(a,len,w))
#define _A2W(w,len,a) (acpToWide(w,len,a))

extern int wideToAcp(char* des,int deslen,const wchar_t* res);
extern int acpToWide(wchar_t* des,int deslen,const char* res);

//#include <string>
//extern std::string& wideToAcp(std::string& des,const std::wstring& res);
//extern std::wstring& acpToWide(std::wstring& des,const std::string& res);

//#ifdef _WIN32
////#include <atlstr.h>
//#include <afxstr.h>
//extern CStringA& wideToAcp(CStringA& des,const CStringW& res);
//extern CStringW& acpToWide(CStringW& des,CStringA& res);
//#endif



