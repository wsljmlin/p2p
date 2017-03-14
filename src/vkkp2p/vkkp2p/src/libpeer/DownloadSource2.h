#pragma once
#include "DownloadSource.h"
#include "HttpPeer.h"


class DownloadSource2 : public DownloadSource
	,public PeerListener
{
public:
	DownloadSource2(void);
	virtual ~DownloadSource2(void);

	typedef struct tagHttpSourceInfo
	{
		string				url;
		string				server;
		unsigned int		ip;
		unsigned short		port;
		string				cgi;
		unsigned int		sessionID;	//分配一个来跟peer关联
		int					support_keepalive;			//0:not support,1:support,2:unknown,the initial value
		int					support_range;              //0:not support,1:support,2:unknown,the initial value
		int					connectTimes;  //连接次数
		int					connectFailTimes;
		int					userDataTimes;
		int					ref;
		unsigned int		lastUseTick;
		tagHttpSourceInfo(void):ip(0),port(0),support_keepalive(2),support_range(2),connectTimes(0),connectFailTimes(0),userDataTimes(0)
		,ref(0),lastUseTick(0){}
	}HttpSourceInfo;

	typedef struct tagHttpPeerData
	{
		PeerState					state;
		int							httpstate;//0:ready,1=requesting,2=dataing
		bool						isReady;
		string						server;
		string						cgi;
		list<BlockInfo>             blockList;
		Speedometer<unsigned int>   speedometer;
		int							support_keepalive;			//0:not support,1:support,2:unknown,the initial value
		int							support_range;              //0:not support,1:support,2:unknown,the initial value
		sint64						http_data_pos;
		sint64						http_data_remain;
		int                         lagTimes;  //竞争下载时的落后次数
		int							resume_state; //0:正常，收到数据后即置0，1：刚resume，2：1状态下分配到下载任务（断开时如果为2状态，即重新创建连接）
		int							userDataTimes;
		tagHttpPeerData(void):state(PS_DISCONNECT),httpstate(0),isReady(false),support_keepalive(2),support_range(2),http_data_pos(-1)
			,http_data_remain(-1),lagTimes(0),resume_state(0),userDataTimes(0){}
		void http_recv_count(int len){
			assert(len>0);
			http_data_pos += len;
			if(http_data_remain!=-1)
			{
				http_data_remain-=len;
				//assert(http_data_remain>=0);
				if(http_data_remain<0)
				{
					printf("****************************%lld************************\n",http_data_remain);
				}
			}
		}
	}HttpPeerData;
	
	typedef map<unsigned int,HttpSourceInfo*> HttpSourceMap;
	typedef HttpSourceMap::iterator HttpSourceIter;

	typedef map<HttpPeer*,HttpPeerData*> HttpPeerMap;
	typedef HttpPeerMap::iterator HttpPeerIter;

public:
	HttpSourceInfo* http_add_source(const string& url,int sessionID=0,bool trycreate=false,int userDataTimes=0);
	int get_http_max_num() const { return m_http_max_num;}
	int http_create_all_channel();
protected:
	int http_del_source(unsigned int id);
	HttpSourceInfo* http_get_source(unsigned int id);
	int http_create_channel(int count=1);
	int http_create_channel_i(HttpSourceInfo* inf);

	void http_clear_source_all();
	void http_put_peer_all();
	int http_deal_deadpeer(); //返回断开的个数
	int http_attach_peer(HttpPeer* peer,HttpSourceInfo* inf);
	int http_detach_peer(HttpPeer* peer,bool on_disconnected=false);
	void http_deal_pending();

	int http_assign_job(HttpPeer* peer);
	void http_resume_assign_job();
	int http_cancel_job_p(HttpPeer* peer,int index=-1,bool allowTryAssign=false);

	
	int http_req_data(HttpPeer *peer);
	bool http_check_reconnect(HttpPeer *peer,HttpPeerData* data,int index);
	bool http_check_rerequest(HttpPeer *peer,HttpPeerData *data);
	bool http_check_need_add_connection();
	bool http_check_need_del_connection(HttpPeer *peer);

	virtual int get_block_speed(int index,int seconds);
	
public:
	virtual void on(Connected,Peer* peer);
	virtual void on(Disconnected,Peer* peer);
	virtual void on(HttpHeader,Peer* peer,char* buf,int len);
	virtual void on(Data,Peer* peer,char* buf,int len);

	int check_disconnect_reattach(HttpPeer* peer,HttpPeerData* data);
	int on_attach_http_peer(HttpPeer* peer,const string& url);
	int update_http_max_num(int n);
	int get_httpconnect_amount()const {return m_ihttpConnectAmount;}
protected:
	HttpSourceMap				m_http_sources;
	HttpPeerMap					m_http_peers;
	list<HttpPeer*>				m_http_pending_peerls;
	int							m_http_max_num;
	unsigned int				m_http_last_check_add_del_tick;
	bool						m_isHttpAssignJobPause;
	char						m_use_http_type; //0不使用，1使用
	unsigned int              m_ihttpConnectAmount;
};

#define HTTP_GET_DATA(peer) HttpPeerData* data = m_http_peers.find(peer)->second

