#include "MessagePusher.h"
#include "Setting.h"

#define ZERO_PORT_RETURN(i) if(0==m_port) return i;

MessagePusher::MessagePusher(void)
: m_fd(INVALID_SOCKET)
, m_port(0)
{
}

MessagePusher::~MessagePusher(void)
{
	assert(INVALID_SOCKET==m_fd);
	close_sock();
}

int MessagePusher::init()
{
	assert(INVALID_SOCKET==m_fd);
	m_port = SettingSngl::instance()->get_message_port();
	string ip = SettingSngl::instance()->get_message_ip();
	if(ip.empty())
		ip = "127.0.0.1";
	memset(&m_addr,0,sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	m_addr.sin_port = htons(m_port);

	open_sock();
	return 0;
}
int MessagePusher::fini()
{
	close_sock();
	return 0;
}

int MessagePusher::on_downloadlist_start(const string& url)
{
	ZERO_PORT_RETURN(-1)
	PTL_MSG_DownloadListStart inf;
	strcpy(inf.url,url.c_str());
	SendPTLPacket(PTL_MSG_DOWNLOADLIST_START,inf,1024);
	return 0;
}
int MessagePusher::on_downloadlist_stop(const string& url)
{
	ZERO_PORT_RETURN(-1)
	PTL_MSG_DownloadListStop inf;
	strcpy(inf.url,url.c_str());
	SendPTLPacket(PTL_MSG_DOWNLOADLIST_STOP,inf,1024);
	return 0;
}
int MessagePusher::on_download_file_start(const string& url)
{
	ZERO_PORT_RETURN(-1)
	PTL_MSG_DownloadStart inf;
	strcpy(inf.url,url.c_str());
	SendPTLPacket(PTL_MSG_DOWNLOAD_START,inf,1024);
	return 0;
}
int MessagePusher::on_download_file_stop(const string& url)
{
	ZERO_PORT_RETURN(-1)
	PTL_MSG_DownloadStop inf;
	strcpy(inf.url,url.c_str());
	SendPTLPacket(PTL_MSG_DOWNLOAD_STOP,inf,1024);
	return 0;
}

int MessagePusher::open_sock()
{
	close_sock();
	ZERO_PORT_RETURN(-1)

	m_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(INVALID_SOCKET == m_fd)
	{
		return -1;
	}
	sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if(SOCKET_ERROR==::bind(m_fd, (sockaddr *)&addr, sizeof(addr)))
	{
		closesocket(m_fd);
		return -1;
	}
	return 0;
}
int MessagePusher::close_sock()
{
	if(INVALID_SOCKET!=m_fd)
	{
		closesocket(m_fd);
	}
	return 0;
}

template<typename T>
int MessagePusher::SendPTLPacket(uint16 cmd,T& inf,int iMaxSize)
{
	PTLStream ss(iMaxSize);
	tmp_head.cmd = cmd;
	ss << tmp_head;
	ss << inf;
	ss.fitsize32(4);
	::sendto(m_fd,ss.buffer(),ss.length(),0,(sockaddr*)&m_addr,sizeof(m_addr));
	return 0;
}



