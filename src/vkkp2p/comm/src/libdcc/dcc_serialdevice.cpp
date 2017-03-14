#include "dcc_serialdevice.h"
#include "basetypes.h"
#include "ascii.h"

dcc_serialdevice::dcc_serialdevice(void)
:m_brun(false)
{
}

dcc_serialdevice::~dcc_serialdevice(void)
{
}

int dcc_serialdevice::open(serial_Device_t& conf)
{
	Lock l(m_mt);
	if(m_file.is_open())
		return 1;
	m_conf = conf;
	if(0!=m_file.open(m_conf.device.c_str(),m_conf.BaudRate,m_conf.ByteSize,m_conf.StopBits,m_conf.Parity
		,m_conf.to.ReadIntervalTimeout,m_conf.to.ReadTotalTimeoutMultiplier,m_conf.to.ReadTotalTimeoutConstant
		,m_conf.to.WriteTotalTimeoutMultiplier,m_conf.to.WriteTotalTimeoutConstant))
	{
		printf("# ***%s(%d,%d,stop:%d,parity:%d) OPEN fail \n",m_conf.device.c_str(),m_conf.BaudRate,m_conf.ByteSize,m_conf.StopBits,m_conf.Parity);
		return -1;
	}
	printf("# %s(%d,%d,stop:%d,parity:%d) OPEN ok \n",m_conf.device.c_str(),m_conf.BaudRate,m_conf.ByteSize,m_conf.StopBits,m_conf.Parity);

	m_brun = true;
	//this->activate(); 暂不使用线程
	return 0;
}
int dcc_serialdevice::close()
{
	Lock l(m_mt);
	if(m_brun)
	{
		m_brun = false;
		wait();
		m_file.close();
	}
	return 0;
}
int dcc_serialdevice::work(int e)
{
	while(m_brun)
	{
	}
	return 0;
}
int dcc_serialdevice::call_cmd(list<int>& cmd_ids,string& outmsg)
{
	Lock l(m_mt);
	char msgbuf[1024];
	outmsg = "";
	if(!m_file.is_open())
	{
		if(0!=m_file.open(m_conf.device.c_str(),m_conf.BaudRate,m_conf.ByteSize,m_conf.StopBits,m_conf.Parity
			,m_conf.to.ReadIntervalTimeout,m_conf.to.ReadTotalTimeoutMultiplier,m_conf.to.ReadTotalTimeoutConstant
			,m_conf.to.WriteTotalTimeoutMultiplier,m_conf.to.WriteTotalTimeoutConstant))
		{
			sprintf(msgbuf,"<ERR_call code=\"21\" dev=\"%s:\" msg=\"open fail\"/>\r\n",m_conf.device.c_str());
			outmsg = msgbuf;
			return -1;
		}
	}

	int retcode = 0;
	string str;
	for(list<int>::iterator it=cmd_ids.begin();it!=cmd_ids.end();++it)
	{
		if(0!=call_cmd_i(*it,str))
			retcode = -1;
		outmsg += str;
		Sleep(100);
		
	}
	return retcode;
}
int dcc_serialdevice::call_cmd_i(int id,string& outmsg)
{
	char buf[1024];
	char msgbuf[1024];
	int retcode = 0;
	string str;
	bool bok;
	int jump_id;
	int pos = 0;
	serial_DeviceCommand_t *pcmd;
	outmsg = "";
	pcmd = find_cmd(id);
	if(NULL==pcmd)
	{
		sprintf(msgbuf,"<ERR_call code=\"22\" dev=\"%s:%d\" msg=\"unkown id\"/>\r\n",m_conf.device.c_str(),id);
		outmsg += msgbuf;
		retcode = -1;
	}
	else
	{
		//发送命令前，清空一下缓冲，以免收到垃圾
		m_file.clear_io();
		if(!send_cmd(pcmd->send_cmd_list,pcmd->send_delay_ms))
		{
			sprintf(msgbuf,"<ERR_call code=\"23\" dev=\"%s:%d\" send=\"%s\" okrecv=\"%s\" msg=\"send cmd fail\"/>\r\n",m_conf.device.c_str(),id,pcmd->sendstr.c_str(),pcmd->rr.resstr.c_str());
			outmsg += msgbuf;
			retcode = -1;
		}
		else
		{
			pcmd->rr.recv_src.copy(NULL,0);
			pcmd->rr.recv_str = "";
			if(pcmd->recv_maxnum > 0)
			{
				int n = m_file.read(buf,pcmd->recv_maxnum+20);
				if(n>0)
				{
					pcmd->rr.recv_src.copy(buf,n);
					if(DCC_DATA_CODER_HEX==pcmd->coder)
					{
						int len = 1024;
						char hex[1024];
						if(0==ascii_2_hexsz(hex,len,buf,n))
						{
							if(len>0)
							{
								hex[len] = '\0';
								pcmd->rr.recv_str = hex;
							}
						}
					}
					else
					{
						pcmd->rr.recv_str = pcmd->rr.recv_src.buffer();
					}
				}
			}
			
			bok = false;
			jump_id = -1;
			if(pcmd->rr.resls.empty())
			{
				//不校验结果，直接回返
				bok = true;
			}
			else
			{
				//检验结果
				list<serial_DeviceResulti_t>::iterator it;
				for(it=pcmd->rr.resls.begin();it!=pcmd->rr.resls.end();++it)
				{
					serial_DeviceResulti_t& dr = *it;
					if(dr.ls.empty() && pcmd->rr.recv_str.empty())
					{
						//允许空结果且没收到数据时
						bok = true;
						jump_id = dr.jump_id;
						break;
					}
					if(pcmd->rr.recv_str.empty() || dr.ls.empty())
						continue;
					bool notfind = false;
					for(list<serial_Resulti_t>::iterator rit=dr.ls.begin();rit!=dr.ls.end();++rit)
					{
						serial_Resulti_t& r = *rit;					
						if(r.pos>=0)
							pos = (int)pcmd->rr.recv_str.find(r.res,r.pos);
						else
							pos = (int)pcmd->rr.recv_str.find(r.res);
						if((r.pos>=0&&pos!=r.pos) || r.pos<0 && pos<0)
						{
							notfind = true;
							break;
						}
					}
					if(!notfind)
					{
						//匹配上
						bok = true;
						jump_id = dr.jump_id;
						break;
					}
				}
			}

			if(bok)
			{
				if(DCC_INVALID_ID_VALUE!=jump_id)
					sprintf(buf," jump=\"%d\"",jump_id);
				else
					buf[0] = '\0';
				sprintf(msgbuf,"<ok_call dev=\"%s:%d\" send=\"%s\" okrecv=\"%s\" recv=\"%s\"%s/>\r\n",m_conf.device.c_str(),id,pcmd->sendstr.c_str(),pcmd->rr.resstr.c_str(),pcmd->rr.recv_str.c_str(),buf);
				outmsg += msgbuf;
				if(DCC_INVALID_ID_VALUE!=jump_id)
				{
					Sleep(100);
					retcode = call_cmd_i(jump_id,str);
					outmsg += str;
					return retcode;
				}
			}
			else
			{
				if(DCC_INVALID_ID_VALUE!=pcmd->rr.err_jump_id)
					sprintf(buf," ERR_jump=\"%d\"",pcmd->rr.err_jump_id);
				else
					buf[0] = '\0';
				sprintf(msgbuf,"<ERR_call code=\"23\" dev=\"%s:%d\" send=\"%s\" okrecv=\"%s\" recv=\"%s\"%s/>\r\n",m_conf.device.c_str(),id,pcmd->sendstr.c_str(),pcmd->rr.resstr.c_str(),pcmd->rr.recv_str.c_str(),buf);
				outmsg += msgbuf;
				if(DCC_INVALID_ID_VALUE!=pcmd->rr.err_jump_id)
				{
					Sleep(100);
					retcode = call_cmd_i(pcmd->rr.err_jump_id,str);
					outmsg += str;
					return retcode;
				}
				else
				{
					retcode = -1;
				}
			}
		}
	}
	return retcode;
}
serial_DeviceCommand_t* dcc_serialdevice::find_cmd(int id)
{
	serial_DeviceCommand_t *p;
	list<serial_DeviceCommand_t>::iterator it;
	for(it=m_conf.cmdls.begin();it!=m_conf.cmdls.end();++it)
	{
		p = &(*it);
		if(id == p->id)
			return p;
	}
	return NULL;
}
bool dcc_serialdevice::send_cmd(list<CharBuffer>& ls,int delay_ms)
{
	int i=0;
	for(list<CharBuffer>::iterator it=ls.begin();it!=ls.end();++it)
	{
		CharBuffer& b = *it;
		printf("path=%s send buf=%s size=%d \n",m_conf.device.c_str(),b.buffer(),b.size());
		if(b.size()!=m_file.write(b.buffer(),b.size()))
		{
			printf("***send fail \n");
			return false;
		}
		printf("send ok \n");
		if(i++>0)
		{
			Sleep(delay_ms);
		}
	}
	return true;
}