#include "DownloadSource2.h"
#include "FileStorage.h"
#include "Setting.h"
#include "Statistician.h"
#include "Util.h"
#include "DownloadListManager.h"

#ifdef SM_DBG
#define DOWNLOADSOURCE2_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "DownloadSource2" ,__LINE__, ##arg)
#else
#define DOWNLOADSOURCE2_PRT(fmt, arg...) 
#endif


DownloadSource2::DownloadSource2(void)
:m_http_max_num(0)
,m_http_last_check_add_del_tick(0)
,m_isHttpAssignJobPause(false)
,m_use_http_type(1)
,m_ihttpConnectAmount(0)
{
}
 
DownloadSource2::~DownloadSource2(void)
{
}

DownloadSource2::HttpSourceInfo* DownloadSource2::http_add_source(const string& url,int sessionID/*=0*/,bool trycreate/*=false*/,int userDataTimes/*=0*/)
{
	static unsigned int __sessionID = 1;
	HttpSourceInfo *inf = NULL;
	for(HttpSourceIter it=m_http_sources.begin();it!=m_http_sources.end();++it)
	{
		if(it->second->url == url)
		{
			inf = it->second;
			break;
		}
	}
	if(inf)
		return inf;
	inf = new HttpSourceInfo();
	if(!inf)
		return NULL;
	if(0==sessionID)
		sessionID = __sessionID++;
	inf->sessionID = sessionID;
	inf->url = url;
	inf->userDataTimes = userDataTimes;
	Util::url_element_split(url,inf->server,inf->port,inf->cgi);
	m_http_sources[inf->sessionID] = inf;
	if(trycreate && (int)m_http_peers.size()<m_http_max_num)
	{
		http_create_channel_i(inf);
	}
	return inf;
}

int DownloadSource2::http_del_source(unsigned int id)
{
	HttpSourceIter it=m_http_sources.find(id);
	assert(it!=m_http_sources.end());
	if(it==m_http_sources.end())
		return 0;
	delete it->second;
	m_http_sources.erase(it);
	return 0;
}
DownloadSource2::HttpSourceInfo* DownloadSource2::http_get_source(unsigned int id)
{
	HttpSourceIter it=m_http_sources.find(id);
	if(it==m_http_sources.end())
		return NULL;
	return it->second;
}
int DownloadSource2::http_create_all_channel()
{
	return http_create_channel(m_http_max_num-(int)m_http_peers.size());
}
int DownloadSource2::http_create_channel(int count/*=1*/)
{
	if((int)m_http_peers.size()>=m_http_max_num)
		return 0;
	// 再次防止建立连接数多于配置指定连接数
	int max = m_http_max_num - m_http_peers.size();
	if(count>max) count=max;
	if(count<=0) return 0;

	int n = 0;
	HttpSourceInfo* inf = NULL;
	while(n<count)
	{
		inf = NULL;
		//找引用最少的一个资源创建连接
		for(HttpSourceIter it=m_http_sources.begin();it!=m_http_sources.end();++it)
		{
			if(!inf) 
				inf = it->second;
			else
			{
				if(inf->connectFailTimes > it->second->connectFailTimes)
				{
					inf = it->second;
				}
				else if(inf->connectFailTimes == it->second->connectFailTimes)
				{
					if(inf->ref > it->second->ref)
						inf = it->second;
				}
			}
		}
		if(inf && 0==http_create_channel_i(inf))
		{
			++n;
		}
		else
		{
			//因为下次仍然可能找到同样的
			break;
		}
	}
	return n;
}
int DownloadSource2::http_create_channel_i(HttpSourceInfo* inf)
{
	HttpPeer* peer = new HttpPeer();
	if(!peer)
		return -1;
	inf->connectTimes++;
	inf->lastUseTick = m_iCurrTick;
	peer->set_sessionID(inf->sessionID);
	if(0!=http_attach_peer(peer,inf))
	{
		delete peer;
		return -1;
	}

	unsigned int hip=0;
	if(inf->ip)
		hip = inf->ip;
	else
	{
		string ip = Util::ip_explain_ex(inf->server.c_str(),15000);
		hip = Util::ip_atoh(ip.c_str());
		DOWNLOADSOURCE2_PRT("will connect to %s:%d\n", ip.c_str(), inf->port);
	}
	if(0!=peer->connect(hip,inf->port,0))
	{
		peer->disconnect(); //会调用detach
		return -1;
	}
	DOWNLOADSOURCE2_PRT("connect to %d:%d\n", hip, inf->port);
	return 0;
}
void DownloadSource2::http_clear_source_all()
{
	for(HttpSourceIter it=m_http_sources.begin();it!=m_http_sources.end();++it)
		delete it->second;
	m_http_sources.clear();
}
void DownloadSource2::http_put_peer_all()
{
	HttpPeerIter it;
	while(!m_http_peers.empty())
	{
		it = m_http_peers.begin();
		check_disconnect_reattach(it->first,it->second);
		//m_http_peers.begin()->first->disconnect();
	}
	http_deal_pending();
}
int DownloadSource2::http_deal_deadpeer()
{
	int n = 0;
	int speed = 0;
	for(HttpPeerIter it=m_http_peers.begin();it!=m_http_peers.end();++it)
	{
		if(it->first->m_last_active_tick+5<m_iCurrTick)
		{
			DEBUGMSG("#*** clear http dead peer !\n");
			it->first->disconnect();
			n++;
			break; //破坏队列
		}
		if(SettingSngl::instance()->get_min_httpi_speedB() && it->second->speedometer.get_sec_amount()>2)
		{
			speed = it->second->speedometer.get_speed(2);
			if(speed < SettingSngl::instance()->get_min_httpi_speedB())
			{
				DEBUGMSG("#*** clear http slowly peer(speed=%d KB) !\n",speed>>10);
				it->first->disconnect();
				n++;
				break; //破坏队列
			}
		}
			
	}
	return n;
}

