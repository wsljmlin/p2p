//dcc---- device command control 

#pragma once

#include "SimpleString.h"


int dcc_init(const char* conf_xmlpath);
int dcc_fini();

//执行1个指令，返回0成功， outmsg返回执行相关的信息描述
int dcc_call(int call_id,string& outmsg);

