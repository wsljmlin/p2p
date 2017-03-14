#pragma once
#include "SerialStream.h"

enum PTLMSG
{
	PTL_MSG_ = 100
	,PTL_MSG_DOWNLOADLIST_START			= PTL_MSG_ + 1
	,PTL_MSG_DOWNLOADLIST_STOP			= PTL_MSG_ + 2
	,PTL_MSG_DOWNLOAD_START				= PTL_MSG_ + 5
	,PTL_MSG_DOWNLOAD_STOP				= PTL_MSG_ + 6
};

//*******************************************************************
typedef  struct  tagPTL_MSG_DownloadListStart
{
	char         url[1024];
}PTL_MSG_DownloadListStart;

typedef  struct  tagPTL_MSG_DownloadListStop
{
	char         url[1024];
}PTL_MSG_DownloadListStop;

typedef  struct  tagPTL_MSG_DownloadStart
{
	char         url[1024];
}PTL_MSG_DownloadStart;

typedef  struct  tagPTL_MSG_DownloadStop
{
	char         url[1024];
}PTL_MSG_DownloadStop;



//********************************************************************
int operator << (SerialStream& ss, const PTL_MSG_DownloadListStart& inf);
int operator >> (SerialStream& ss, PTL_MSG_DownloadListStart& inf);
int operator << (SerialStream& ss, const PTL_MSG_DownloadListStop& inf);
int operator >> (SerialStream& ss, PTL_MSG_DownloadListStop& inf);

int operator << (SerialStream& ss, const PTL_MSG_DownloadStart& inf);
int operator >> (SerialStream& ss, PTL_MSG_DownloadStart& inf);
int operator << (SerialStream& ss, const PTL_MSG_DownloadStop& inf);
int operator >> (SerialStream& ss, PTL_MSG_DownloadStop& inf);



