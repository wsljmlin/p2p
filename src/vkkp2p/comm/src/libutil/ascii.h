#pragma once

// '0' = 48, '9' = 57,'A'=65 'Z'=90'a' = 97 'z'=122
//


//����Ƿ���hex��ʾ�ַ�
inline bool is_hexsz(char c);
//ʮ�������ַ�תAC��outLen outLenָ�����Ȳ���ʱ������-1��outLen�������Ҫ�ĳ���,�в��Ϸ��ַ�����-2
int hexsz_2_ascii(char* outBuf,int& outLen,const char* inHexsz,int inLen); 

//ascii�ַ�ת��16����hex��ʾ�ַ�ֵ
int ascii_2_hexsz(char* outHexsz,int& outLen,const char* inBuf,int inLen); 
