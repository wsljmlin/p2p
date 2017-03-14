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
	uint32     sessionID;         //������ʱȷ���Է���uidҲ����
	char       utype;             //user type,�û�����
	char       ntype;             //ͳ��ʱʹ��
	char       turn;              //ʶ���Ƿ�Ϊ�����ӣ�1Ϊ������
}PTL_P2P_Welcome;

//�ͷ�ͨ��
//typedef struct tagPTL_P2P_Release
//{
//}PTL_P2P_Release;


typedef struct tagPTL_P2P_RequestFileBlockTable
{
	fhash_t    fhash;
	uint32     blockSize;          //��������ͬ��ߴ�Ĺ���
	uint32     startBufI;			//��ʾtable���еĵڼ����ֽ����е�����,��ÿ8��1�ֽ�,startBufI*8��ʾʵ�ʵľ���������
	uint32     maxnum;             //�ֽ��������ǿ�����һ�β���Ҫ����1024��
}PTL_P2P_RequestFileBlockTable;

typedef struct tagPTL_P2P_ResponseFileBlockTable
{
	uint32     result;               //-1:û���ļ���0�������У��п��1���Ѿ����
	fhash_t    fhash;
	uint64     fsize;
	uint32     blockSize;
	uint32     startBufI;            //��ʾtable���еĵڼ����ֽ����е�����
	uint32     num;
	char       tableBuf[1024];        //32000*100KB=3.200,000KB ; ��100Kÿ��Ļ������Ա�ʾ3.2G��С�ļ���
}PTL_P2P_ResponseFileBlockTable;

typedef struct tagPTL_P2P_RequestFileBlocks
{
	fhash_t    fhash;
	uint32     blockSize;
	uint32     num;                  //����2���������Ч��С
	uint32     indexs[32];           //���
	uint32     offsets[32];          //����ƫ��
}PTL_P2P_RequestFileBlocks;

typedef struct tagPTL_P2P_ResponseFileBlocks
{
	fhash_t    fhash;
	uint32     blockSize;
	uint32     num;                  //����2���������Ч��С
	uint32     indexs[32];           //���
	uint32     blockState[32];       //0:ȱ�٣�1������
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
	uint32     num;                  //����2���������Ч��С
	uint32     indexs[32];           //���
}PTL_P2P_CancelFileBlocks;

typedef struct tagPTL_P2P_RequestFileSubKeys
{
	fhash_t    fhash;
	uint32     blockSize;         //����ķֿ��С������block�Ĵ�С��һ��һ���������������block��������
	uint32     num;
	uint32     indexs[1024];
}PTL_P2P_RequestFileSubKeys;

typedef struct tagPTL_P2P_ResponseFileSubKeys
{
	uint32     result;
	fhash_t    fhash;
	uint32     blockSize;         //����ķֿ��С������block�Ĵ�С��һ��һ���������������block��������
	uint32     num;
	uint32     indexs[1024];
	uint32     keys[1024];        //�ӿ�ֵΪһ��4λ����
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

