#include "Util.h"
#include <time.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

string Util::get_string_index(const string& source,int index,const string& sp)
{
	if(sp.empty() || index<0)
		return source;

	int splen = (int)sp.length();
	int pos1=0-splen,pos2=0-splen;

	int i=0;
	for(i=0;i<(index+1);i++)
	{
		pos1 = pos2+splen;
		pos2 = (int)source.find(sp,pos1);
		if(pos2<0)
			break;
	}
	if(i<index)
		return "";

	pos2 = pos2<pos1?(int)source.length():pos2;
	return source.substr(pos1,pos2-pos1);

}
int Util::get_string_index_pos(const string& source,int index,const string& sp)
{
	if(index < 0 || sp.empty())
		return -1;
	int splen = (int)sp.length();
	int pos = 0 - splen;
	for(int i=0;(pos=(int)source.find(sp,pos+splen))>=0 && i<index;i++);
	return pos;
}
int Util::get_string_index_count(const string& source,const string& sp)
{
	if(sp.empty() || source.empty())
		return 0;
	int i=1,pos=0,splen = (int)sp.length();
	pos = 0-splen;
	for(i=1;(pos=(int)source.find(sp,pos+splen))>=0;i++);
	return i;

}
string Util::set_string_index(string &source,int index,const string& val, const string& sp)
{
	if(index<0 || sp.empty())
		return source;

	int splen = (int)sp.length();
	int i=0,pos1,pos2=0;

	pos1=0-splen,pos2=0-splen;
	for(i=0;i<(index+1);i++)
	{
		pos1 = pos2+splen;
		pos2 = (int)source.find(sp,pos1);
		if(pos2<0)
		{
			if(i==index)
				break;
			else
			{
				source += sp;//补充;
				pos2  = (int)source.find(sp,pos1);//这次一定找到
			}
		}
	}

	if(pos2<0)
	{
		assert(i == index);
		source = source.substr(0,pos1) + val;
	}
	else
	{
		source = source.substr(0,pos1) + val + source.substr(pos2);
	}
	return source;

}
string& Util::string_trim(string& str,char c/*=' '*/)
{
	int pos1=0,pos2=(int)str.length()-1;
	for(; pos1<=pos2 && c==str.at(pos1);++pos1);//全空的话超越边界
	for(; pos2>pos1 && c==str.at(pos2);--pos2);//上面已经判断过=,此处不再
	if( pos1>pos2)
		str = "";
	else
		str = str.substr(pos1,pos2-pos1+1);
	return str;
}

string& Util::string_trim_endline(string& str)
{
	int pos1=0,pos2=(int)str.length()-1;
	for(; pos1<=pos2 && ('\r'==str.at(pos1)||'\n'==str.at(pos1));++pos1);//全空的话超越边界
	for(; pos2> pos1 && ('\r'==str.at(pos2)||'\n'==str.at(pos2));--pos2);//上面已经判断过=,此处不再
	if( pos1>pos2)
		str = "";
	else
		str = str.substr(pos1,pos2-pos1+1);
	return str;
}
string& Util::str_replace(string& str,const string& str_old,const string& str_new)
{
	int pos = (int)str.find(str_old);
	while(pos > -1)
	{
		str.replace(pos,str_old.length(),str_new.c_str());
		pos = (int)str.find(str_old);
	}
	return str;
}
bool Util::get_stringlist_from_file(const string& path,list<string>& ls)
{
	char buf[2048];
	char *ptr = NULL;
	FILE *fp = fopen(path.c_str(),"rb+");
	if(!fp)
		return false;
	ls.clear();
	while(!feof(fp))
	{
		memset(buf,0,2048);
		if(NULL==fgets(buf,2048,fp))
			continue;
		if((ptr=strstr(buf,"\r")))
			*ptr = '\0';
		if((ptr=strstr(buf,"\n")))
			*ptr = '\0';

		ls.push_back(buf);
	}
	fclose(fp);
	return true;
}
bool Util::put_stringlist_to_file(const string& path,list<string>& ls)
{
	FILE *fp = fopen(path.c_str(),"wb+");
	if(!fp)
		return false;
	for(list<string>::iterator it=ls.begin();it!=ls.end(); ++it)
	{
		fwrite((*it).c_str(),(*it).length(),1,fp);
		fwrite("\r\n",2,1,fp);
	}
	fclose(fp);
	return true;
}

int Util::write_log(const char *strline,const char *path)
{
	char *buf=new char[strlen(strline)+128];
	if(!buf)
		return -1;
	time_t tt = time(0);
	tm *t = localtime(&tt);
	sprintf(buf,"[%d-%d-%d %d:%d:%d] %s \r\n",
		t->tm_year+1900,t->tm_mon+1,t->tm_mday,
		t->tm_hour,t->tm_min,t->tm_sec,strline);

	FILE *fp = fopen(path,"ab+");
	if(fp)
	{
		fwrite(buf,strlen(buf),1,fp);
		fclose(fp);
	}
	delete[] buf;
	return 0;
}

