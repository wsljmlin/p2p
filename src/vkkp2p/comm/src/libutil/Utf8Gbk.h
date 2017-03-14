#pragma once

//文本文件头:FF、FE（Unicode）,FE、FF（Unicode big endian）,EF、BB、BF（UTF-8）
//GB2312的原文还是区位码，从区位码到内码，需要在高字节和低字节上分别加上A0
//在DBCS中，GB内码的存储格式始终是big endian，即高位在前
//GB2312的两个字节的最高位都是1。但符合这个条件的码位只有128*128=16384个。所以GBK和GB18030的低字节最高位都可能不是1。不过这不影响DBCS字符流的解析：在读取DBCS字符流时，只要遇到高位为1的字节，就可以将下两个字节作为一个双字节编码，而不用管低字节的高位是什么



int utf8_2_gbk(char *utf8,int utf8len,char *gbk,int gbklen);
///////////////////////////////////////////
//说明:
//unicodelen为unsigned short个数
//utf8len为char个数,不表示实际字符串长度
//当unicode为NULL时:return 转换需要的最小长度
//当unicode!=NULL && unicodelen小于实际需要长度时,return 0;表示没有转换,
//当unicode!=NULL,return > 0表示转换成功
//实际长度不包括字符串结束标志.即程序不自动处理字符串结束标志,请你自行决定处理
int utf8_2_unicode(char *utf8,int utf8len,unsigned short *unicode,int unicodelen);

//////////////////////////////////////////
//unicodelen为unsigned short个数
//gbklen为char个数
//当gbk为NULL时:return 转换需要的最小长度
//当gbk!=NULL && gbklen小于实际需要长度时,return 0;表示没有转换,
//当gbk!=NULL,return > 0表示转换成功
int unicode_2_gbk(unsigned short *unicode,int unicodelen,char *gbk,int gbklen);

