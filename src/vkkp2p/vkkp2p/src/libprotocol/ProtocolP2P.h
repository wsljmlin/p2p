#pragma once
#include "SerialStream.h"

//*************************
//peer to peer
enum PTLP2P
{
	PTL_P2P_                             = 17307
	,PTL_P2P_WELCOME                     = PTL_P2P_ + 1
	,PTL_P2P_RELEASE                     = PTL_P2P_ + 2
	,PTL_P2P_REQUEST_FILE_BLOCK_TABLE    = PTL_P2P_ + 3
	,PTL_P2P_RESPONSE_FILE_BLOCK_TABLE   = PTL_P2P_ + 4
	,PTL_P2P_REQUEST_FILE_BLOCKS         = PTL_P2P_ + 5
	,PTL_P2P_RESPONSE_FILE_BLOCKS        = PTL_P2P_ + 6
	,PTL_P2P_RESPONSE_FILE_BLOCKS_DATA   = PTL_P2P_ + 7
	,PTL_P2P_CANCEL_FILE_BLOCKS          = PTL_P2P_ + 8
	,PTL_P2P_REQUEST_FILE_SUB_KEYS       = PTL_P2P_ + 9
	,PTL_P2P_RESPONSE_FILE_SUB_KEYS      = PTL_P2P_ + 10
};

typedef struct tagPTL_P2P_Welcome
{
	uint32     ver;               //
	uint32     sessionID;         //反连接时确定对方，uid也可以
	char       utype;             //user type,用户类型
	char       ntype;             //统计时使用
	char       turn;              //识别是否为返连接，1为返连接
}PTL_P2P_Welcome;

//释放通道
//typedef struct tagPTL_P2P_Release
//{
//}PTL_P2P_Release;


typedef struct tagPTL_P2P_RequestFileBlockTable
{
	fhash_t    fhash;
	uint32     blockSize;          //允许请求不同块尺寸的共享
	uint32     startBufI;			//表示table表中的第几个字节起中的数据,即每8块1字节,startBufI*8表示实际的绝对索引号
	uint32     maxnum;             //字节数，不是块数，一次不能要超过1024块
}PTL_P2P_RequestFileBlockTable;

typedef struct tagPTL_P2P_ResponseFileBlockTable
{
	uint32     result;               //-1:没有文件，0：下载中，有块表，1：已经完成
	fhash_t    fhash;
	uint64     fsize;
	uint32     blockSize;
	uint32     startBufI;            //表示table表中的第几个字节起中的数据
	uint32     num;
	char       tableBuf[1024];        //32000*100KB=3.200,000KB ; 即100K每块的话，可以表示3.2G大小文件了
}PTL_P2P_ResponseFileBlockTable;

typedef struct tagPTL_P2P_RequestFileBlocks
{
	fhash_t    fhash;
	uint32     blockSize;
	uint32     num;                  //后面2个数组的有效大小
	uint32     indexs[32];           //块号
	uint32     offsets[32];          //块内偏移
}PTL_P2P_RequestFileBlocks;

typedef struct tagPTL_P2P_ResponseFileBlocks
{
	fhash_t    fhash;
	uint32     blockSize;
	uint32     num;                  //后面2个数组的有效大小
	uint32     indexs[32];           //块号
	uint32     blockState[32];       //0:缺少，1：存在
}PTL_P2P_ResponseFileBlocks;

typedef struct tagPTL_P2P_ResponseFileBlocksData
{
	fhash_t    fhash;
	uint32     blockSize;
	uint32     blockIndex;
	uint32     offset;
	uint32     size;
	char*      data;
}PTL_P2P_ResponseFileBlocksData;

typedef struct tagPTL_P2P_CancelFileBlocks
{
	fhash_t    fhash;
	uint32     blockSize;
	uint32     num;                  //后面2个数组的有效大小
	uint32     indexs[32];           //块号
}PTL_P2P_CancelFileBlocks;

typedef struct tagPTL_P2P_RequestFileSubKeys
{
	fhash_t    fhash;
	uint32     blockSize;         //这里的分块大小与下载block的大小不一定一样，但最好是下载block的整倍数
	uint32     num;
	uint32     indexs[1024];
}PTL_P2P_RequestFileSubKeys;

typedef struct tagPTL_P2P_ResponseFileSubKeys
{
	uint32     result;
	fhash_t    fhash;
	uint32     blockSize;         //这里的分块大小与下载block的大小不一定一样，但最好是下载block的整倍数
	uint32     num;
	uint32     indexs[1024];
	uint32     keys[1024];        //子块值为一个4位整数
}PTL_P2P_ResponseFileSubKeys;


//**********************************
int operator << (PTLStream& ss, const PTL_P2P_Welcome& inf);
int operator >> (PTLStream& ss, PTL_P2P_Welcome& inf);
int operator << (PTLStream& ss, const PTL_P2P_RequestFileBlockTable& inf);
int operator >> (PTLStream& ss, PTL_P2P_RequestFileBlockTable& inf);
int operator << (PTLStream& ss, const PTL_P2P_ResponseFileBlockTable& inf);
int operator >> (PTLStream& ss, PTL_P2P_ResponseFileBlockTable& inf);
int operator << (PTLStream& ss, const PTL_P2P_RequestFileBlocks& inf);
int operator >> (PTLStream& ss, PTL_P2P_RequestFileBlocks& inf);
int operator << (PTLStream& ss, const PTL_P2P_ResponseFileBlocks& inf);
int operator >> (PTLStream& ss, PTL_P2P_ResponseFileBlocks& inf);
int operator << (PTLStream& ss, const PTL_P2P_ResponseFileBlocksData& inf);
int operator >> (PTLStream& ss, PTL_P2P_ResponseFileBlocksData& inf);
int operator << (PTLStream& ss, const PTL_P2P_CancelFileBlocks& inf);
int operator >> (PTLStream& ss, PTL_P2P_CancelFileBlocks& inf);
int operator << (PTLStream& ss, const PTL_P2P_RequestFileSubKeys& inf);
int operator >> (PTLStream& ss, PTL_P2P_RequestFileSubKeys& inf);
int operator << (PTLStream& ss, const PTL_P2P_ResponseFileSubKeys& inf);
int operator >> (PTLStream& ss, PTL_P2P_ResponseFileSubKeys& inf);
//int operator << (PTLStream& ss, const & inf);
//int operator >> (PTLStream& ss, & inf);

