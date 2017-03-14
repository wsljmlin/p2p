#include "ProtocolP2S.h"


int operator << (PTLStream& ss, const PTL_P2S_RequestTcpCheck& inf)
{
	ss << inf.tcpPort;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_RequestTcpCheck& inf)
{
	ss >> inf.tcpPort;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_ResponseTcpCheck& inf)
{
	ss << inf.result;
	ss << inf.id;
	ss << inf.eyeIP;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_ResponseTcpCheck& inf)
{
	ss >> inf.result;
	ss >> inf.id;
	ss >> inf.eyeIP;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_ResponseServer& inf)
{
	ss << inf.result;
	ss << inf.eyeIP;
	ss << inf.eyePort;
	ss << inf.stunIPA;
	ss << inf.stunIPB;
	ss << inf.stunPortA;
	ss << inf.stunPortB;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_ResponseServer& inf)
{
	ss >> inf.result;
	ss >> inf.eyeIP;
	ss >> inf.eyePort;
	ss >> inf.stunIPA;
	ss >> inf.stunIPB;
	ss >> inf.stunPortA;
	ss >> inf.stunPortB;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_ResponseNat1& inf)
{
	ss << inf.desIP;
	ss << inf.desPort;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_ResponseNat1& inf)
{
	ss >> inf.desIP;
	ss >> inf.desPort;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_ResponseNat2& inf)
{
	ss << inf.desIP;
	ss << inf.desPort;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_ResponseNat2& inf)
{
	ss >> inf.desIP;
	ss >> inf.desPort;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_RequestNat4& inf)
{
	ss << inf.flag;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_RequestNat4& inf)
{
	ss >> inf.flag;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_ResponseNat4& inf)
{
	ss << inf.flag;
	ss << inf.eyeIP;
	ss << inf.eyePort;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_ResponseNat4& inf)
{
	ss >> inf.flag;
	ss >> inf.eyeIP;
	ss >> inf.eyePort;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_ResponsePulse& inf)
{
	ss << inf.eyeIP;
	ss << inf.eyePort;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_ResponsePulse& inf)
{
	ss >> inf.eyeIP;
	ss >> inf.eyePort;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2S_ResponseConfig& inf)
{
	ss << inf.eyeIP;
	ss << inf.eyePort;
	ss << inf.accept_ip;
	ss << inf.accept_port;
	ss << inf.accept_port2;
	ss << inf.stunA_ip;
	ss << inf.stunA_port;
	ss << inf.stunB_ip;
	ss << inf.stunB_port;
	ss << inf.stunB_port2;
	ss << inf.is_stunB_port_rspnat1;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2S_ResponseConfig& inf)
{
	ss >> inf.eyeIP;
	ss >> inf.eyePort;
	ss >> inf.accept_ip;
	ss >> inf.accept_port;
	ss >> inf.accept_port2;
	ss >> inf.stunA_ip;
	ss >> inf.stunA_port;
	ss >> inf.stunB_ip;
	ss >> inf.stunB_port;
	ss >> inf.stunB_port2;
	ss >> inf.is_stunB_port_rspnat1;
	return ss.ok();
}

