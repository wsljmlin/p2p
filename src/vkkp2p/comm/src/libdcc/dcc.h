//dcc---- device command control 

#pragma once

#include "SimpleString.h"


int dcc_init(const char* conf_xmlpath);
int dcc_fini();

//ִ��1��ָ�����0�ɹ��� outmsg����ִ����ص���Ϣ����
int dcc_call(int call_id,string& outmsg);

