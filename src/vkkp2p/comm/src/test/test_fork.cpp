#include "test_fork.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

void sig_handler(int sig)
{
	switch(sig)
	{
	case 999:
		printf(" signal 999 exiting....! pid=%d \n",getpid());
		exit(0);
		break;
	default:
		break;
	}

}

void* func_1(void*)
{
	int i=0;
	while(1)
	{
		printf(" %d: i=%d \n",getpid(),i++);
		sleep(1);
	}
	return 0;
}


void* func_2(void*)
{
	//
	int pid;
	pid = fork();
	if(pid<0)
	{
		perror("fork faild");
	}
	else if(0==pid)
	{
		printf("this is child process , my_pid = %d (perentid=%d) \n",getpid(),getppid());
		
		pthread_t h = 0;
		pthread_create(&h,NULL,func_1,NULL);
		pthread_detach(h);
		while(1!=getppid())
			sleep(0);
		printf(" child process wait end!!! parent is out \n");
		exit(0);
	}
	else 
	{
		signal(999,sig_handler);
		printf(" parent process, try waiting child process, child_pid=%d \n",pid);
		int ret = waitpid(pid,NULL,0);
		printf(" parent process wait end!!! %d = waitpid() \n",ret);
	}
	return 0;
}



int test_fork_main(int argc,char** argv)
{
	if(2==argc && 0==strcmp("-q",argv[1]))
	{
		int ret = kill(0,999);

		return 0;
	}
	//先创建一个线程
	pthread_t h = 0;
	pthread_create(&h,NULL,func_1,NULL);
	pthread_detach(h);

	pthread_create(&h,NULL,func_2,NULL);
	pthread_join(h,NULL);
	
	printf("exiting pid=%d \n",getpid());
	return 0;
}

int main(int argc,char** argv)
{
	return test_fork_main(argc,argv);
}

#endif
