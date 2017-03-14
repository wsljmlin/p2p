#include "Utf8Gbk.h"
#include "gb2312txt.h"


int utf8_2_gbk(char *utf8,int utf8len,char *gbk,int gbklen)
{
	int reallen=0;
	int i=0;
	int n=0;
	unsigned char *ptr=0;
	int usbuflen = 0;
	unsigned short *usbuf = 0;
	if(utf8==0||utf8len<=0)
		return 0;
	//计数:
	//只考虑UTF8最多3个关联,当大于3个的当坏码处理无法转换.直接返回失败
	for(i=0,ptr=(unsigned char*)utf8;i<utf8len;)
	{
		if(*ptr <= 0x7F) //0xxxxxxx
		{
			n = 1;
			reallen += 1;
		}
		else if(*ptr <= 0xDF) //110xxxxx 10xxxxxx
		{
			n = 2;
			if(i+n>utf8len)
				break;
			reallen += 2;
		}
		else if(*ptr <= 0xEF) //1110xxxx 10xxxxxx 10xxxxxx
		{
			n = 3;
			if(i+n>utf8len)
				break;
			reallen += 2;
		}
		else
		{
			return 0;
		}
		i+=n;
		ptr+=n;
	}
	if(0==gbk)
		return reallen+1;
	if(reallen>gbklen)
		return 0;

	//转unicode
	usbuflen = utf8_2_unicode(utf8,utf8len,0,0);
	if(usbuflen<=0)
		return 0;

#ifdef __cplusplus
	usbuf = new unsigned short[usbuflen+2];
#else
	usbuf = (unsigned short*)malloc((usbuflen+2)*2);
#endif
	usbuflen = utf8_2_unicode(utf8,utf8len,usbuf,usbuflen);
	usbuf[usbuflen]=0;

	reallen = unicode_2_gbk(usbuf,usbuflen,0,0);
	if(reallen>gbklen)
	{
#ifdef __cplusplus
		delete[] usbuf;
#else
		free(usbuf);
#endif
		return 0;
	}
	reallen = unicode_2_gbk(usbuf,usbuflen,gbk,gbklen);
	if(gbklen>reallen)
	{
		gbk[reallen]='\0';
	}

#ifdef __cplusplus
	delete[] usbuf;
#else
	free(usbuf);
#endif
	return reallen;
}
///////////////////////////////////////////
//说明:
//unicodelen为unsigned short个数
//utf8len为char个数,不表示实际字符串长度
//当unicode为NULL时:return 转换需要的最小长度
//当unicode!=NULL && unicodelen小于实际需要长度时,return 0;表示没有转换,
//当unicode!=NULL,return > 0表示转换成功
int utf8_2_unicode(char *utf8,int utf8len,unsigned short *unicode,int unicodelen)
{
	int reallen=0;
	int i=0,j=0;
	int n=0;
	unsigned char *ptr=0;
	if(utf8==0||utf8len<=0)
		return 0;

	//计数:
	//只考虑UTF8最多3个关联,当大于3个的当坏码处理无法转换.直接返回失败
	for(i=0,ptr=(unsigned char*)utf8;i<utf8len;)
	{
		if(*ptr <= 0x7F) //0xxxxxxx
		{
			n = 1;
		}
		else if(*ptr <= 0xDF) //110xxxxx 10xxxxxx
		{
			n = 2;
			if(i+n>utf8len)
				break;
		}
		else if(*ptr <= 0xEF) //1110xxxx 10xxxxxx 10xxxxxx
		{
			n = 3;
			if(i+n>utf8len)
				break;
		}
		else
		{
			return 0;
		}
		reallen++;
		i+=n;
		ptr+=n;
	}
	if(0==unicode)
		return reallen+2;
	if(reallen>unicodelen)
		return 0;

	//转换:
	//注意:字母的unicode只占一个字符,一个字符的只有低位有值,并不指2个字节中的前面那个为0
	for(i=0,j=0,ptr=(unsigned char*)utf8;i<utf8len;)
	{
		if(*ptr <= 0x7F) //0xxxxxxx
		{
			n = 1;
			unicode[j] = (unsigned short)(*ptr) ;
		}
		else if(*ptr <= 0xDF) //110xxxxx 10xxxxxx
		{
			n = 2;
			if(i+n>utf8len)
				break;
			//??? 为什么只移动6位,不过gbk没有2位关联的不用管
			unicode[j] = ((unsigned short)((*ptr)&0x1F))<<6 | (unsigned short)((*(ptr+1))&0x3F) ;
		}
		else if(*ptr <= 0xEF) //1110xxxx 10xxxxxx 10xxxxxx
		{
			n = 3;
			if(i+n>utf8len)
				break;
			unicode[j] = ((unsigned short)((*ptr)&0x000F))<<12;
			unicode[j] |= ((unsigned short)((*(ptr+1))&0x3F))<<6;
			unicode[j] |= (unsigned short)((*(ptr+2))&0x3F);
		}
		else
		{
			return 0;
		}
		reallen++;
		i+=n;
		ptr+=n;
		j++;
	}
	if(unicodelen>j)
		unicode[j]=0;

	return j;
}

///
unsigned short map_unicode_2_gbk(unsigned short unicode)
{
	int first = 0;
	int end = UNICODE2GBK_TABLE_SIZE-1;
	int mid = 0;
	while(first <= end)
	{
		mid = (first+end)/2;
		if(g_unicode2gbk_table[mid].unicode == unicode)
			return g_unicode2gbk_table[mid].gb;
		else if(g_unicode2gbk_table[mid].unicode > unicode)
			end = mid-1;
		else
			first = mid+1;
	}
	return 0;
}

//////////////////////////////////////////
//unicodelen为unsigned short个数
//gbklen为char个数
//当gbk为NULL时:return 转换需要的最小长度
//当gbk!=NULL && gbklen小于实际需要长度时,return 0;表示没有转换,
//当gbk!=NULL,return > 0表示转换成功
int unicode_2_gbk(unsigned short *unicode,int unicodelen,char *gbk,int gbklen)
{
	int reallen=0;
	int i=0,j=0;
	int n=0;
	unsigned short *ptr=0;
	unsigned short *pgb=0;
	unsigned short gb=0;
	if(0==unicode || unicodelen<=0)
		return 0;
	//计数
	for(i=0,ptr=unicode;i<unicodelen;++i,++ptr)
	{
		if(0 == ((*ptr)&0xFF00))
		{
			n++;
		}
	}
	reallen = (unicodelen-n)*2 + n;
	if(0==gbk)
		return reallen+1;
	if(reallen>gbklen)
		return 0;

	//转换:
	for(i=0,j=0,ptr=unicode;i<unicodelen;++i,++ptr)
	{
		if(0==((*ptr)&0xFF00))
		{
			gbk[j] = (char)(*ptr);
			j++;
		}
		else
		{
			pgb = (unsigned short*)(gbk+j);
			gb = map_unicode_2_gbk(*ptr);
			if(0!=gb)
			{
				*pgb = (unsigned short)(gb<<8)|(gb>>8);
				j+=2;
			}
		}
	}
	if(gbklen>j)
		gbk[j]='\0';

	return j;
}


