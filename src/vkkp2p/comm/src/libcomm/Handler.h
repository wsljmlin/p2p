#pragma once


#define SE_READ   0x01
#define SE_WRITE  0x02
#define SE_BOTH   0x03

class SockHandler
{
public:
	SockHandler(void) : __i(-1) {}
	virtual ~SockHandler(void){}

	virtual int sock()=0;
	virtual int handle_input()=0;
	virtual int handle_output()=0;
	virtual int handle_error()=0;
public:
	int __i;
};

class TimerHandler
{
public:
	TimerHandler(void) {}
	virtual ~TimerHandler(void){}

	virtual void on_timer(int e){}
};
