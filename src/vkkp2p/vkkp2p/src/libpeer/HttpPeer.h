#pragma once
#include "Peer.h"

class HttpPeer : public Peer
{
public:
	HttpPeer(void);
	virtual ~HttpPeer(void);

public:
	int send_request(const string& server,const string& cgi,sint64 ibegin=0,sint64 iend=-1,int httpver=1);
	Channel* get_channel(){return m_ch;}

	int get_server_response_code();
	int get_field(const char *szSession, string& text);
	sint64 get_req_begin() const { return m_req_begin;}
	sint64 get_req_end() const { return m_req_end;}
	int get_req_times() const { return m_req_times;}
private:
	void format_head(string& header,const string& server,const string& cgi,sint64 ibegin=0,sint64 iend=-1,int httpver=1);
public:
	virtual void on(Readable,Channel* ch,const int& wait);
	virtual void on_data();
	
protected:
	GETSET(bool,m_is_pause,_is_pause)
private:
	string m_header_string;
	sint64 m_req_begin;
	sint64 m_req_end;
	int m_req_times;
	sint64 m_recv_size;
};
