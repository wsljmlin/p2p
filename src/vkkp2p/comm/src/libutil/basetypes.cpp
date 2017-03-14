#include "basetypes.h"

// int printf(const char *format,[argument]);
// format 参数输出的格式，定义格式为：
// %[flags][width][.perc] [F|N|h|l]type



#ifdef _WIN32

char* strcasestr(const char* haystack,const char* needle)
{
    int i=0;
    while(*haystack&&*needle)
	{           
        while(*haystack==*needle||\
              *haystack==((*needle>='a'&&*needle<='z')?*needle-32:*needle)||\
              *haystack==((*needle>='A'&&*needle<='Z')?*needle+32:*needle))
		{
			if(!*(++needle))return (char*)haystack-i;
			if(!*(++haystack))return 0;
			i++;
        }
        needle-=i;
        haystack-=(i-1);
        i=0;
    }
    return 0;
}

#else

//string   to   low
char* strlwr(char* str)
{
	char * p = str;
	if(!p)
		return p;
	for(;*p!='\0';++p)
		*p=tolower(*p);
	return str;
}

#endif

