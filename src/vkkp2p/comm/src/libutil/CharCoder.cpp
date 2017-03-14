#include "CharCoder.h"


void CharCoder::UTF_8ToUnicode(wchar_t* pOut,const char *pText)  // 把UTF-8转换成Unicode  
{
	char* uchar = (char *)pOut;  
	uchar[1] = ((pText[0] & 0x0F) << 4) + ((pText[1] >> 2) & 0x0F);  
	uchar[0] = ((pText[1] & 0x03) << 6) + (pText[2] & 0x3F);  
	return; 
}

void CharCoder::UnicodeToUTF_8(char* pOut,wchar_t* pText)  //Unicode 转换成UTF-8  
{
	// 注意 WCHAR高低字的顺序,低字节在前，高字节在后  
	char* pchar = (char *)pText;  
	pOut[0] = (0xE0 | ((pchar[1] & 0xF0) >> 4));  
	pOut[1] = (0x80 | ((pchar[1] & 0x0F) << 2)) + ((pchar[0] & 0xC0) >> 6);  
	pOut[2] = (0x80 | (pchar[0] & 0x3F));  
	return;  
}

void CharCoder::UnicodeToGB2312(char* pOut,wchar_t uData)  // 把Unicode 转换成 GB2312    
{
#ifdef _WIN32
	WideCharToMultiByte(CP_ACP,NULL,&uData,1,pOut,sizeof(wchar_t),NULL,NULL);  
#endif
	return;  
}

void CharCoder::Gb2312ToUnicode(wchar_t* pOut,const char *gbBuffer)// GB2312 转换成　Unicode  
{
#ifdef _WIN32
	::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,gbBuffer,2,pOut,1); 
#endif
	return;
}

void CharCoder::GB2312ToUTF_8(string& pOut,const char *pText, int pLen)//GB2312 转为 UTF-8  
{
	char buf[4];  
	int nLength = pLen* 3;  
	char* rst = new char[nLength];  
	memset(buf,0,4);  
	memset(rst,0,nLength);  
	int i = 0;  
	int j = 0;        
	while(i < pLen)  
	{
	   //如果是英文直接复制就可以  
	   if( *(pText + i) >= 0)  
	   {  
			rst[j++] = pText[i++];  
	   }  
	   else  
	   {
			wchar_t pbuffer;  
		   Gb2312ToUnicode(&pbuffer,pText+i);  
		   UnicodeToUTF_8(buf,&pbuffer);  
			rst[j] = buf[0];  
			rst[j+1] = buf[1];  
			rst[j+2] = buf[2];      
		   j += 3;  
		   i += 2;  
	   }  
	}  
	rst[j] = '\0';  
	//返回结果  
	pOut = rst;            
	delete []rst;     
	return;  
}

void CharCoder::UTF_8ToGB2312(string& pOut,const char *pText, int pLen) //UTF-8 转为 GB2312 
{
	char * newBuf = new char[pLen*2];  
	char Ctemp[4];  
	memset(Ctemp,0,4);  
	int i =0;  
	int j = 0;  
	while(i < pLen)  
	{  
		if(pText[i] > 0)  
		{  
			newBuf[j++] = pText[i++];               
		}  
		else                    
		{  
			wchar_t Wtemp;  
			UTF_8ToUnicode(&Wtemp,pText + i);        
			UnicodeToGB2312(Ctemp,Wtemp);         
			newBuf[j] = Ctemp[0];  
			newBuf[j + 1] = Ctemp[1];    
			i += 3;      
			j += 2;     
		}  
	}  
	newBuf[j] = '\0';    
	pOut = newBuf;    
	delete []newBuf;  
	return; 
}


