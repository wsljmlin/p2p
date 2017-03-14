#pragma once
#include "basetypes.h"

namespace DIPCache
{
string findip(const char* domain);
void addip(const char* domain,const string& iplist);
void load_hosts_file( const string & hosts_file );
};
