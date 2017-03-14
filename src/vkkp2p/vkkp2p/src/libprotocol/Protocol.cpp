#include "Protocol.h"

PLTNetInfo_t g_netLiveInfo;
const char *g_str_usertype[6] = {"","client","server","http","center","center"};

int operator << (PTLStream& ss, const PTL_Head& inf)
{
	ss.write(PTL_HEAD_STX,4);
	ss << inf.size;
	ss << inf.mask;
	ss << inf.checksum;
	ss << inf.cmd;
	ss << inf.id;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_Head& inf)
{
	ss.read(inf.stx,4);
	ss >> inf.size;
	ss >> inf.mask;
	ss >> inf.checksum;
	ss >> inf.cmd;
	ss >> inf.id;
	return ss.ok();
}


