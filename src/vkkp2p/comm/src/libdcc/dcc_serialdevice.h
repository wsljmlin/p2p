#pragma once
#include "dcc_config.h"
#include "SerialPortFile.h"
#include "Thread.h"
#include "SynchroObj.h"

class dcc_serialdevice : public Thread
{
public:
	dcc_serialdevice(void);
	virtual ~dcc_serialdevice(void);

	typedef Simple_Mutex Mutex;
	typedef TLock<Simple_Mutex> Lock;
public:
	int open(serial_Device_t& conf);
	int close();
	virtual int work(int e);

	int call_cmd(list<int>& cmd_ids,string& outmsg);
	int call_cmd_i(int id,string& outmsg);
private:
	serial_DeviceCommand_t* find_cmd(int id);
	bool send_cmd(list<CharBuffer>& ls,int delay_ms);
private:
	bool m_brun;
	Mutex m_mt;
	serial_Device_t m_conf;
	SerialPortFile m_file;
};
