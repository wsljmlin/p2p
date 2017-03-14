
#include <stdio.h>
#include <string.h>
#include "Util.h"

int main(int argc,char** argv)
{
	printf("test begin... \n");
	
	unsigned char umac[8];
	Util::get_umac(umac);
	char buf[32];
	sprintf(buf,"%02X%02X%02X%02X%02X%02X", 
		umac[0], umac[1], umac[2], umac[3], umac[4], umac[5]);
	string str = Util::get_mac();

	printf("test end! \n");
}
