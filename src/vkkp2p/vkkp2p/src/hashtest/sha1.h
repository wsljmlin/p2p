#pragma once

//sOutStr��sOutBuf����ͬʱΪ�գ�sOutStr��С41���ȣ�sOutBuf��С20���ȣ�iSleepMSec��ʾÿ����һ����ʱ��Ϣһ��
int Sha1_BuildFile(const char* sFile,char* sOutStr,char* sOutBuf,int iSleepMSec=-1);  
int Sha1_BuildBuffer(const char* buf,int bufLen,char* sOutStr,char* sOutBuf);
int Sha1_Buf2Str(char* sOutStr, const char* sInBuf);
int Sha1_Str2Buf(char* sOutBuf, const char* sInStr);
