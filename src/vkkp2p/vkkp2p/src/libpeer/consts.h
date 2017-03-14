#pragma once

#ifndef OSTYPE
#define OSTYPE "none"
#endif

#define VERSION_NUM 3000
//#define VERSION_STR "vp-20141120"
#define VERSION_STR "vp-20160930"



#define DEFAULT_BLOCK_SIZE 102400

enum {FTYPE_VOD=0,FTYPE_DOWNLOAD,FTYPE_SHAREONLY};


#define IPT_TCP  0x01
#define IPT_UDP  0x02
#define IPT_HTTP  0x03

enum ConnectType{ TCP_CONN=0,TCP_TURN,UDP_CONN,LOCAL_TCP,LOCAL_UDP,UNUSABLE,UNKNOW };