int DownloadSource2::http_attach_peer(HttpPeer* peer,HttpSourceInfo* inf)
{
	char buf[128];
	HttpPeerData *data = new HttpPeerData();
	if(!data)
		return -1;
	if(1==SettingSngl::instance()->get_http_no_keep_alive())
		data->support_keepalive = 0;
	m_http_peers[peer] = data;
	peer->add_listener(this);
	peer->set_PeerReadLimitSinker(this);
	peer->set_user_type(UT_HTTP);
	if(80==inf->port)
		data->server = inf->server;
	else
	{
		sprintf(buf,"%s:%d",inf->server.c_str(),inf->port);
		data->server = buf;
	}
	data->cgi = inf->cgi;
	data->support_keepalive = inf->support_keepalive;
	data->support_range = inf->support_range;
	data->userDataTimes = inf->userDataTimes;
	m_ihttpConnectAmount++;
	inf->ref++;
	return 0;
}
int DownloadSource2::http_detach_peer(HttpPeer* peer,bool on_disconnected/*=false*/)
{
	HttpPeerIter it=m_http_peers.find(peer);
	//如果先detach_peer再disconnect()，这里会重复，传递连接时用到
	if(it==m_http_peers.end())
		return -1;
	HttpPeerData* data = it->second;
	if(data->isReady)
		StatisticianSngl::instance()->OnConnectionSpeed(peer->get_channel()->get_hip(),data->speedometer.get_speed(),
			data->speedometer.get_sec_amount(),peer->get_ip_type(),peer->get_user_type(),true);
	http_cancel_job_p(peer);
	HttpSourceIter sit = m_http_sources.find(peer->get_sessionID());
	if(sit!=m_http_sources.end())
	{
		HttpSourceInfo *inf = sit->second;
		inf->ref--;
		if(!data->isReady)
			inf->connectFailTimes++;
		else
			inf->connectFailTimes = 0;
		//如果resume之后分配到任务但服务器已经断开，此时要新开连接
		if(on_disconnected &&(2==data->resume_state || 0==inf->connectTimes))
			http_create_channel_i(inf);
	}
	m_ihttpConnectAmount--;
	peer->remove_listener(this);
	peer->set_PeerReadLimitSinker(NULL);
	m_http_peers.erase(it);
	delete data;
	return 0;
}
void DownloadSource2::http_deal_pending()
{
	if(m_http_pending_peerls.empty())
		return;
	list<HttpPeer*> ls;
	m_http_pending_peerls.swap(ls);
	for(list<HttpPeer*>::iterator it=ls.begin();it!=ls.end();++it)
		delete *it;
}


void DownloadSource2::on(Connected,Peer* peer)
{
	m_http_peers.find(static_cast<HttpPeer*>(peer))->second->isReady = true;
	http_assign_job(static_cast<HttpPeer*>(peer));
}
void DownloadSource2::on(Disconnected,Peer* peer)
{
	StatisticianSngl::instance()->OnConnection(peer->get_conn_type(),peer->get_conn_success());
	if(HT_URL2==m_fi->hash.hash_type())
		DownloadListManagerSngl::instance()->on_connection(m_fi->hash,peer->get_conn_type(),peer->get_conn_success());
	HttpPeer* hpeer = static_cast<HttpPeer*>(peer);
	http_detach_peer(hpeer,true);
	m_http_pending_peerls.push_back(hpeer);
}

