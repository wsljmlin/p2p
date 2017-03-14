#include "test_serialport.h"
#include <stdio.h>
#include "SerialPortFile.h"
#pragma warning(disable:4996)

int test_serialport_main(int argc,char** argv)
{

	SerialPortFile sp;
	if(argc<3)
	{
		printf("please use \" %s [path] [rsp_string] \"\n",argv[0]);
		return 0;
	}
	printf("%s %s %s : \n",argv[0],argv[1],argv[2]);
	char rsp[1024];
	char buf[1024];
	int n ;
	int rsplen = (int)strlen(argv[2]);
	strcpy(rsp,argv[2]);
	if(0==sp.open(argv[1]))
	{
		printf(" open %s ok:\n",argv[1]);
		
		while(1)
		{
			if((n=sp.read(buf,1024))>0)
			{
				buf[n] = '\0';
				printf(" recv data: %s \n",buf);
				if(rsplen ==sp.write(rsp,rsplen))
				{
					printf(" send data: %s => ok\n",rsp);
				}
				else
				{
					printf(" ***send data: %s => fail\n",rsp);
				}
			}
		}

		sp.close();
	}
	else
	{
		printf(" ***open %s fail:\n",argv[1]);
	}
	
	return 0;
}

