#include "ProtocolP2T.h"

int operator << (SerialStream& ss, const PTL_P2T_FileInfo& inf)
{
	ss << inf.fhash;
	ss << inf.fsize;
	return ss.ok();
}
int operator >> (SerialStream& ss, PTL_P2T_FileInfo& inf)
{
	ss >> inf.fhash;
	ss >> inf.fsize;
	return ss.ok();
}
int operator << (SerialStream& ss, const PTL_P2T_PeerInfo& inf)
{
	ss << inf.trackID;
	ss << inf.sessionID;
	ss << inf.utype;
	ss << inf.ntype;
	ss << inf.menu;
	ss << inf.tcpLocalIP;
	ss << inf.tcpRealIP;
	ss << inf.tcpLocalPort;
	ss << inf.tcpRealPort;
	ss << inf.udpLocalIP;
	ss << inf.udpRealIP;
	ss << inf.udpLocalPort;
	ss << inf.udpRealPort;
	return ss.ok();
}
int operator >> (SerialStream& ss, PTL_P2T_PeerInfo& inf)
{
	ss >> inf.trackID;
	ss >> inf.sessionID;
	ss >> inf.utype;
	ss >> inf.ntype;
	ss >> inf.menu;
	ss >> inf.tcpLocalIP;
	ss >> inf.tcpRealIP;
	ss >> inf.tcpLocalPort;
	ss >> inf.tcpRealPort;
	ss >> inf.udpLocalIP;
	ss >> inf.udpRealIP;
	ss >> inf.udpLocalPort;
	ss >> inf.udpRealPort;
	return ss.ok();
}
int operator << (SerialStream& ss, const PTL_P2T_ServerInfo& inf)
{
	ss << inf.uid;
	ss << inf.ver;
	ss << inf.ntype;
	ss << inf.utype;
	ss << inf.menu;
	ss << inf.sessionID;
	ss << inf.beginTime;
	ss << inf.sourceNum;
	ss << inf.tcpRealIP;
	ss << inf.udpRealIP;
	ss << inf.tcpRealPort;
	ss << inf.udpRealPort;
	ss.write((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator >> (SerialStream& ss, PTL_P2T_ServerInfo& inf)
{
	ss >> inf.uid;
	ss >> inf.ver;
	ss >> inf.ntype;
	ss >> inf.utype;
	ss >> inf.menu;
	ss >> inf.sessionID;
	ss >> inf.beginTime;
	ss >> inf.sourceNum;
	ss >> inf.tcpRealIP;
	ss >> inf.udpRealIP;
	ss >> inf.tcpRealPort;
	ss >> inf.udpRealPort;
	ss.read((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}

//*****************************************************************************

int operator << (PTLStream& ss, const PTL_P2T_RequestLogin& inf)
{
	ss << inf.uid;
	ss << inf.utype;
	ss << inf.menu;
	ss << inf.ver;
	ss << inf.sysver;
	ss.write((void*)inf.mac,sizeof(inf.mac));
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_RequestLogin& inf)
{
	ss >> inf.uid;
	ss >> inf.utype;
	ss >> inf.menu;
	ss >> inf.ver;
	ss >> inf.sysver;
	ss.read((void*)inf.mac,sizeof(inf.mac));
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ResponseLogin& inf)
{
	ss << inf.result;
	ss << inf.uid;
	ss << inf.trackID;
	ss << inf.trackVer;
	ss << inf.sessionID;
	ss << inf.eyeIP;
	ss.write((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ResponseLogin& inf)
{
	ss >> inf.result;
	ss >> inf.uid;
	ss >> inf.trackID;
	ss >> inf.trackVer;
	ss >> inf.sessionID;
	ss >> inf.eyeIP;
	ss.read((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
//int operator << (PTLStream& ss, const PTL_P2T_ReportExit& inf)
//{
//	return ss.ok();
//}
//int operator >> (PTLStream& ss, PTL_P2T_ReportExit& inf)
//{
//	return ss.ok();
//}
int operator << (PTLStream& ss, const PTL_P2T_ReportNat& inf)
{
	ss << inf.ntype;
	ss << inf.tcpLocalIP;
	ss << inf.tcpRealIP;
	ss << inf.tcpLocalPort;
	ss << inf.tcpRealPort;
	ss << inf.udpLocalIP;
	ss << inf.udpRealIP;
	ss << inf.udpLocalPort;
	ss << inf.udpRealPort;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportNat& inf)
{
	ss >> inf.ntype;
	ss >> inf.tcpLocalIP;
	ss >> inf.tcpRealIP;
	ss >> inf.tcpLocalPort;
	ss >> inf.tcpRealPort;
	ss >> inf.udpLocalIP;
	ss >> inf.udpRealIP;
	ss >> inf.udpLocalPort;
	ss >> inf.udpRealPort;
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportShareFile& inf)
{
	ss << inf.num;
	ss.write_array(inf.files,inf.num);
#ifdef SM_VOD
	ss << inf.filetype;
#endif
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportShareFile& inf)
{
	ss >> inf.num;
	if(inf.num>20) inf.num = 20;
	ss.read_array(inf.files,inf.num);
#ifdef SM_VOD
	ss >> inf.filetype;
#endif
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportRemoveFile& inf)
{
	ss << inf.num;
	ss.write_array(inf.files,inf.num);
#ifdef SM_VOD
	ss << inf.filetype;
#endif
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportRemoveFile& inf)
{
	ss >> inf.num;
	if(inf.num>20) inf.num = 20;
	ss.read_array(inf.files,inf.num);
#ifdef SM_VOD
	ss >> inf.filetype;
#endif
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportStartDownloadFile& inf)
{
	ss << inf.fhash;
#ifdef SM_VOD
	ss << inf.filetype;
#endif
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportStartDownloadFile& inf)
{
	ss >> inf.fhash;
#ifdef SM_VOD
	ss >> inf.filetype;
#endif
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportStopDownloadFile& inf)
{
	ss << inf.fhash;
#ifdef SM_VOD
	ss << inf.filetype;
#endif
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportStopDownloadFile& inf)
{
	ss >> inf.fhash;
#ifdef SM_VOD
	ss >> inf.filetype;
#endif
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_RequestFileSource& inf)
{
	ss << inf.fhash;
	ss << inf.maxnum;
#ifdef SM_VOD
	ss << inf.filetype;
#endif
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_RequestFileSource& inf)
{
	ss >> inf.fhash;
	ss >> inf.maxnum;
#ifdef SM_VOD
	ss >> inf.filetype;
#endif
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ResponseFileSource& inf)
{
	ss << inf.result;
	ss << inf.fhash;
	ss << inf.fsize;
	ss << inf.urlflag;
	ss << inf.num;
	ss.write_array(inf.peers,inf.num);
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ResponseFileSource& inf)
{
	ss >> inf.result;
	ss >> inf.fhash;
	ss >> inf.fsize;
	ss >> inf.urlflag;
	ss >> inf.num;
	if(inf.num>20) inf.num=20;
	ss.read_array(inf.peers,inf.num);
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadWrong& inf)
{
	ss << inf.fhash;
	ss.write((void*)inf.fhashnew,sizeof(inf.fhashnew));
#ifdef SM_VOD
	ss << inf.filetype;
#endif
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadWrong& inf)
{
	ss >> inf.fhash;
	ss.read((void*)inf.fhashnew,sizeof(inf.fhashnew));
#ifdef SM_VOD
	ss >> inf.filetype;
#endif

	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadFileSpeed& inf)
{
	ss << inf.fhash;
	ss << inf.size;
	ss << inf.speed;
	ss << inf.cacheTimes;
	ss << inf.dragTimes;
	ss << inf.downSeconds;
	ss << inf.cacheSenconds;
	ss.write((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadFileSpeed& inf)
{
	ss >> inf.fhash;
	ss >> inf.size;
	ss >> inf.speed;
	ss >> inf.cacheTimes;
	ss >> inf.dragTimes;
	ss >> inf.downSeconds;
	ss >> inf.cacheSenconds;
	ss.read((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportStat& inf)
{
	ss.write_array(inf.connSucceedPerNetT,5);
	ss.write_array(inf.connFailedPerNetT,5);
	ss.write_array(inf.downBytesPerIPT_KB,3);
	ss.write_array(inf.shareBytesPerIPT_KB,3);
	ss.write_array(inf.downBytesPerUserT_KB,6);
	ss << inf.shareSeconds;
	ss << inf.downSeconds;
	ss.write((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportStat& inf)
{
	ss.read_array(inf.connSucceedPerNetT,5);
	ss.read_array(inf.connFailedPerNetT,5);
	ss.read_array(inf.downBytesPerIPT_KB,3);
	ss.read_array(inf.shareBytesPerIPT_KB,3);
	ss.read_array(inf.downBytesPerUserT_KB,6);
	ss >> inf.shareSeconds;
	ss >> inf.downSeconds;
	ss.read((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ReportError& inf)
{
	ss << inf.dumpTimes;
	ss << inf.downWrongBlocks;
	ss.write((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportError& inf)
{
	ss >> inf.dumpTimes;
	ss >> inf.downWrongBlocks;
	ss.read((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_RequestConnTurn& inf)
{
	ss << inf.desTrackID;
	ss << inf.desSessionID;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_RequestConnTurn& inf)
{
	ss >> inf.desTrackID;
	ss >> inf.desSessionID;
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ResponseConnTurn& inf)
{
	ss << inf.desPeerInfo;
	ss.write((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ResponseConnTurn& inf)
{
	ss >> inf.desPeerInfo;
	ss.read((void*)inf.reserve,sizeof(inf.reserve));
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2T_RequestServerList& inf)
{
	ss << inf.maxnum;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_RequestServerList& inf)
{
	ss >> inf.maxnum;
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_ResponseServerList& inf)
{
	ss << inf.trackID;
	ss << inf.trackVer;
	ss << inf.userNum;
	ss << inf.beginTime;
	ss << inf.allNum;
	ss << inf.startNum;
	ss << inf.num;
	ss.write_array(inf.servers,inf.num);
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ResponseServerList& inf)
{
	ss >> inf.trackID;
	ss >> inf.trackVer;
	ss >> inf.userNum;
	ss >> inf.beginTime;
	ss >> inf.allNum;
	ss >> inf.startNum;
	ss >> inf.num;
	if(inf.num>14) inf.num = 14;
	ss.read_array(inf.servers,inf.num);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadFileInfo& inf)
{
	ss << inf.flag;
	ss << inf.fhash;
	ss << inf.size;
	ss << inf.speed_KB;
	ss << inf.downSeconds;
	ss << inf.dragTimes;
	ss << inf.cacheTimes;
	ss << inf.cacheSenconds;
	ss.write_array(inf.connSucceedPerNetT,5);
	ss.write_array(inf.connFailedPerNetT,5);
	ss.write_array(inf.shareBytesPerIPT_KB,2);
	ss.write_array(inf.downBytesPerIPT_KB,3);
	ss.write_array(inf.downBytesPerUserT_KB,6);
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadFileInfo& inf)
{
	ss >> inf.flag;
	ss >> inf.fhash;
	ss >> inf.size;
	ss >> inf.speed_KB;
	ss >> inf.downSeconds;
	ss >> inf.cacheTimes;
	ss >> inf.dragTimes;
	ss >> inf.cacheSenconds;
	ss.read_array(inf.connSucceedPerNetT,5);
	ss.read_array(inf.connFailedPerNetT,5);
	ss.read_array(inf.shareBytesPerIPT_KB,2);
	ss.read_array(inf.downBytesPerIPT_KB,3);
	ss.read_array(inf.downBytesPerUserT_KB,6);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2T_ReportStartDownloadList& inf)
{
	ss << inf.fhash;
	ss.write_string(inf.url);
#ifdef SM_VOD
	ss << inf.filetype;
#endif /* end of SM_VOD */
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportStartDownloadList& inf)
{
	ss >> inf.fhash;
	ss.read_string(inf.url,1024);
#ifdef SM_VOD
	ss >> inf.filetype;
#endif /* end of SM_VOD */

	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_RequestStartDownloadList& inf)
{
	ss << inf.fhash;
	ss.write_string(inf.url);
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_RequestStartDownloadList& inf)
{
	ss >> inf.fhash;
	ss.read_string(inf.url,1024);
	return ss.ok();
}
int operator << (PTLStream& ss, const PTL_P2T_RequestStopDownloadList& inf)
{
	ss << inf.fhash;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_RequestStopDownloadList& inf)
{
	ss >> inf.fhash;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2T_ReportDownloadListMaxnum& inf)
{
	ss << inf.downloadlist_maxnum;
	return ss.ok();
}
int operator >> (PTLStream& ss, PTL_P2T_ReportDownloadListMaxnum& inf)
{
	ss >> inf.downloadlist_maxnum;
	return ss.ok();
}

