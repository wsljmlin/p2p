
#include <stdio.h>


#include "test_serialport.h"

#define TRACE printf
int main(int argc,char** argv)
{

	test_serialport_main(argc,argv);

	getchar();
	return 0;
}