//************************************************************************************
int DownloadSource2::http_assign_job(HttpPeer* peer)
{
	HttpPeerData* data = m_http_peers.find(peer)->second;
	data->state = PS_DOWNLOADING;
	if(1==data->httpstate)
	{
		DEBUGMSG("#assign_job:*****************requesting header*********************** \n");
		peer->disconnect();
		return -1;
	}
	if(m_fi->size == 0 || 0==data->userDataTimes)
	{
		assert(data->http_data_pos == -1);
		http_req_data(peer);
		return 1;
	}
	//todo:这里需要考虑第一次连接上还没被分配过任务就被断开的情况
	if(http_check_need_del_connection(peer))
	{
		peer->disconnect();
		return -1;
	}

	//如果本连接上次已经分配下载过任务，看看是否还可继续使用本连接
	if(http_check_reconnect(peer,data,-1))
	{
		uint32 cid = peer->get_sessionID();
		peer->disconnect();
		
		HttpSourceInfo *inf = http_get_source(cid);
		assert(inf);
		if(inf)
			http_create_channel_i(inf);
		return -1;
	}

	BlockInfo bi;
	bi.index = 0;
	assert(data->blockList.empty());
	data->blockList.clear();

	int i=0,min_i=-1;
	int blocks = m_fi->block_offset + m_fi->blocks;

	list<int> ls;
	int need_i = (int)(m_fi->req_offset/m_fi->block_size);
	int speed = data->speedometer.get_speed(5);
	unsigned int block_nums = 6;
	if(1==data->support_keepalive)
		block_nums = 2;
	int urgent_win = need_i+SettingSngl::instance()->get_urgent_win();
	int smooth_win = need_i+SettingSngl::instance()->get_smoot_win();
	unsigned int random_win = SettingSngl::instance()->get_random_win()+block_nums;
	//如果不支持keepalive的话，不进行随机下载，即随机窗口为1
	if(1!=data->support_keepalive)
		random_win = block_nums+1;
	int http_pause_win = need_i + SettingSngl::instance()->get_memcache_win();
	if(SettingSngl::instance()->get_http_pause_win()>0 && SettingSngl::instance()->get_http_pause_win()<SettingSngl::instance()->get_memcache_win())
	{
		http_pause_win = need_i + SettingSngl::instance()->get_http_pause_win();
	}
	i = need_i;
	//1.紧急窗，抢紧急块,随机窗为0
	if(speed>0 && 0==data->lagTimes)
	{
		int need_i_speed =0;
		for(;i<blocks&&i<urgent_win;++i)
		{
			if(-1==min_i && !m_fi->bt_memfinished[i])
				min_i = i;

			if(m_fi->bt_memfinished[i])
				continue;
			need_i_speed = get_block_speed(i,5);
			if(0==need_i_speed || (need_i_speed < (DEFAULT_STREAM_SPEED/3) && (need_i_speed+5000)<speed))
			{
				DEBUGMSG("#:rob important_block => %d \n",i);
				ls.push_back(i);
				random_win = block_nums+1;
			}
		}
	}

	//2.平稳窗,随机窗为0
	for(;i<blocks && i<smooth_win && i<http_pause_win && ls.size()<random_win;++i)
	{
		if(-1==min_i && !m_fi->bt_memfinished[i])
			min_i = i;
		if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i))
		{
			ls.push_back(i);
			random_win = block_nums+1;
		}
	}

	//3.http_pause 窗
	for(;i<blocks && i<http_pause_win && ls.size()<random_win;++i)
	{
		if(-1==min_i && !m_fi->bt_memfinished[i])
			min_i = i;
		if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i))
			ls.push_back(i);
	}

	//因为随机，取消长距离抢夺
	////3.分析一下最靠前未完成的块与当前分配到的块的距离，如果太远，就考虑抢块下载
	//if(!ls.empty() && min_i+30<ls.front())
	//{
	//	//距离10块以上
	//	int min_i_speed = get_block_speed(min_i,5);
	//	if(speed > 3*min_i_speed)
	//	{
	//		ls.front() = min_i;
	//		DEBUGMSG("#---Long Block Distance ,rob(%d) \n",min_i);
	//	}
	//}

	if(ls.empty() && (m_fi->is_allow_pause()||SettingSngl::instance()->get_http_pause_win()>0))
	{
		if(0==data->lagTimes)
		{
			//在此一定要抢速度慢的下载
			int tmp = 1000000,tmp2=0,index=-1;
			for(i=need_i;i<blocks && i<urgent_win && i<http_pause_win && ls.size()<block_nums;++i)
			{
				if(!m_fi->bt_memfinished[i])
				{
					tmp2 = get_block_speed(i,5);
					//smooth_win内的暂时不抢，成为紧急窗再抢
					if(tmp2<=tmp)
					{
						index = i;
						tmp = tmp2;
					}
				}
			}
			if(index!=-1)
			{
				DEBUGMSG("#:rob slow_block => %d \n",index);
				ls.push_back(index);
			}
		}
		if(ls.empty())
		{
			//if(check_memcache_done())
			//{
			//	//传递给下一个下载
			//	check_disconnect_reattach(peer,data);
			//	return -1;
			//}
			//todo_hcl: 暂停会导致有多余数据未被使用，考虑peer缓存已经收到的数据
			data->state = PS_ASSIGN_JOB_PAUSE;
			peer->set_is_pause(true);
			m_isHttpAssignJobPause = true;
			m_lastPauseNeedI = need_i;
			DEBUGMSG("---Http AssignJobPause!--\n");
			return 2;
			//peer->disconnect();
			//return -1;
		}
	}

	if(ls.empty())
	{
		//一直向后寻找
		for(;i<blocks && ls.size()<random_win;++i)
		{
			if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i))
				ls.push_back(i);
		}

		if(ls.empty() && !m_fi->is_memcache_only())
		{
			//分配其它空闲块
			for(i=m_fi->block_gap;i<blocks && ls.size()<random_win;++i)
			{
				if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i))
					ls.push_back(i);
			}
		}
	}

	//.抢速度慢的块
	if(ls.empty() && 0==data->lagTimes)
	{
		int tmp = 1000000,tmp2=0,index=-1;
		for(i=m_fi->block_gap; i<blocks && ls.size()<block_nums; ++i)
		{
			if(!m_fi->bt_memfinished[i])
			{
				tmp2 = get_block_speed(i,5);
				if(tmp2<=tmp)
				{
					index = i;
					tmp = tmp2;
				}
			}
		}
		if(index!=-1)
		{
			DEBUGMSG("#:rob slow_block => %d \n",index);
			ls.push_back(index);
		}
	}
	
	if(!ls.empty())
	{
		//计算随机：
		random_win = ls.size();
		if(random_win>block_nums) 
		{
			random_win-=block_nums;
			srand((unsigned int)time(NULL));
			//printf("# ------ rand win=%d / block_nums=%d\n",random_win,block_nums);
			random_win = rand()%random_win;
		}
		else
		{
			random_win = 0;
		}
		unsigned int n = 0;
		//要考虑ls重复问题
		for(list<int>::iterator it=ls.begin();it!=ls.end();++it)
		{
			if(n++<random_win)
				continue;
			//在此只加连续块任务
			if(data->blockList.empty() || (i+1)==*it)
			{
				i = *it;
				bi.index = i;
				bi.pos = m_fi->get_block_downing_size(i);
				bi.blockSize = m_fi->get_block_size(i);
				if(bi.pos>=bi.blockSize)
				{
					on_block_done(i,1);
					if(m_isFinished)
						return -1;
				}
				else
				{
					if(data->blockList.empty())
					{
						if(http_check_reconnect(peer,data,i))
						{
							int cid = peer->get_sessionID();
							peer->disconnect();
							HttpSourceInfo *inf = http_get_source(cid);
							assert(inf);
							if(inf)
								http_create_channel_i(inf);
							return -1;
						}
					}
					add_block_ref(i,peer);
					data->blockList.push_back(bi);
				}
			}
			if(data->blockList.size()>=block_nums)
				break;
		}
	}

	if(data->blockList.empty())
	{
		//传递给下一个下载
		check_disconnect_reattach(peer,data);
		return -1;
	}
	if(1==data->resume_state)
		data->resume_state = 2;
	if(http_check_rerequest(peer,data))
	{
		http_req_data(peer);
		return 1;
	}
	return 0;
}
void DownloadSource2::http_resume_assign_job()
{
	if(!m_isHttpAssignJobPause)
		return;
	int need_i = (int)(m_fi->req_offset/m_fi->block_size);
	int http_pause_win = need_i + SettingSngl::instance()->get_memcache_win();
	if(SettingSngl::instance()->get_http_pause_win()>0 && SettingSngl::instance()->get_http_pause_win()<SettingSngl::instance()->get_memcache_win())
	{
		http_pause_win = need_i + SettingSngl::instance()->get_http_pause_win();
	}
	//
	if(need_i < m_lastPauseNeedI || (need_i-m_lastPauseNeedI)>(http_pause_win/3) || !is_urgent_win_ok(need_i))
	{
		//当请求点前移或者可用窗口数据不足2/3时，启动下载,(注意:当指针移到文件尾倒数2/3窗口以内时，会不断执行此动作)
		m_isHttpAssignJobPause = false;
		HttpPeer *peer = NULL;
		HttpPeerData *data = NULL;
		for(HttpPeerIter it=m_http_peers.begin();it!=m_http_peers.end();++it)
		{
			peer = it->first;
			data = it->second;
			if(peer->get_is_pause())
			{
				//如果有删除节点，it指针会受到破坏
				assert(PS_ASSIGN_JOB_PAUSE==data->state);
				peer->set_is_pause(false);
				data->resume_state = 1;
				//pause状态下有可能服务器连接已经断开，此处会导致恢复后没有下载到数据
				if(-1==http_assign_job(peer))
					it=m_http_peers.begin();
			}
		}
		DEBUGMSG("...resume http_assign_job...\n");
	}
}
int DownloadSource2::http_cancel_job_p(HttpPeer* peer,int index/*=-1*/,bool allowTryAssign/*=false*/)
{
	HttpPeerData* data = m_http_peers.find(peer)->second;
	list<BlockInfo>::iterator it;
	for(it=data->blockList.begin();it!=data->blockList.end();)
	{
		if(-1==index || index==(int)(*it).index)
		{
			del_block_ref((*it).index,peer);
			it = data->blockList.erase(it);
			if(-1!=index)
				break;
		}
		else
			++it;
	}
	if(allowTryAssign && data->blockList.empty())
		return http_assign_job(peer);
	return 0;
}

