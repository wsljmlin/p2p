
//#include <ctime>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mini_md5.h"
#include "random.h"

using namespace miniutil;

void Random::rand_seed(unsigned int seed)
{
    srand(seed);
}
/*
	GNU lib RAND_MAX = 0x7FFFFFFF
*/
unsigned int Random::rand_value(unsigned int minvalue, unsigned int maxvalue)
{
    if(minvalue > maxvalue)
        return 0;

	int partial, total;

    partial = (rand()&0x7FFF);
	total = (RAND_MAX&0x7FFF);

	return (unsigned int)(minvalue + (maxvalue-minvalue)*(partial/(double)total));	
}

unsigned int Random::rand_value(unsigned int maxvalue)
{
	int partial, total;
	//assert(maxvalue < 0x7FFF);
	partial = (rand()&0x7FFF);
	total = (RAND_MAX&0x7FFF);

	return (unsigned int)(maxvalue*(partial/(double)total));	
}
	
int Random::rand_stringbuf16(int seed_in, char *buf_out)
{
	int seed,i;
	char buf[1024];
	seed = (int)time(NULL)+seed_in;
	srand(seed);
	memset(buf,0x00,sizeof(buf));
	for(i=0; i<(int)sizeof(buf); i++)
	{
		//buf[i] = (char)((rand()/(float)RAND_MAX)*255);
		buf[i] = (char)rand_value(255);
	}
	MD5 md5;
	md5.update((unsigned char*)buf,1024);
	md5.finalize();
	memcpy((char*)buf_out, md5.raw_digest(), 16);
	return 0;	
}
	
int Random::rand_stringbuf(int len, char *buf_out, char* excludechars, int seed_in)
{
	int seed,i;
	char aByte;
	assert(buf_out != NULL);
	seed = (int)time(NULL)+seed_in;
	srand(seed);
	for(i=0; i<len; i++)
	{
		aByte = (char)rand_value(255);

		//RAND_MAX is not 7FFFF on linux ubuntu..
		//aByte = (char)(255*((double)rand()/RAND_MAX));
		if(aByte == 0x00 || 
			(excludechars != NULL && strchr(excludechars, aByte) != NULL))
		{
			i--;
		}
		else
			buf_out[i] = aByte;
	}	
	return 0;
}	
