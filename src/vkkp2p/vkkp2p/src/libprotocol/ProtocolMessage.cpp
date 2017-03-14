#include "ProtocolMessage.h"



int operator << (SerialStream& ss, const PTL_MSG_DownloadListStart& inf)
{
	ss.write_string(inf.url);
	return ss.ok();
}
int operator >> (SerialStream& ss, PTL_MSG_DownloadListStart& inf)
{
	ss.read_string(inf.url,1024);
	return ss.ok();
}
int operator << (SerialStream& ss, const PTL_MSG_DownloadListStop& inf)
{
	ss.write_string(inf.url);
	return ss.ok();
}
int operator >> (SerialStream& ss, PTL_MSG_DownloadListStop& inf)
{
	ss.read_string(inf.url,1024);
	return ss.ok();
}

int operator << (SerialStream& ss, const PTL_MSG_DownloadStart& inf)
{
	ss.write_string(inf.url);
	return ss.ok();
}
int operator >> (SerialStream& ss, PTL_MSG_DownloadStart& inf)
{
	ss.read_string(inf.url,1024);
	return ss.ok();
}
int operator << (SerialStream& ss, const PTL_MSG_DownloadStop& inf)
{
	ss.write_string(inf.url);
	return ss.ok();
}
int operator >> (SerialStream& ss, PTL_MSG_DownloadStop& inf)
{
	ss.read_string(inf.url,1024);
	return ss.ok();
}


