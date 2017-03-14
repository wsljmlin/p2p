
#include "ChrConverter.h"
#include <string.h>

#ifdef _WIN32
#include <windows.h>
int wideToAcp(char* des,int deslen,const wchar_t* res)
{
	//简体中文系统:CP_ACP转出来的就是GBK
	//繁体中文系统:936转出来的就是GBK
	//为了繁简歉用,暂均用936未发现新问题,(注.CP_ACP在繁体系统中是乱码的)
	//_tcslen / wcslen / strlen
	//注:1个汉字作为1个宽字单位,而1个汉字作为2个char单位.所以:2*res长度>=des长度>=res长度
	if(NULL==res)
		return -1;
	int n=0;
	int len=(int)::wcslen(res);
	if(0==len) return 0;
	n = WideCharToMultiByte(/*CP_ACP*/936,0,res,len,NULL,0,NULL,NULL);
	if(NULL==des) return n+1;
	if(deslen<n+1) return -2;
	n = WideCharToMultiByte(/*CP_ACP*/936,0,res,len,&des[0],n,NULL,NULL);
	des[n] = '\0';
	return n;
}
int acpToWide(wchar_t* des,int deslen,const char* res)
{
	if(NULL==res)
		return -1;
	int n=0;
	int len=(int)strlen(res);
	if(0==len) return 0;
	n = MultiByteToWideChar(/*CP_ACP*/936,/*MB_PRECOMPOSED*/0,res,len,NULL,0);
	if(NULL==des) return n+1;
	if(deslen<n+1) return -2;
	n = MultiByteToWideChar(/*CP_ACP*/936,/*MB_PRECOMPOSED*/0,res,len,&des[0],n);
	des[n]='\0';
	return n;
}
#else

#endif
//
//std::string& wideToAcp(std::string& des,const std::wstring& res)
//{
//	//简体中文系统:CP_ACP转出来的就是GBK
//	//繁体中文系统:936转出来的就是GBK
//	//为了繁简歉用,暂均用936未发现新问题,(注.CP_ACP在繁体系统中是乱码的)
//	//_tcslen / wcslen / strlen
//	//注:1个汉字作为1个宽字单位,而1个汉字作为2个char单位.所以:2*res长度>=des长度>=res长度
//	int n=0;
//	int len=res.length();
//	char *p = NULL;
//	n = WideCharToMultiByte(/*CP_ACP*/936,0,res.c_str(),len,NULL,0,NULL,NULL);
//	p = new char[n+1];
//	n = WideCharToMultiByte(/*CP_ACP*/936,0,res.c_str(),len,p,n,NULL,NULL);
//	p[n] = '\0';
//	des = p;
//	delete[] p;
//	return des;
//}
//std::wstring& acpToWide(std::wstring& des,const std::string& res)
//{
//	int n=0;
//	int len=res.length();
//	wchar_t *p = NULL;
//	n = MultiByteToWideChar(/*CP_ACP*/936,/*MB_PRECOMPOSED*/0,res.c_str(),len,NULL,0);
//	p = new wchar_t[n+1];
//	n = MultiByteToWideChar(/*CP_ACP*/936,/*MB_PRECOMPOSED*/0,res.c_str(),len,p,n);
//	p[n]='\0';
//	des = p;
//	delete []p;
//	return des;
//}
//CStringA& wideToAcp(CStringA& des,const CStringW& res)
//{
//	//简体中文系统:CP_ACP转出来的就是GBK
//	//繁体中文系统:936转出来的就是GBK
//	//为了繁简歉用,暂均用936未发现新问题,(注.CP_ACP在繁体系统中是乱码的)
//	//_tcslen / wcslen / strlen
//	//注:1个汉字作为1个宽字单位,而1个汉字作为2个char单位.所以:2*res长度>=des长度>=res长度
//	int n=0;
//	int len=res.GetLength();
//	char *p = NULL;
//	n = WideCharToMultiByte(/*CP_ACP*/936,0,res,len,NULL,0,NULL,NULL);
//	p = new char[n+1];
//	n = WideCharToMultiByte(/*CP_ACP*/936,0,res,len,p,n,NULL,NULL);
//	p[n] = '\0';
//	des = p;
//	delete[] p;
//	return des;
//}
//CStringW& acpToWide(CStringW& des,CStringA& res)
//{
//	int n=0;
//	int len=res.GetLength();
//	wchar_t *p = NULL;
//	n = MultiByteToWideChar(/*CP_ACP*/936,/*MB_PRECOMPOSED*/0,res,len,NULL,0);
//	p = new wchar_t[n+1];
//	n = MultiByteToWideChar(/*CP_ACP*/936,/*MB_PRECOMPOSED*/0,res,len,p,n);
//	p[n]='\0';
//	des = p;
//	delete []p;
//	return des;
//}

