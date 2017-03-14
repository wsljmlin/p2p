#include "dcc_devicepool.h"

dcc_devicepool::dcc_devicepool(void)
:m_binit(false)
,m_bcalling(false)
{
}

dcc_devicepool::~dcc_devicepool(void)
{
}
int dcc_devicepool::init(const char* conf_xmlpath)
{
	Lock l(m_mt);
	if(m_binit)
		return 1;
	m_binit = true;
	m_conf.load_xml(conf_xmlpath);

	//init serialdevice
	list<serial_Device_t*>::iterator it;
	serial_Device_t *conf;
	dcc_serialdevice *sdev;
	for(it=m_conf.m_sdevls.begin();it!=m_conf.m_sdevls.end();++it)
	{
		conf = *it;
		sdev = new dcc_serialdevice();
		sdev->open(*conf);
		m_sdevmap[conf->device] = sdev; //load配置时确保COM不重复
	}
	return 0;
}

int dcc_devicepool::fini()
{
	Lock l(m_mt);
	if(!m_binit)
		return 0;
	m_binit = false;
	map<string,dcc_serialdevice*>::iterator it;
	for(it=m_sdevmap.begin();it!=m_sdevmap.end();++it)
	{
		it->second->close();
		delete it->second;
	}
	m_sdevmap.clear();
	m_conf.clear();
	return 0;
}


//执行1个指令，返回0成功， outmsg返回执行相关的信息描述
//同一时刻只能执行一个指令
int dcc_devicepool::call(int call_id,string& outmsg)
{
	char msgbuf[1024];
	outmsg = "";
	{
		Lock l(m_mt);
		if(!m_binit)
		{
			outmsg = "<ERR_call code=\"11\" msg=\"dcc not init\"/>\r\n";
			return -1;
		}
		if(m_bcalling)
		{
			outmsg = "<ERR_call code=\"12\" msg=\"dcc call busying\"/>\r\n";
			return -1;
		}
		m_bcalling = true;
	}

	//执行
	CallDevice_t* cd = m_conf.find_call(call_id);
	if(NULL==cd)
	{
		sprintf(msgbuf,"<ERR_call code=\"13\" call_id=\"%d\" msg=\"unkown call_id\"/>\r\n",call_id);
		outmsg = msgbuf;

		Lock l(m_mt);
		m_bcalling = false;
		return -1;
	}
	dcc_serialdevice* sdev;
	string str;
	int ret = 0;
	list<CallDevicei_t>::iterator it;
	for(it=cd->devls.begin();it!=cd->devls.end();++it)
	{
		CallDevicei_t& cdc = (*it);
		if(cdc.type == "serialport")
		{
			sdev = find_serialdevice(cdc.device);
			if(sdev)
			{
				if(0!=sdev->call_cmd(cdc.idls,str))
				{
					ret = -1;
				}
				outmsg += str;
				Sleep(cdc.delay_ms);
			}
			else
			{
				sprintf(msgbuf,"<ERR_call code=\"14\" call_id=\"%d\" dev=\"%s\" msg=\"unkown device\"/>\r\n",call_id,cdc.device.c_str());
				outmsg += msgbuf;
				ret = -1;
			}
		}
		else
		{
			sprintf(msgbuf,"<ERR_call code=\"15\" call_id=\"%d\" dev=\"%s\" device_type=\"%s\" msg=\"unkown device_type\"/>\r\n",call_id,cdc.device.c_str(),cdc.type.c_str());
			outmsg += msgbuf;
			ret = -1;
		}
	}
	
	{
		Lock l(m_mt);
		m_bcalling = false;
	}
	return ret;
}

dcc_serialdevice* dcc_devicepool::find_serialdevice(const string& device)
{
	map<string,dcc_serialdevice*>::iterator it = m_sdevmap.find(device);
	if(it!=m_sdevmap.end())
		return it->second;
	return NULL;
}
