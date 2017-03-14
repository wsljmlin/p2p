#pragma once

//sOutStr和sOutBuf不能同时为空，sOutStr最小41长度，sOutBuf最小20长度，iSleepMSec表示每运算一长度时休息一下
int Sha1_BuildFile(const char* sFile,char* sOutStr,char* sOutBuf,int iSleepMSec=-1);  
int Sha1_BuildBuffer(const char* buf,int bufLen,char* sOutStr,char* sOutBuf);
int Sha1_Buf2Str(char* sOutStr, const char* sInBuf);
int Sha1_Str2Buf(char* sOutBuf, const char* sInStr);
