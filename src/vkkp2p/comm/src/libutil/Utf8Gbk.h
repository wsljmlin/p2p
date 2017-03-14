#pragma once

//�ı��ļ�ͷ:FF��FE��Unicode��,FE��FF��Unicode big endian��,EF��BB��BF��UTF-8��
//GB2312��ԭ�Ļ�����λ�룬����λ�뵽���룬��Ҫ�ڸ��ֽں͵��ֽ��Ϸֱ����A0
//��DBCS�У�GB����Ĵ洢��ʽʼ����big endian������λ��ǰ
//GB2312�������ֽڵ����λ����1�������������������λֻ��128*128=16384��������GBK��GB18030�ĵ��ֽ����λ�����ܲ���1�������ⲻӰ��DBCS�ַ����Ľ������ڶ�ȡDBCS�ַ���ʱ��ֻҪ������λΪ1���ֽڣ��Ϳ��Խ��������ֽ���Ϊһ��˫�ֽڱ��룬�����ùܵ��ֽڵĸ�λ��ʲô



int utf8_2_gbk(char *utf8,int utf8len,char *gbk,int gbklen);
///////////////////////////////////////////
//˵��:
//unicodelenΪunsigned short����
//utf8lenΪchar����,����ʾʵ���ַ�������
//��unicodeΪNULLʱ:return ת����Ҫ����С����
//��unicode!=NULL && unicodelenС��ʵ����Ҫ����ʱ,return 0;��ʾû��ת��,
//��unicode!=NULL,return > 0��ʾת���ɹ�
//ʵ�ʳ��Ȳ������ַ���������־.�������Զ������ַ���������־,�������о�������
int utf8_2_unicode(char *utf8,int utf8len,unsigned short *unicode,int unicodelen);

//////////////////////////////////////////
//unicodelenΪunsigned short����
//gbklenΪchar����
//��gbkΪNULLʱ:return ת����Ҫ����С����
//��gbk!=NULL && gbklenС��ʵ����Ҫ����ʱ,return 0;��ʾû��ת��,
//��gbk!=NULL,return > 0��ʾת���ɹ�
int unicode_2_gbk(unsigned short *unicode,int unicodelen,char *gbk,int gbklen);

