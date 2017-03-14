#pragma once
#include "Protocol.h"
#include <winsock2.h>

class Tracker
{
public:
	Tracker(void);
	~Tracker(void);

	////注意：rsp的servers不装具体信息，
	int get_server(const char *ip,short port,PTL_P2T_ResponseServerList& rsp,PTL_P2T_ServerInfo *svr,int svr_size);
	//int update_server(const char *ip,short port,P2TRspUpdateServer *svr,int size);

private:
	int send_n(SOCKET sock,char *buf,int size);
	int recv_pack(SOCKET m_sock,char *m_buf,int m_maxlen,int& m_buflen);
	int send_http_req(SOCKET sock);
	int recv_http_rsp(SOCKET sock);

};
