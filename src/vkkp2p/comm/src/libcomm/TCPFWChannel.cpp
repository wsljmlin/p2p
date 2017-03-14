#include "TCPFWChannel.h"

#define FW_HTTP_SIZE 256
static char* format_date(char *str)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	static const char* sMonth[] = { "Jan","Feb","Mar", "Apr","May", "Jun","Jul","Aug", "Sep", "Oct","Nov", "Dec"};
	static const char* sDay[] = { "Sun","Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	sprintf(str,"%s, %02d %s %04d %02d:%02d:%02d GMT", sDay[st.wDayOfWeek], 
			   st.wDay, sMonth[st.wMonth], st.wYear, st.wHour, st.wMinute, st.wSecond);
  return str;
}


void TCPFWChannel::on_connected_ex()
{
	m_recv_size = 0;
	if(!get_is_accept())
		send_http(0);
}
int TCPFWChannel::handle_input()
{
	if(m_recv_size<FW_HTTP_SIZE)
	{

		int ret = recv_http();
		if(1!=ret)
			return ret;
	}
	return TCPChannel::handle_input();
}

int TCPFWChannel::send_http(int type)
{
	MemBlock *b = MemBlock::allot(FW_HTTP_SIZE);
	if(!b)
	{
		assert(false);
		disconnect();
		return -1;
	}
	if(0==type)
		format_req(b->buf,FW_HTTP_SIZE);
	else
		format_rsp(b->buf,FW_HTTP_SIZE);
	b->datalen = FW_HTTP_SIZE;
	return send(b);
}

int TCPFWChannel::recv_http()
{
	char buf[FW_HTTP_SIZE];
	int ret = 0;
	while(m_recv_size<FW_HTTP_SIZE)
	{
		ret = this->recv(buf,FW_HTTP_SIZE-m_recv_size);
		if(ret>0)
			m_recv_size += ret;
		else if(0==ret)
			break;
		else
			return -1;
	}
	if(m_recv_size==FW_HTTP_SIZE)
	{
		if(get_is_accept())
			send_http(1);
		if(CONNECTED==m_state)
		{
			fire(ChannelListener::Connected(),this);
			return 1;
		}
	}
	return 0;
}

//http req
//POST /news/%path%/%name%.htm HTTP/1.1\r\n
//Host: %ip%\r\n
//Accept: */*\r\n
//Pragma: no-cache\r\n
//Content-Length: %length%\r\n
//Connection: keep-alive\r\n
//\r\n
char* TCPFWChannel::format_req(char* buf,int size)
{
	in_addr in;
	in.s_addr = htonl(m_hip);
	memset(buf,0,size);
	srand((unsigned int)time(NULL));
	sprintf(buf,"POST /news/%d/%d.htm HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Accept: */*\r\n"
		"Pragma: no-cache\r\n"
		"Content-Length: %d\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		,rand()%100000+100
		,rand()%10000+10
		,inet_ntoa(in)
		,rand()%100000+1568);
	return buf;
}

//http rsp
//HTTP/1.1 200 OK\r\n
//Date: %date%\r\n      Из:Mon,31Dec200104:25:57GMT
//Server: Apache/1.3.14(Unix)\r\n
//Accept-Ranges: bytes\r\n
//Content-Length: %length%\r\n
//Content-Type: text/plain\r\n
//Keep-Alive: timeout=15, max=100\r\n
//Connection: keep-alive\r\n
//\r\n

char* TCPFWChannel::format_rsp(char* buf,int size)
{
	char date[128];
	memset(buf,0,size);
	srand((unsigned int)time(NULL));
	sprintf(buf,"HTTP/1.1 200 OK\r\n"
		"Date: %s\r\n"
		"Server: Apache/1.3.14(Unix)\r\n"
		"Accept-Ranges: bytes\r\n"
		"Content-Length: %d\r\n"
		"Keep-Alive: timeout=15, max=100\r\n"
		"Connection: keep-alive\r\n"
		"\r\n"
		,format_date(date)
		,rand()%100000+45687);
	return buf;
}

