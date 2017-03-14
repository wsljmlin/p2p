#include "ascii.h"
#include <assert.h>

inline bool is_hexsz(char c)
{
	if((c>='0' && c<='9') || (c>='A' && c<='F') || (c>='a' && c<= 'f'))
		return true;
	return false;
}
int hexsz_2_ascii(char* outBuf,int& outLen,const char* inHexsz,int inLen)
{
	//inLen 必须是2的倍数
	int n = inLen/2;
	int i;
	unsigned char c=0,c2=0;
	for(i=0;(i+1)<inLen;i+=2)
	{
		if(inHexsz[i]=='0' && (inHexsz[i+1]=='x' || inHexsz[i+1]=='X'))
			n--;
		if(!is_hexsz(inHexsz[i]) || !is_hexsz(inHexsz[i+1]))
			return -2;
	}
	if(outLen < n)
	{
		outLen = n;
		return -1;
	}
	outLen = n;
	n = 0;
	for(i=0;(i+1)<inLen;i+=2)
	{
		if(inHexsz[i]=='0' && (inHexsz[i+1]=='x' || inHexsz[i+1]=='X'))
			continue;
		//'0'-48;'A'-65;'a'-97
		c = inHexsz[i];
		if(c>='0' && c<='9')
			c2 = c-48;
		else if(c>='A' && c<='F')
			c2 = c-55;
		else if(c>='a' && c<= 'f')
			c2 = c-87;
		else
			return -2;
		c2 *= 16;
		c = inHexsz[i+1];
		if(c>='0' && c<='9')
			c2 += c-48;
		else if(c>='A' && c<='F')
			c2 += c-55;
		else if(c>='a' && c<= 'f')
			c2 += c-87;
		else
			return -2;
		outBuf[n++] = c2;
	}
	assert(n==outLen);
	return 0;
}
int ascii_2_hexsz(char* outHexsz,int& outLen,const char* inBuf,int inLen)
{
	unsigned char c, c1,c2;
	if(outLen<2*inLen)
	{
		outLen = 2*inLen;
		return -1;
	}
	outLen = 2*inLen;
	for(int i=0;i<inLen;++i)
	{
		c = inBuf[i];
		c1 = c/16;
		c2 = c%16;
		if(c1<10)
			outHexsz[2*i] = '0' + c1;
		else
			outHexsz[2*i] = 'A' + (c1-10);
		if(c2<10)
			outHexsz[2*i+1] = '0' + c2;
		else
			outHexsz[2*i+1] = 'A' + (c2-10);
	}
	return 0;
}