int DownloadSource2::http_req_data(HttpPeer *peer)
{
	HttpPeerData *data = m_http_peers.find(peer)->second;
	sint64 begin =-1,end =-1;

	if(data->blockList.empty())
	{
		//初次使用此连接的时候现在也是不指定range
		//assert(m_fi->size == 0);
		begin = 0;
		end = -1;
		//end = 1; //等1时，有可能导致一些不支持range的情况
		//DEBUGMSG("# ====> http_req_data: cgi=%s \n",data->cgi.c_str());
	}
	else
	{
		BlockInfo& bi=data->blockList.front();
		BlockInfo& bi2=data->blockList.back();
		
		begin = bi.index*(sint64)m_fi->block_size + bi.pos;
		end = bi2.index*(sint64)m_fi->block_size + bi2.blockSize-1;
	}
	if(0==data->support_keepalive)
	{
		end = -1;
	}
	//DEBUGMSG("#...request: %x--%lld-%lld\n",(int)peer,begin,end);
	data->httpstate = 1;
	DEBUGMSG("#****************0==data->support_keepalive: %lld-%lld\n",begin,end);
	peer->send_request(data->server,data->cgi,begin,end);
	return 0;
}

bool DownloadSource2::http_check_reconnect(HttpPeer *peer,HttpPeerData* data,int index)
{
	if(data->http_data_pos==-1)
		return false; //还没连通

	//如果不是第一次请求,判断是否断开重来
	bool breconnect = false;
	//过多数据按速度节算
	int speed_B = data->speedometer.get_speed(5);
	if(0==speed_B) speed_B = 1024;
	sint64 pos = -1;
	if(-1==index)
	{
		//支持keeplive 或 剩可下载数据,不重连
		if(1!=data->support_keepalive && data->http_data_remain == 0)
			breconnect = true;
	}
	else
	{
		pos = index * (sint64)m_fi->block_size + m_fi->get_block_downing_size(index);
		if(pos < data->http_data_pos)
		{
			//只能重新请求的
			//如果不支持keeplive 或者 支持keeplive但乘除数据过多,重连
			//这里注意，不支持range的话，一定不要支持support_keepalive
			if(1!=data->support_keepalive)
				breconnect = true;
			else 
			{
				//支持keeplive
				if(data->http_data_remain==-1 || data->http_data_remain>0)
					breconnect = true;
			}
		}
		else
		{
			//只要不支持range，有数据就不重新连接
			if(0==data->support_range)
			{
				//只要有数据就不重连
				if(-1==data->http_data_remain||data->http_data_remain>0)
					return false;
				else if(1==data->support_keepalive)
					return false;
			}
			//可以不重新请求的
			if(1==data->support_keepalive)
			{
				if((pos-data->http_data_pos)>speed_B && data->http_data_remain>speed_B)
					breconnect = true;
				////剩余数据不多或者剩余全部,不连接
				//if(data->http_data_remain==-1 || (data->http_data_pos + data->http_data_remain)==(sint64)m_fi->size)
				//{
				//	//剩余全部的
				//	if((pos-data->http_data_pos)>speed_B && data->http_data_remain>speed_B)
				//		breconnect = true;
				//}
				//else
				//{
				//	//剩余数据不多可以重新请求不连接
				//	if((pos-data->http_data_pos)>speed_B && data->http_data_remain>speed_B)
				//		breconnect = true;
				//}
			}
			else
			{
				//剩余数据不全 或者 剩余数据全但空隔过大,重连
				if(data->http_data_remain!=-1&&(data->http_data_pos + data->http_data_remain)<(sint64)m_fi->size)
					breconnect = true;
				else if((pos-data->http_data_pos)>speed_B)
					breconnect = true;
			}
		}
	}
	//DEBUGMSG("#---http reconnect=%d keepalive=%d,httpremain=%lld,offset = %lld\n",breconnect?1:0,data->support_keepalive
	//		,data->http_data_remain,data->http_data_pos-pos);
	if(breconnect)
		DEBUGMSG("# http_check_reconnect = true !!!\n");
	return breconnect;
}
bool DownloadSource2::http_check_rerequest(HttpPeer *peer,HttpPeerData *data)
{
	////判断是否需要发送头请求
	assert(!data->blockList.empty());
	if(data->http_data_pos == -1)
		return true;

	//data->http_data_pos != -1 至少已经发送过一次请求
	BlockInfo& bi = data->blockList.front();
	sint64 pos = bi.index*(sint64)m_fi->block_size + bi.pos;
	sint64 speed_B=data->speedometer.get_speed(5);
	if(1!=data->support_keepalive)
	{
		//不keeplive不可能发第二次请求
		assert(pos>=data->http_data_pos);
		//assert((pos-data->http_data_pos)<=speed_B); //不支持range的时候，有可能出现很长垃圾数据夸越也不会重新连接
		assert(data->http_data_remain==-1 || (data->http_data_remain + data->http_data_pos)==(sint64)m_fi->size);
		if(pos<data->http_data_pos || (pos-data->http_data_pos)>speed_B){}
		return false;
	}
	else
	{
		//只有部分的只能发请求
		if(0==data->http_data_remain)
			return true;
		return false;
		////数据全部不可能发第二次请求
		//if(data->http_data_remain==-1 || (data->http_data_remain + data->http_data_pos)==(sint64)m_fi->size)
		//{
		//	assert(data->http_data_remain<=speed_B || (pos-data->http_data_pos)<=speed_B);
		//	return false;
		//}
		//else
		//{
		//	return false;
		//}
	}
}
bool DownloadSource2::http_check_need_add_connection()
{
	if(DOWNS_START!=m_state)
		return false;
	if(m_http_last_check_add_del_tick + 10 > m_iCurrTick)
		return false;
	m_http_last_check_add_del_tick = m_iCurrTick;
	if((int)m_http_peers.size()>=m_http_max_num)
		return false;
	sint64 speed = m_speedometer.get_speed(5);
	if(m_http_peers.size()<1 || speed < STREAM_SIZE_PER_SECOND)
		return true;
	return false;
}
bool DownloadSource2::http_check_need_del_connection(HttpPeer *peer)
{
	m_http_last_check_add_del_tick = m_iCurrTick;
	if(peer->get_req_times()<2 || m_peers.size()<1)
		return false;
	if((int)m_http_peers.size()> m_http_max_num)
		return true;
	return false;
}
int DownloadSource2::get_block_speed(int index,int seconds)
{
	int speed = 0,tmp=0;
	BlockRefIter it = m_blockRefs.find(index);
	if(it==m_blockRefs.end())
	{
		return 0;
	}
	list<Peer*>& ls = it->second;
	for(list<Peer*>::iterator it=ls.begin();it!=ls.end();++it)
	{
		tmp = 0;
		if(IPT_HTTP==(*it)->get_ip_type())
		{
			//tmp = m_http_peers.find(static_cast<HttpPeer*>(*it))->second->speedometer.get_speed(seconds);
			HttpPeerIter it2 = m_http_peers.find(static_cast<HttpPeer*>(*it));
			if(it2!=m_http_peers.end())
			{
				tmp = it2->second->speedometer.get_speed(seconds);
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			//tmp = get_peerdata(*it)->speedometer.get_speed(seconds);
			PeerData *data = get_peerdata(*it);
			if(data)
			{
				tmp = data->speedometer.get_speed(seconds);
			}
			else
			{
				assert(false);
			}
		}
		if(tmp>speed)
			speed = tmp;
	}
	return speed;
}
void DownloadSource2::on(HttpHeader,Peer* peer,char* buf,int len)
{
	peer->m_last_active_tick = m_iCurrTick;
	HttpPeer* httppeer = static_cast<HttpPeer*>(peer);
	HttpPeerData* data = m_http_peers.find(httppeer)->second;
	data->resume_state = 0;
	string header = buf;
	int result = httppeer->get_server_response_code();
	data->httpstate = 2;
	data->userDataTimes++;
	DEBUGMSG("# --> onhttpheader : result=%d \n",result);
	if (result != 200 && result != 206)//出错
	{
		string newurl;
		if (301==result || 302 == result)
		{
			httppeer->get_field("Location",newurl);
			if (newurl.find("http://") != string::npos)
			{
				DEBUGMSG("#change url to: %s \n",newurl.c_str());
				this->http_del_source(peer->get_sessionID());
			}
		}
		httppeer->disconnect();
		if(!newurl.empty())
			this->http_add_source(newurl,0,true);
		return;
	}
	string text;
	httppeer->get_field("Content-Length",text);
	size64_t contentsize = Util::atoll(text.c_str());
	sint64 iBegin=0,iEnd=-1,iLen=0,iSize=0;
	if(206==result)
	{
		DOWNLOADSOURCE2_PRT("http-206 \n");
		if(httppeer->get_field("Content-Range",text)!= -1)//479742-2398709/2398710
		{
			int scan_count = sscanf(text.c_str(),"bytes %lld-%lld/%lld",&iBegin,&iEnd,&iSize);
			if (scan_count == 3)
			{
				iLen = iEnd - iBegin + 1;
			}
		}
		if(iLen<=0)
		{
			assert(0);
			httppeer->disconnect();
			return;
		}
	}
	if(iLen == 1)
	{
		DEBUGMSG("# ====>On_header: iSize=%lld \n",iSize);
	}
	if (m_fi->size == 0)
	{
		size64_t filesize = 0;
		if(result==200)
			filesize = contentsize;
		else //206
			filesize = iSize;
		if(0==filesize)
		{
			DEBUGMSG("#*** http no Content-Length or Content-Range:(%lld/%lld)!! \n",contentsize,iSize);
			httppeer->disconnect();
			return;
		}else
		{
			update_filesize(filesize);
			//DEBUGMSG("# ====>On_header:update_filesize=0x%x, iSize=%lld,filesize=%lld \n",(int)peer,iSize,filesize);
		}
	}
	else
	{
		if(iSize && iSize!=(sint64)m_fi->size)
		{
			DEBUGMSG("# ====>****On_header iSize=%lld,filesize=%lld \n",iSize,m_fi->size);
		}
	}
	if(200==result)
	{
		data->http_data_pos = 0;
		if (contentsize>0)
			data->http_data_remain = contentsize;
		else
			data->http_data_remain = -1;
		if(httppeer->get_req_begin()>0)
		{
			data->support_range = 0;
			data->support_keepalive = 0;
			DEBUGMSG("#******* http not support_range *** \n");
		}
	}
	else //if(206==result)
	{
		data->support_range = 1;
		data->http_data_pos = iBegin;
		data->http_data_remain = iLen;
	}
	if(0!=data->support_range && 0==SettingSngl::instance()->get_http_no_keep_alive() && httppeer->get_field("Connection",text)!= -1 && 0==stricmp(text.c_str(),"Keep-Alive"))
		data->support_keepalive = 1;
	else
		data->support_keepalive = 0;

	HttpSourceInfo* inf = http_get_source(peer->get_sessionID());
	if(inf)
	{
		inf->support_keepalive = data->support_keepalive;
		inf->support_range = data->support_range;
		inf->ip = peer->get_channel()->get_hip();
		inf->userDataTimes++;
	}
	//**********************************
	if(data->blockList.empty())
	{
		//可能已经断开
		if(-1==http_assign_job(httppeer))//可能同时创建了2两个http连接,所以第2个的回应时filesize已经取到,但任务还是要重分
			return;
		if(!httppeer->get_is_pause() && data->blockList.empty())
		{
			httppeer->disconnect();
			return;
		}
	}
	data->state = PS_DOWNLOADING;
	
	//当分配了任务，未收到回头前，已经被取消了任务，然后分配进入pause，再分配任务，再收到旧的头，下面就不一定会成立
	sint64 pos = (data->blockList.front().index * (sint64)m_fi->block_size + data->blockList.front().pos);
	//assert( pos >= data->http_data_pos);
	if(pos<data->http_data_pos)
	{
		uint32 cid = peer->get_sessionID();
		peer->disconnect();
		HttpSourceInfo *inf = http_get_source(cid);
		assert(inf);
		if(inf)
			http_create_channel_i(inf);
		return;
	}
	//DEBUGMSG("#http keepalive = %d ,rang=%d\n",data->support_keepalive,data->support_range);
}
void DownloadSource2::on(Data,Peer* peer,char* buf,int len)
{
	peer->m_last_active_tick = m_iCurrTick;
	HttpPeerData* data = m_http_peers.find(static_cast<HttpPeer*>(peer))->second;
	data->resume_state = 0;
	data->speedometer.add(len);
	m_speedometer.add(len);
	StatisticianSngl::instance()->OnDownloadBytes(len,peer->get_ip_type(),peer->get_user_type());
	if(HT_URL2==m_fi->hash.hash_type())
	{
		DownloadListManagerSngl::instance()->on_download_bytes(m_fi->hash,len,peer->get_ip_type(),peer->get_user_type());
	}

	if(data->state != PS_DOWNLOADING || m_isFinished)
	{
		//keeplive已经重新发请求,旧数据
		data->http_recv_count(len);
		return;
	}
	sint64 filepos = 0;
	unsigned int write_size=0;
	do
	{
		if(data->blockList.empty())
		{
			assert(0);
			data->http_recv_count(len);
			return;
		}
		BlockInfo& bi = data->blockList.front(); 
		filepos = bi.index * (sint64)m_fi->block_size + bi.pos;
		assert(filepos>=data->http_data_pos);
		if(filepos < data->http_data_pos)
		{
			peer->disconnect();
			return;
		}
		if((data->http_data_pos + len) <= filepos)
		{
			//中间有间隔的任务
			data->http_recv_count(len);
			len = 0;
			break;
		}
		else if(data->http_data_pos<filepos)
		{
			int offset = (int)(filepos-data->http_data_pos);
			data->http_recv_count(offset);
			assert(data->http_data_pos == filepos);
			buf += offset;
			len -= offset;
		}
		
		write_size = len;
		if(write_size > (bi.blockSize-bi.pos))
			write_size = bi.blockSize-bi.pos;

		int result =  FileStorageSngl::instance()->write_filedata(m_fi->hash, buf,write_size,
			bi.index,bi.pos);
		bi.pos += write_size;
		data->http_recv_count(write_size);
		buf += write_size;
		len -= write_size;
		//assert(write_size==result);
		if(result>0)
		{
			data->lagTimes = 0;
			if(bi.pos>=bi.blockSize)
			{
				int index = bi.index;
				del_block_ref(index,peer);
				data->blockList.pop_front();
				on_block_done(index,1);
				if(!m_isFinished)
				{
					cancel_job(index,true,true);
					if(data->blockList.empty())
					{
						//todo: 如果里面pause下载，将会有多余数据被忽略掉
						//返回0表示继续分配到相应任务不用重新发请求
						int ret = http_assign_job(static_cast<HttpPeer*>(peer));
						if(-1==ret || 1==ret)
						{
							return; 
						}
						else if(2==ret)
						{
							if(len>0)
								data->http_recv_count(len);
							return;
						}
						else if(0==ret)
						{
							//DEBUGMSG("# http unrequest for new assign_job = %d \n",len);
						}
					}
				}
				else
				{
					cancel_job(index,false,true);
					//on_file_done不要立即回调来删除自己，到hashmanager中通过timer回调
					//传递给下一个下载
					check_disconnect_reattach(static_cast<HttpPeer*>(peer),data);
					return;
				}
			}
		}
		else if(result==0)
		{
			//多连接同时下载一块
			data->lagTimes++;
			DEBUGMSG("#****http download slowly \n");
			if(data->lagTimes>=3)
			{
				//如果连续两次落后于人，则取消任务。试图重新分配下载。
				http_cancel_job_p(static_cast<HttpPeer*>(peer),-1,true);
				return;
			}
		}
		else
		{
			//不应该出现
			//assert(0);
		}
	}while(len>0);
	if(0==data->http_data_remain && !data->blockList.empty()&& 1==data->support_keepalive && 1==data->support_range)
		http_req_data(static_cast<HttpPeer*>(peer));
}
int DownloadSource2::check_disconnect_reattach(HttpPeer* peer,HttpPeerData* data)
{
	//连接传递需要考虑连接是否还存在剩余数据发过来
	if(HT_URL2==m_fi->hash.hash_type() && 1==data->support_keepalive && 0==data->http_data_remain)
	{
		if(1!=data->httpstate)
		{
			http_detach_peer(peer);
			if(0!=DownloadListManagerSngl::instance()->on_http_peer_reattach(m_fi->hash,peer))
			{
				peer->disconnect();
				m_http_pending_peerls.push_back(peer); //被detach后不会再回调disconnected()
			}
			return 0;
		}
		else
		{
			DEBUGMSG("#*****************requesting header*********************** \n");
		}
	}
	peer->disconnect();
	return -1;
}
int DownloadSource2::on_attach_http_peer(HttpPeer* peer,const string& url)
{
	DEBUGMSG("# DownloadSource2::on_attach_http_peer...\n");
	//检查连接数限制
	if((int)m_http_peers.size()>=m_http_max_num)
		return -1;
	HttpSourceInfo* inf = http_add_source(url,peer->get_sessionID(),false,1); //1成功连接过的
	if(!inf)
		return -1;
	if(0!=http_attach_peer(peer,inf))
		return -1;
	on(Connected(),peer);
	return 0;
}
int DownloadSource2::update_http_max_num(int n)
{
	m_http_max_num = n;

	//update 连接数导致的关闭连接不再reattach 重用.
	while(!m_http_peers.empty() && (int)m_http_peers.size()>m_http_max_num)
	{
		HttpPeerIter it = m_http_peers.begin();
		it->first->disconnect();
	}

	//暂时不考虑被暂停的情况
	if(/*!m_isHttpAssignJobPause &&*/ (int)m_http_peers.size()<m_http_max_num)
		http_create_channel(m_http_max_num-(int)m_http_peers.size());
	return 0;
}



