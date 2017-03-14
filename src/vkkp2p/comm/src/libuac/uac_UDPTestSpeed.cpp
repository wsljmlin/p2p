#include "uac_UDPTestSpeed.h"

namespace UAC
{
int operator << (PTLStream& ps, const UDPS_ptl_TestSpeed_data_t& inf)
{
	ps << inf.sid;
	ps << inf.size;
	ps << inf.seq;
	return ps.ok();
}
int operator >> (PTLStream& ps, UDPS_ptl_TestSpeed_data_t& inf)
{
	ps >> inf.sid;
	ps >> inf.size;
	ps >> inf.seq;
	return ps.ok();
}

int operator << (PTLStream& ps, const UDPS_ptl_TestSpeed_ack_t& inf)
{
	ps << inf.sid;
	ps << inf.size;
	ps << inf.seq;
	ps << inf.speedB;
	return ps.ok();
}
int operator >> (PTLStream& ps, UDPS_ptl_TestSpeed_ack_t& inf)
{
	ps >> inf.sid;
	ps >> inf.size;
	ps >> inf.seq;
	ps >> inf.speedB;
	return ps.ok();
}

}


