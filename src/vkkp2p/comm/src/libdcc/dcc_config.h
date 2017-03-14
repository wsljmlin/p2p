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
	int pos; //-1 ��ʾ��ָ����ʼλƥ��
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
	CharBuffer						recv_src; //ʵ�ʽ���Դ��
	string							recv_str; //����code_typeת��
	string							resstr; //�����
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

//ִ��һ������ֻ����һ�Σ�Ȼ�����һ�ν��
typedef struct
{
	int								id;
	string							description;
	int								coder; //DCC_DATA_CODE_TYPE_ASCII=Դ�룬DCC_DATA_CODE_TYPE_HDEX=16������ʾ�ַ�
	int								send_delay_ms; //���Ͷ�����ʹ�ã����ǰһ��������ʱ����,ԭ��
	string							sendstr; //���ʹ�
	list<CharBuffer>				send_cmd_list;
	int								recv_maxnum;  //���Խ������������
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

	int				BaudRate; //������
	unsigned char	ByteSize; //λ����һ��4-8λ
	unsigned char	StopBits; //ֹͣλ��־:ONESTOPBIT=0��ʾ1λ��1��ʾ1.5λ��2��ʾ2λ
	unsigned char	Parity;  //У���־: NOPARITY=0 ��N�� �޻�żУ��

	serial_DeviceTimeOuts_t		to;
	list<serial_DeviceCommand_t> cmdls;

	tag_serial_Device(void)
		:BaudRate(9600)
		,ByteSize(8)
		,StopBits(0)
		,Parity(0)
	{
		to.ReadIntervalTimeout = 300; //����2�ֽ�֮������������һ���ֽڶ�û���յ����ò������
		to.ReadTotalTimeoutMultiplier = 20; //��ÿ���ֽ�ƽ����ʱϵͳ,
		to.ReadTotalTimeoutConstant = 500; //�ܳ�ʱ����
		//д��ʱ:��дn�ֽڣ���ʱΪn*50+1000,10�ֽ�Ϊ1500���볬ʱ
		to.WriteTotalTimeoutMultiplier = 50; //д��ʱϵͳ
		to.WriteTotalTimeoutConstant = 500; //����
	}
}serial_Device_t;


//************************************** serial end *************************************************
typedef struct tag_CallDevicei
{
	string			type;
	string			device;
	list<int>		idls;
	unsigned int	delay_ms; //ִ�����˯��tick
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
