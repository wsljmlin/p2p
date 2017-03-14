#include "ProtocolP2P.h"


int operator << (PTLStream& ss, const PTL_P2P_Welcome& inf)
{
	ss << inf.ver;
	ss << inf.sessionID;
	ss << inf.utype;
	ss << inf.ntype;
	ss << inf.turn;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_Welcome& inf)
{
	ss >> inf.ver;
	ss >> inf.sessionID;
	ss >> inf.utype;
	ss >> inf.ntype;
	ss >> inf.turn;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_RequestFileBlockTable& inf)
{
	ss << inf.fhash;
	ss << inf.blockSize;
	ss << inf.startBufI;
	ss << inf.maxnum;
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_RequestFileBlockTable& inf)
{
	ss >> inf.fhash;
	ss >> inf.blockSize;
	ss >> inf.startBufI;
	ss >> inf.maxnum;
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_ResponseFileBlockTable& inf)
{
	ss << inf.result;
	ss << inf.fhash;
	ss << inf.fsize;
	ss << inf.blockSize;
	ss << inf.startBufI;
	ss << inf.num;
	ss.write_array(inf.tableBuf,inf.num);
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_ResponseFileBlockTable& inf)
{
	ss >> inf.result;
	ss >> inf.fhash;
	ss >> inf.fsize;
	ss >> inf.blockSize;
	ss >> inf.startBufI;
	ss >> inf.num;
	if(inf.num>1024) inf.num = 1024;
	ss.read_array(inf.tableBuf,inf.num);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_RequestFileBlocks& inf)
{
	ss << inf.fhash;
	ss << inf.blockSize;
	ss << inf.num;
	ss.write_array(inf.indexs,inf.num);
	ss.write_array(inf.offsets,inf.num);
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_RequestFileBlocks& inf)
{
	ss >> inf.fhash;
	ss >> inf.blockSize;
	ss >> inf.num;
	if(inf.num>32) inf.num = 32;
	ss.read_array(inf.indexs,inf.num);
	ss.read_array(inf.offsets,inf.num);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_ResponseFileBlocks& inf)
{
	ss << inf.fhash;
	ss << inf.blockSize;
	ss << inf.num;
	ss.write_array(inf.indexs,inf.num);
	ss.write_array(inf.blockState,inf.num);
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_ResponseFileBlocks& inf)
{
	ss >> inf.fhash;
	ss >> inf.blockSize;
	ss >> inf.num;
	if(inf.num>32) inf.num = 32;
	ss.read_array(inf.indexs,inf.num);
	ss.read_array(inf.blockState,inf.num);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_ResponseFileBlocksData& inf)
{
	ss << inf.fhash;
	ss << inf.blockSize;
	ss << inf.blockIndex;
	ss << inf.offset;
	ss << inf.size;
	ss.write((void*)inf.data,inf.size);
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_ResponseFileBlocksData& inf)
{
	ss >> inf.fhash;
	ss >> inf.blockSize;
	ss >> inf.blockIndex;
	ss >> inf.offset;
	ss >> inf.size;
	assert((uint32)ss.length()>=inf.size);
	inf.data = ss.read_ptr();
	ss.skipr(inf.size);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_CancelFileBlocks& inf)
{
	ss << inf.fhash;
	ss << inf.blockSize;
	ss << inf.num;
	ss.write_array(inf.indexs,inf.num);
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_CancelFileBlocks& inf)
{
	ss >> inf.fhash;
	ss >> inf.blockSize;
	ss >> inf.num;
	if(inf.num>32) inf.num = 32;
	ss.read_array(inf.indexs,inf.num);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_RequestFileSubKeys& inf)
{
	ss << inf.fhash;
	ss << inf.blockSize;
	ss << inf.num;
	ss.write_array(inf.indexs,inf.num);
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_RequestFileSubKeys& inf)
{
	ss >> inf.fhash;
	ss >> inf.blockSize;
	ss >> inf.num;
	if(inf.num>1024) inf.num = 1024;
	ss.read_array(inf.indexs,inf.num);
	return ss.ok();
}

int operator << (PTLStream& ss, const PTL_P2P_ResponseFileSubKeys& inf)
{
	ss << inf.result;
	ss << inf.fhash;
	ss << inf.blockSize;
	ss << inf.num;
	ss.write_array(inf.indexs,inf.num);
	ss.write_array(inf.keys,inf.num);
	return ss.ok();
}

int operator >> (PTLStream& ss, PTL_P2P_ResponseFileSubKeys& inf)
{
	ss >> inf.result;
	ss >> inf.fhash;
	ss >> inf.blockSize;
	ss >> inf.num;
	if(inf.num>1024) inf.num = 1024;
	ss.read_array(inf.indexs,inf.num);
	ss.read_array(inf.keys,inf.num);
	return ss.ok();
}

