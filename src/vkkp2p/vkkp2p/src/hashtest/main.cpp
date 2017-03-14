
#include <stdio.h>
#include "nhash.h"

int main(int argc,char** argv)
{
	hash_t hash;
	char buf[48];
	//char *url = "http://127.0.0.1/live/test.m3u8";
	if(argc>1)
	{
		hash.set_urldl_string(argv[1]);
		hash.to_string(buf,48);
		printf(buf);
	}
	return 0;
}