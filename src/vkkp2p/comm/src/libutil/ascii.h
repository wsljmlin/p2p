#pragma once

// '0' = 48, '9' = 57,'A'=65 'Z'=90'a' = 97 'z'=122
//


//检查是否是hex显示字符
inline bool is_hexsz(char c);
//十六进制字符转AC当outLen outLen指定长度不够时，返回-1，outLen计算出需要的长度,有不合法字符返回-2
int hexsz_2_ascii(char* outBuf,int& outLen,const char* inHexsz,int inLen); 

//ascii字符转成16进制hex显示字符值
int ascii_2_hexsz(char* outHexsz,int& outLen,const char* inBuf,int inLen); 
