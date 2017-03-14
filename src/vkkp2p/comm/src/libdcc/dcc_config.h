#pragma once

#include "cyclist.h"
#include "rbtmap.h"
#include "SimpleString.h"
#include "CharBuffer.h"
#pragma warning(disable:4996)

#define DCC_MAX_CMD_LENGTH 256

#define DCC_INVALID_ID_VALUE ((int)-1)

#define DCC_DATA_CODER_NULL 0
#define DCC_DATA_CODER_HEX 1

//************************************** serial beg *************************************************
typedef struct
{
	string res;
	int pos; //-1 表示不指定起始位匹配
	void reset()
	{
		res = "";
		pos = -1;
	}
}serial_Resulti_t;
typedef struct tag_serial_DeviceResulti
{
	list<serial_Resulti_t> ls;
	int jump_id; // 
	void reset()
	{
		ls.clear();
		jump_id = DCC_INVALID_ID_VALUE;
	}
}serial_DeviceResulti_t;
typedef struct tag_serial_DeviceResult
{
	CharBuffer						recv_src; //实际接收源码
	string							recv_str; //根据code_type转码
	string							resstr; //结果串
	list<serial_DeviceResulti_t>	resls;
	int								err_jump_id;
	void reset()
	{
		recv_src.copy(NULL,0);
		recv_str = "";
		resstr = "";	
		resls.clear();
		err_jump_id = DCC_INVALID_ID_VALUE;
	}
}serial_DeviceResult_t;

//执行一次命令只发送一次，然后接收一次结果
typedef struct
{
	int								id;
	string							description;
	int								coder; //DCC_DATA_CODE_TYPE_ASCII=源码，DCC_DATA_CODE_TYPE_HDEX=16进制显示字符
	int								send_delay_ms; //发送队列里使用，相对前一个包的延时毫秒,原码
	string							sendstr; //发送串
	list<CharBuffer>				send_cmd_list;
	int								recv_maxnum;  //尝试接收最大结果个数
	serial_DeviceResult_t			rr;
	void reset()
	{
		id = 0;
		description = "";
		coder = DCC_DATA_CODER_NULL;
		send_delay_ms = 200;
		sendstr = "";
		send_cmd_list.clear();
		recv_maxnum = 0;
		rr.reset();
	}
}serial_DeviceCommand_t;

typedef struct tag_serial_DeviceTimeOuts 
{
    unsigned int ReadIntervalTimeout;          /* Maximum time between read chars. */
    unsigned int ReadTotalTimeoutMultiplier;   /* Multiplier of characters.        */
    unsigned int ReadTotalTimeoutConstant;     /* Constant in milliseconds.        */
    unsigned int WriteTotalTimeoutMultiplier;  /* Multiplier of characters.        */
    unsigned int WriteTotalTimeoutConstant;    /* Constant in milliseconds.        */
}serial_DeviceTimeOuts_t;

typedef struct tag_serial_Device
{
	string			device; //"COM1"

	int				BaudRate; //波特率
	unsigned char	ByteSize; //位数，一般4-8位
	unsigned char	StopBits; //停止位标志:ONESTOPBIT=0表示1位，1表示1.5位，2表示2位
	unsigned char	Parity;  //校验标志: NOPARITY=0 （N） 无机偶校验

	serial_DeviceTimeOuts_t		to;
	list<serial_DeviceCommand_t> cmdls;

	tag_serial_Device(void)
		:BaudRate(9600)
		,ByteSize(8)
		,StopBits(0)
		,Parity(0)
	{
		to.ReadIntervalTimeout = 300; //接收2字节之间最大间隔，如果一个字节都没有收到就用不到这个
		to.ReadTotalTimeoutMultiplier = 20; //读每个字节平均超时系统,
		to.ReadTotalTimeoutConstant = 500; //总超时常量
		//写超时:如写n字节，则超时为n*50+1000,10字节为1500毫秒超时
		to.WriteTotalTimeoutMultiplier = 50; //写超时系统
		to.WriteTotalTimeoutConstant = 500; //常量
	}
}serial_Device_t;


//************************************** serial end *************************************************
typedef struct tag_CallDevicei
{
	string			type;
	string			device;
	list<int>		idls;
	unsigned int	delay_ms; //执行完后睡眠tick
	void reset()
	{
		type = "";
		device = "";
		idls.clear();
		delay_ms = 0;
	}
}CallDevicei_t;

typedef struct
{
	int call_id;
	list<CallDevicei_t> devls;
}CallDevice_t;


class dcc_config
{
public:
	dcc_config(void);
	~dcc_config(void);
public:
	unsigned short m_httpport;
	list<serial_Device_t*> m_sdevls;
	map<int,CallDevice_t*> m_callmap;

public:
	int load_xml(const char* path);
	void clear();

	CallDevice_t* find_call(int call_id);
	serial_Device_t* find_sdev(const string& device_path);
};
