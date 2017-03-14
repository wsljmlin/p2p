#include "dcc_config.h"
#include "ICXml.h"
#include "Util.h"
#include "ascii.h"

dcc_config::dcc_config(void)
:m_httpport(8080)
{
}

dcc_config::~dcc_config(void)
{
}

int dcc_config::load_xml(const char* path)
{
	clear();
	ICXml xml;
	XMLNode *node,*node2;
	if(0!=xml.load_file(path))
		return -1;

	//http port
	m_httpport = 0;
	node = xml.find_first_node("dcc_config/http_port");
	if(node)
	{
		if(node->get_data()) m_httpport = atoi(node->get_data());
	}
	if(0==m_httpport) m_httpport = 8080;

	//devices
	const char* sz;
	serial_Device_t *sdev;
	serial_DeviceCommand_t sdc;
	serial_DeviceResulti_t dr;
	serial_Resulti_t r;
	CharBuffer cbuf;
	string device_path,str,str2,str3,str4;
	int n,m,len,id;
	int i=0,j=0;
	int imax;
	char buf[1024];
	node = xml.find_first_node("dcc_config/devices/serialport/device");
	for(;node;node=node->next())
	{
		device_path = node->attri.get_attri("path");
		if(device_path.empty()||find_sdev(device_path))
			continue;
		sdev = new serial_Device_t();
		sdev->device = device_path;

		sz = node->attri.get_attri("BaudRate");
		if(sz) sdev->BaudRate = atoi(sz);
		sz = node->attri.get_attri("ByteSize");
		if(sz) sdev->ByteSize = atoi(sz);
		sz = node->attri.get_attri("StopBits");
		if(sz) sdev->StopBits = atoi(sz);
		sz = node->attri.get_attri("Parity");
		if(sz) sdev->Parity = atoi(sz);

		//timeout
		sz = node->attri.get_attri("to_RInterval");
		if(sz) sdev->to.ReadIntervalTimeout = atoi(sz);
		sz = node->attri.get_attri("to_RTMultiplier");
		if(sz) sdev->to.ReadTotalTimeoutMultiplier = atoi(sz);
		sz = node->attri.get_attri("to_RTConstant");
		if(sz) sdev->to.ReadTotalTimeoutConstant = atoi(sz);
		sz = node->attri.get_attri("to_WTMultiplier");
		if(sz) sdev->to.WriteTotalTimeoutMultiplier = atoi(sz);
		sz = node->attri.get_attri("to_WTConstant");
		if(sz) sdev->to.WriteTotalTimeoutConstant = atoi(sz);
		
		node2 = node->child();
		for(;node2;node2 = node2->next())
		{
			if(0!=strcmp("cmd",node2->get_tag()))
				continue;
			sdc.reset();
			sz = node2->attri.get_attri("id");
			if(NULL==sz) continue;
			sdc.id = atoi(sz);
			sdc.description = node2->attri.get_attri("description");
			sz = node2->attri.get_attri("coder");
			if(sz && 0==strcmp(sz,"hex")) sdc.coder = DCC_DATA_CODER_HEX;
			sz = node2->attri.get_attri("send_delay_ms");
			if(sz) sdc.send_delay_ms = atoi(sz);
			sz = node2->attri.get_attri("recv_maxnum");
			if(sz) sdc.recv_maxnum = atoi(sz);
			sz = node2->attri.get_attri("err_jump_id");
			if(sz&&strlen(sz)>0) sdc.rr.err_jump_id = atoi(sz);
			
			//send data:
			str = node2->attri.get_attri("send");
			sdc.sendstr = str;
			n = Util::get_string_index_count(str,",");
			for(i=0;i<n;++i)
			{
				str2 = Util::get_string_index(str,i,",");
				if(str2.empty()) continue;
				len = 1024;
				if(DCC_DATA_CODER_HEX == sdc.coder)
				{
					if(0==hexsz_2_ascii(buf,len,str2.c_str(),(int)str2.length()))
					{
						if(len>0)
						{
							cbuf.copy(buf,len);
							sdc.send_cmd_list.push_back(cbuf);
						}
					}
				}
				else
				{
					cbuf.copy(str2.c_str(),(int)str2.length());
					sdc.send_cmd_list.push_back(cbuf);
				}
				
			}
			if(sdc.send_cmd_list.empty())
				continue;

			//recv result
			//recv="Res1:Pos1#Res2=Jump1,R2:=,R3"
			
			imax = 0;
			str = node2->attri.get_attri("recv");
			sdc.rr.resstr = str;
			n = Util::get_string_index_count(str,",");
			for(i=0;i<n;++i)
			{
				dr.reset();
				str2 = Util::get_string_index(str,i,",");
				//jump id
				str3 = Util::get_string_index(str2,1,"=");
				if(!str3.empty()) dr.jump_id = atoi(str3.c_str());
				
				//多个&结果,用#分隔
				str3 = Util::get_string_index(str2,0,"=");
				if(imax<(int)str3.length()) imax = (int)str3.length(); //与的总长作为参考接收字节数，接收时会在此直再+20
				m = Util::get_string_index_count(str3,"#");
				for(j=0;j<m;j++)
				{
					r.reset();
					str2 = Util::get_string_index(str3,j,"#");
					r.res = Util::get_string_index(str2,0,":"); //匹配串
					if(r.res.empty()) continue; //不允为空
					str4 = Util::get_string_index(str2,1,":"); //pos匹配位置；
					if(!str4.empty()) r.pos = atoi(str4.c_str());
					dr.ls.push_back(r);
				}

				//if(dr.ls.empty()) continue; //允许为空
				sdc.rr.resls.push_back(dr);
			}
			if(DCC_DATA_CODER_HEX == sdc.coder)
					imax /=2;
			if(sdc.recv_maxnum < imax) sdc.recv_maxnum = imax;
			sdev->cmdls.push_back(sdc);
		}
		
		m_sdevls.push_back(sdev);	
	}

	//call
	CallDevice_t *pcd;
	CallDevicei_t cd;
	node = xml.find_first_node("dcc_config/calls/call");
	for(;node;node=node->next())
	{
		sz = node->attri.get_attri("id");
		if(NULL==sz) continue;
		id = atoi(sz);
		if(m_callmap.find(id)!=m_callmap.end())
			continue;
		pcd = new CallDevice_t();
		pcd->call_id = id;

		node2 = node->child();
		for(;node2;node2 = node2->next())
		{
			if(0!=strcmp("device",node2->get_tag()))
				continue;
			cd.reset();
			cd.type = node2->attri.get_attri("type");
			cd.device = node2->attri.get_attri("path");
			str = node2->attri.get_attri("delay_ms");
			if(!str.empty()) cd.delay_ms = atoi(str.c_str());
			str = node2->attri.get_attri("cmd_ids");
			n = Util::get_string_index_count(str,",");
			for(int i=0;i<n;++i)
			{
				str2 = Util::get_string_index(str,i,",");
				if(str2.empty()) continue;
				id = atoi(str2.c_str());
				cd.idls.push_back(id);
			}
			if(cd.type.empty() || cd.device.empty() || cd.idls.empty())
				continue;
			pcd->devls.push_back(cd);
		}

		m_callmap[pcd->call_id] = pcd;

	}
	return 0;
}
void dcc_config::clear()
{
	{
		map<int,CallDevice_t*>::iterator it;
		for(it=m_callmap.begin();it!=m_callmap.end();++it)
			delete it->second;
		m_callmap.clear();
	}
	{
		list<serial_Device_t*>::iterator it;
		for(it=m_sdevls.begin();it!=m_sdevls.end();++it)
			delete (*it);
		m_sdevls.clear();
	}
}
CallDevice_t* dcc_config::find_call(int call_id)
{
	map<int,CallDevice_t*>::iterator it = m_callmap.find(call_id);
	if(it!=m_callmap.end())
		return it->second;
	return NULL;
}
serial_Device_t* dcc_config::find_sdev(const string& device_path)
{
	for(list<serial_Device_t*>::iterator it=m_sdevls.begin();it!=m_sdevls.end();++it)
	{
		if(device_path == (*it)->device)
			return *it;
	}
	return NULL;
}