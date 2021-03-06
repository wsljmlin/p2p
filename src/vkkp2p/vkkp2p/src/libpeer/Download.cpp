#include "Download.h"
#include "FileStorage.h"
#include "Tracker.h"
#include "Statistician.h"
#include "PeerManager.h"
#include "Setting.h"
#include "DownloadManager.h"
#include "DownloadListManager.h"
#include "Util.h"
//#include "IniFile.h"
#include "MessagePusher.h"

#ifdef SM_DBG
#define DOWNLOAD_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "Download",__LINE__, ##arg)
#else
#define DOWNLOAD_PRT(fmt, arg...)
#endif


#define GET_PEERDATA_RETURN(_name,_peer,_ret) \
	PeerData * _name = get_peerdata(_peer); \
	if(NULL==_name) return _ret;



Download::Download(void)
: m_iStartTick(0)
, m_iLastSearchTick(0)
, m_iSearchCycle(120)
, m_iZeroSpeedCount(0)
{
}

Download::~Download(void)
{
	assert(0 == m_peers.size());
	clear_source_all();
	http_clear_source_all();
}

bool Download::create_new_download(const hash_t& hash,const string& path,const string& url/*=""*/,int http_max_num/*=-1*/,size64_t size/*=0*/,size64_t offset/*=0*/,
		unsigned int blockSize/*=DEFAULT_BLOCK_SIZE*/,int ftype/*=FTYPE_VOD*/,bool isStart/*=true*/,int rdbftype/*=RDBF_AUTO*/)
{
	if(DOWNS_INIT!=m_state)
	{
		assert(0);
		return false;
	}
#ifdef SM_VOD
	if(!(m_fi=FileStorageSngl::instance()->create_downinfo(hash,get_playtype(),path,size,offset,blockSize,ftype,rdbftype)))
		return false;
#else
	if(!(m_fi=FileStorageSngl::instance()->create_downinfo(hash,path,size,offset,blockSize,ftype,rdbftype)))
		return false;
#endif /* end of SM_VOD */

	m_fi->url = url;

	//-1时表示不指定,则使用本地配置
	if(-1==http_max_num)
		http_max_num = SettingSngl::instance()->get_downloadhttpi_cnns();
	////如果没有登陆P2P tracker的情况
	//if(http_max_num<=0 &&!TrackerSngl::instance()->is_login())
	//	http_max_num = 1;

	m_http_max_num = http_max_num;
	m_downInfo.hash = m_fi->hash;
	m_downInfo.tth = m_fi->tth;
	m_downInfo.size = m_fi->size;
	m_downInfo.reqOffset = 0;
	m_downInfo.block_offset = m_fi->block_offset;
	m_downInfo.blocks = m_fi->blocks;
	m_downInfo.bufferingSize = 0;
	m_downInfo.streamSize = 0;
	m_downInfo.ftype = m_fi->ftype;
	m_downInfo.path = m_fi->path;
	m_downInfo.state = 0;
	m_downInfo.down_blocks = m_fi->down_blocks;
	m_downInfo.speedB = 0;
	m_downInfo.srcNum = 0;
	m_downInfo.connNum = 0;
	m_state=DOWNS_STOP;
	fire(DownloadListener::DLCreate(),m_downInfo);
	if(!url.empty())
		http_add_source(url);
	if(isStart) 
		start();
	return true;
}
bool Download::create_old_download(FileInfo *fi,const string& url/*=""*/,int http_max_num/*=2*/,bool isStart/*=true*/)
{
	if(DOWNS_INIT!=m_state)
	{
		assert(0);
		return false;
	}
	if(fi != FileStorageSngl::instance()->get_downinfo(fi->hash))
	{
		assert(0);
		return false;
	}

	//-1时表示不指定,则使用本地配置
	if(-1==http_max_num)
		http_max_num = SettingSngl::instance()->get_downloadhttpi_cnns();
	////如果没有登陆P2P tracker的情况
	//if(http_max_num<=0 &&!TrackerSngl::instance()->is_login())
	//	http_max_num = 1;

	m_http_max_num = http_max_num;
	m_fi = fi;
	m_fi->url = url;
	m_downInfo.hash = m_fi->hash;
	m_downInfo.tth = m_fi->tth;
	m_downInfo.size = m_fi->size;
	m_downInfo.reqOffset = 0;
	m_downInfo.block_offset = m_fi->block_offset;
	m_downInfo.blocks = m_fi->blocks;
	m_downInfo.bufferingSize = 0;
	m_downInfo.streamSize = 0;
	m_downInfo.ftype = m_fi->ftype;
	m_downInfo.path = m_fi->path;
	m_downInfo.state = 0;
	m_downInfo.down_blocks = m_fi->down_blocks;
	m_downInfo.speedB = 0;
	m_downInfo.srcNum = 0;
	m_downInfo.connNum = 0;
	m_state=DOWNS_STOP;
	fire(DownloadListener::DLCreate(),m_downInfo);
	if(!url.empty())
		http_add_source(url);
	if(isStart) 
		start();
	return true;
}
int Download::start()
{
	assert(DOWNS_INIT!=m_state);
	if(m_isFinished)
		return 0;
	if(DOWNS_START==m_state)
		return 0;
	if(m_fi->is_finished())
	{
		this->on_file_done();
		return 0;
	}
	
	DEBUGMSG("#Download::start(%s)\n",m_fi->tth.c_str());

	if(HT_URL2!=m_fi->hash.hash_type())
		MessagePusherSngl::instance()->on_download_file_start(m_fi->url);
	m_state = DOWNS_START;
#ifdef SM_VOD
	TrackerSngl::instance()->PTL_ReportStartDownloadFile(m_fi->hash, get_playtype());
#else
	TrackerSngl::instance()->PTL_ReportStartDownloadFile(m_fi->hash);
#endif /* end of SM_VOD */
	m_iStartTick = m_iCurrTick;
	m_downInfo.begintime = (unsigned int)time(NULL);
	m_speedometer.reset();
	m_fi->req_offset = 0;
	m_fi->last_req_offset = 0;
	m_fi->last_req_offset = 0;
	m_downInfo.state = 1;
	StatisticianSngl::instance()->OnDownloadStart();
	fire(DownloadListener::DLStart(),m_downInfo);
	fire(DownloadListener::DLBlockTable(),m_fi->hash,m_fi->bt_memfinished);
	load_client_sn(); //尝试获取clientconf.ini中的SN
	create_channel();
	if(!TrackerSngl::instance()->is_login())
		load_source_server_cache();
	search_source();
	////test:
	//this->http_add_source("http://127.0.0.1:7080/vttv/bdac5c0e344778158017e063ac44ed89e492785f.rm");
	http_create_all_channel();
	m_iZeroSpeedCount = 0;
	return 0;
}

int Download::stop()
{
	if(DOWNS_STOP == m_state)
		return 0;
	
	if(HT_URL2!=m_fi->hash.hash_type())
		MessagePusherSngl::instance()->on_download_file_stop(m_fi->url);

	DEBUGMSG("#Download::stop(%s)\n",m_fi->tth.c_str());

	m_state = DOWNS_STOP;
	put_peer_all();
	http_put_peer_all();
	http_deal_pending();
	assert(m_blockRefs.empty());
	FileStorageSngl::instance()->flush_file_all(m_fi);
	StatisticianSngl::instance()->OnDownloadStop();
	TrackerSngl::instance()->PTL_ReportStopDownloadFile(m_fi->hash);

	unsigned int speed = (unsigned int)m_speedometer.get_speed();
	unsigned int seconds = m_speedometer.get_sec_amount();
	StatisticianSngl::instance()->OnConnectionSpeed(0,speed,seconds,0,0,true);
	//上报速度信息
#ifdef SM_VOD
	//T.B.D if you want to print every download speed information, please comment  bellow "&& HT_URL2!=m_fi->hash.hash_type()"
	if(m_fi->size > 0&&seconds>15 && HT_URL2!=m_fi->hash.hash_type())
#else
	if(m_fi->size > 0&&seconds>15 && HT_URL2!=m_fi->hash.hash_type())
#endif /* end of SM_VOD */
	{
		PTL_P2T_ReportDownloadFileSpeed inf;
		memset(&inf,0,sizeof(inf));
		memcpy(inf.fhash,m_fi->hash.buffer(),HASHLEN);
		inf.size = m_fi->size;
		inf.speed = (speed+512)>>10;
		inf.downSeconds = seconds;
		TrackerSngl::instance()->PTL_ReportDownloadFileSpeed(inf);
	}
	m_speedometer.reset();
	m_downInfo.state = 0;
	fire(DownloadListener::DLStop(),(void*)this,m_downInfo);
	return 0;
}

int Download::on_second()
{
	m_iCurrTick++;
	if(DOWNS_START==m_state && !m_isFinished)
	{
		m_downInfo.seconds++;
		http_deal_pending();
		//第5秒没有数据源，使用本地cache服务器
		if( m_sources.empty() && (m_iStartTick+4)==m_iCurrTick )
			load_source_server_cache();
		http_resume_assign_job();
		resume_assign_job();
		auto_change_demand_point();
		check_assign_fails();

		m_downInfo.down_blocks = m_fi->down_blocks;
		m_downInfo.speedB = (int)m_speedometer.get_speed(5);
		m_downInfo.connNum = m_iConnectAmount;
		m_downInfo.httpConnNum = m_ihttpConnectAmount;
		m_downInfo.srcNum = (int)m_sources.size();
		fire(DownloadListener::DLDownloading(),m_downInfo);

		for(PeerIter it=m_peers.begin();it!=m_peers.end();++it)
		{
			if(!it->second->isReady)
				continue;
			if(PS_ASSIGN_JOB_PAUSE!=it->second->state)
				it->second->speedometer.on_second();
		}

		{
			for(HttpPeerIter it=m_http_peers.begin();it!=m_http_peers.end();++it)
			{
				if(!it->second->isReady)
					continue;
				if(PS_ASSIGN_JOB_PAUSE!=it->second->state)
					it->second->speedometer.on_second();
			}
		}

		m_speedometer.on_second();

		if(m_iLastCreateConnectTick+60<m_iCurrTick)
		{
			create_channel();
			if(this->http_check_need_add_connection())
				http_create_channel();
		}

		if(m_iCurrTick%2==0)
		{
			if(http_deal_deadpeer()>0)
				http_create_channel();
			int speed = (int)m_speedometer.get_speed(3)>>10;
			if(0==speed)
			{
				if(!m_isAssignJobPause && !m_isHttpAssignJobPause)
					m_iZeroSpeedCount++;
			}
			else
				m_iZeroSpeedCount = 0;

			DEBUGMSG("speed_KB=%d\n",speed);
			//test:
			if(0==speed && m_http_peers.empty() && m_peers.empty())
			{
				////无连接无速度时立即建立连接.20160316取消.因为影响P2P分享控制策略
				//if(m_http_max_num==0)
				//	m_http_max_num = 1; //无速度时临时调为1
				create_channel();
				if(this->http_check_need_add_connection())
					http_create_channel();
			}
			m_iSearchCycle = 120;
			if(speed < 70)
				m_iSearchCycle = 120;  //0~70 KB 的2分钟搜索一次
			else if(speed < 120)
				m_iSearchCycle = 300;  //70~120 KB 的5分钟搜索一次
			else if(speed < 200)
				m_iSearchCycle = 600;  //120~200 KB 的10分钟搜索一次
			else
				m_iSearchCycle = 1200; //0~70 KB 的20分钟搜索一次
		}
		if(m_sources.empty())
		{
			if(m_iLastSearchTick + 60 < m_iCurrTick)
			{
				search_source();
			}
		}
		else if(m_iLastSearchTick + m_iSearchCycle < m_iCurrTick && m_sources.size()<60)
		{
			search_source();
		}
	}
	else
	{
	}
	return 0;
}
const DownloadInfo& Download::get_downloadinfo()
{
	m_downInfo.reqOffset = m_fi->req_offset; 
	int need_i = (int)(m_fi->req_offset/m_fi->block_size);
	int blocks = m_fi->bt_memfinished.get_block_num();// m_fi->tableMDFinished.GetBlocksNum();
	int n = 0,m = 0;
	int i,j;
	bool b = true;
	for(i=need_i,j=0;i<blocks && j<SettingSngl::instance()->get_memcache_win();++i,++j)
	{
		if(m_fi->bt_memfinished[i])
		{
			n++;
			if(b)
				m++;
		}
		else
			b = false;
	}
	if(0==n)
		m_downInfo.bufferingSize = n*(uint64)m_fi->block_size;
	else
		m_downInfo.bufferingSize = n*(uint64)m_fi->block_size - (m_fi->req_offset%m_fi->block_size);

	if(0==m)
		m_downInfo.streamSize = m*(uint64)m_fi->block_size;
	else
		m_downInfo.streamSize = m*(uint64)m_fi->block_size - (m_fi->req_offset%m_fi->block_size);
	return m_downInfo;
}

int Download::on_peer_ready(Peer* peer)
{
	GET_PEERDATA_RETURN(data,peer,-1)
	data->isReady = true;
	return this->PTL_RequestFileBlockTable(peer);
}

void Download::on_peer_close(Peer* peer)
{
	StatisticianSngl::instance()->OnConnection(peer->get_conn_type(),peer->get_conn_success());
	if(HT_URL2==m_fi->hash.hash_type())
		DownloadListManagerSngl::instance()->on_connection(m_fi->hash,peer->get_conn_type(),peer->get_conn_success());
	detach_peer(peer);
}

int Download::on_tracker_connected()
{
	search_source();
	return 0;
}


int Download::ON_PTL_ResponseFileSource(Peer* peer,PTL_P2T_ResponseFileSource& inf)
{
	DOWNLOAD_PRT("receive file source peer, num=%d\n", inf.num);
#ifdef SM_VOD
	if(PLAYTYPE_VOD==get_playtype()) {
		static int  httpcnns_max ;
		static int httpcnns_max_prev = -1;
		httpcnns_max= (inf.num >= (unsigned int)SettingSngl::instance()->get_http_close_peercnt())?0:SettingSngl::instance()->get_downloadhttpi_cnns();
		if(httpcnns_max!=httpcnns_max_prev) {
			update_http_max_num(httpcnns_max);
			DOWNLOAD_PRT("update http max connection:%d\n",httpcnns_max); 
			httpcnns_max_prev = httpcnns_max;
		}	
	}
#endif /* end of SM_VOD */

	if(0==m_sources.size() && inf.num<=0)
		load_source_server_cache();
	if(0!=inf.result)
		return -1;
	if(0==m_fi->size)
		update_filesize(inf.fsize);

	if(inf.urlflag>5) 
		inf.urlflag = 5;
	////todo:返回源时再设置使用http连接数其实有问题
	//if(SettingSngl::instance()->get_downloadhttpi_cnns()<0)
	//	m_http_max_num = inf.urlflag;

	for(uint32 i=0; i<inf.num; i++)
#ifdef SM_DBG
	{
		DOWNLOAD_PRT("will download %s\n", m_downInfo.path.c_str());
		DOWNLOAD_PRT("receive filesource peer%d %s:%d, and self %s:%d\n", i, Util::ip_htoas(inf.peers[i].tcpRealIP).c_str(), inf.peers[i].tcpRealPort, Util::ip_htoas(g_netLiveInfo.tcpRealIP).c_str(), g_netLiveInfo.tcpRealPort);
		DOWNLOAD_PRT("receive filesource peer localip %s:%d, and self %s:%d\n",Util::ip_htoas(inf.peers[i].tcpLocalIP).c_str(), inf.peers[i].tcpLocalPort, Util::ip_htoas(g_netLiveInfo.tcpLocalIP).c_str(), g_netLiveInfo.tcpLocalPort);
#else
#endif /* end of SM_DBG */
		this->add_source(&inf.peers[i]);
#ifdef SM_DBG
	}
#else
#endif /* end of SM_DBG */
	create_channel();
	return 0;
}

int Download::ON_PTL_ResponseFileBlockTable(Peer* peer,PTL_P2P_ResponseFileBlockTable& inf)
{
	GET_PEERDATA_RETURN(data,peer,-1)
	if((0!=inf.result && 1!=inf.result) || inf.fsize==0)
	{
		DEBUGMSG("#*** request table fail! no file !*** \n");
		data->bt_info.assign_fails++;
		if(data->bt_info.assign_fails>20)
		{
			DEBUGMSG("#*** request table fail! kill peer \n");
			detach_peer(peer);
			return -1;
		}
		data->state = PS_ASSIGN_FAIL_WAITING;
		m_assign_fails++;
		return 0;
	}
	if(m_fi->size==0)
	{
		if((0==inf.result||1==inf.result)&&inf.fsize>0)
			this->update_filesize(inf.fsize);
	}
	if(m_fi->size==0 || m_fi->size!=inf.fsize)
	{
		//assert(0);
		detach_peer(peer);
		return -1;
	}
	if(inf.blockSize != m_fi->block_size)
	{
		DEBUGMSG("# ***block size wrong!!! \n");
		detach_peer(peer,false,true);
		return -1;
	}
	//assert(inf.blockSize == m_fi->block_size);
	if(0==data->bt_finished.get_block_num())
	{
		data->bt_finished.resize(m_fi->blocks,m_fi->block_offset,false);
	}
	data->bt_info.last_update_tick = m_iCurrTick;
	if(inf.result == 1)
	{
		data->bt_finished.set_all_block(true);
		data->bt_info.state = 1;
	}
	else if(inf.result == 0)
	{
		assert(inf.blockSize == m_fi->block_size);
		if(inf.blockSize != m_fi->block_size || 0 == inf.num)
		{
			assert(0);
			detach_peer(peer);
			return -1;
		}
		unsigned int real_off=0;
		unsigned int real_len=0;
		data->bt_finished.set_block_buf(inf.tableBuf,inf.num,inf.startBufI,real_off,real_len);
		if(real_len)
		{
			data->bt_info.index_offset = (inf.startBufI+real_off) * 8;
			data->bt_info.index_num = real_len*8;
		}
		else
		{
			assert(false); //其实可能发生，因为在这过程中更新了下载窗口
			detach_peer(peer);
			return -1;
		}
	}
	else
	{
		data->state = PS_NOTHING_TO_DOWNLOAD; //丢弃该源
		detach_peer(peer);
		return -1;
	}

	return assign_job(peer,false);
}

int Download::ON_PTL_ResponseFileBlocks(Peer* peer,PTL_P2P_ResponseFileBlocks& inf)
{
	GET_PEERDATA_RETURN(data,peer,-1)
	assert(0==inf.blockSize || inf.blockSize == m_fi->block_size);
	bool bcancel = false;
	for(unsigned int i=0;i<inf.num;++i)
	{
		if(0==inf.blockState[i])
		{
			cancel_job(peer,inf.indexs[i]);
			bcancel = true;
		}
	}
	if(data->blockList.empty())
	{
		//可能刚刚分配到本块，本块就已经被其它连接下载完成，并且取消了本块的任务，对方收到取消之前发回复的信息
		if(bcancel)
		{
			detach_peer(peer);
			return -1;
		}
	}
	if(PS_ASSIGN_JOB_PAUSE!=data->state)
		data->state = PS_DOWNLOADING;
	return 0;
}

int Download::ON_PTL_ResponseFileBlocksData(Peer* peer,PTL_P2P_ResponseFileBlocksData& inf)
{
	GET_PEERDATA_RETURN(data,peer,-1)
	assert(inf.blockSize == m_fi->block_size);
	data->speedometer.add(inf.size);
	m_speedometer.add(inf.size);
	StatisticianSngl::instance()->OnDownloadBytes(inf.size,peer->get_ip_type(),peer->get_user_type());
	if(HT_URL2==m_fi->hash.hash_type())
	{
		DownloadListManagerSngl::instance()->on_download_bytes(m_fi->hash,inf.size,peer->get_ip_type(),peer->get_user_type());
	}
	list<BlockInfo>::iterator it;
	for(it=data->blockList.begin();it!=data->blockList.end();++it)
	{
		if((*it).index == inf.blockIndex)
			break;
	}
	if(it==data->blockList.end())
	{
		//过时数据,这可能发生，例如块已经被取消
		return -1;
	}
	BlockInfo& bi = *it;
	if(inf.size<=0 || inf.offset!=bi.pos)
	{
		//可能性极低，除非刚取消了此块，又重新分配了此块
		return -1; 
	}
	int size = inf.size;
	if(inf.offset+size>bi.blockSize)
	{
		assert(0);
		size = bi.blockSize-inf.offset;
	}

	DOWNLOAD_PRT("%s receive fileblock data from %d, and will save %d shared data.......\n", m_fi->despath.c_str(),data->sourceSessionID,size);
	int ret = FileStorageSngl::instance()->write_filedata(m_fi->hash,inf.data,size,inf.blockIndex,inf.offset);
	bi.pos += size;
	if(ret>0)
	{
		data->lagTimes = 0;
		if(bi.pos>=bi.blockSize)
		{
			del_block_ref(inf.blockIndex,peer);
			data->blockList.erase(it);
			on_block_done(inf.blockIndex,0);
			if(!m_isFinished)
			{
				cancel_job(inf.blockIndex,true,true);
				//完成1块尝试继续分任务保持连续性
				//if(data->blockList.empty())
					return assign_job(peer);
			}
			else
			{
				cancel_job(inf.blockIndex,false,true);
				detach_peer(peer,true);
			}
			return 0;
		}
	}
	else if(0==ret)
	{
		data->lagTimes++;
		int blockSpeed = this->get_block_speed(inf.blockIndex,5);
		int peerSpeed = data->speedometer.get_speed(5);
		//我速度比下载同块的其它PEER速度低于6K左右，不再下载此块
		if(data->lagTimes > 3 || (peerSpeed+6000)<blockSpeed)
		{
			data->lagTimes = 0;
			DEBUGMSG("#***write data 0, cancel_job  \n");
			cancel_job(peer,inf.blockIndex,true,true);
		}
	}
	else
	{
	}
	return 0;
}
int Download::on_peer_readlimit(Peer* peer)
{
	return StatisticianSngl::instance()->GetMaxRecvAllow();
}

void Download::search_source()
{
	if(m_isUseClientSNOnly)
		return;
#ifdef SM_VOD
	if(DOWNS_START != m_state || m_isFinished)
		return;
	if(PLAYTYPE_LIVE!=get_playtype())
		TrackerSngl::instance()->PTL_RequestFileSource(m_fi->hash, get_playtype());
#else
	//url2类型不搜索
	if(HT_URL2==m_fi->hash.hash_type())
		return;
	if(DOWNS_START != m_state || m_isFinished)
		return;
	TrackerSngl::instance()->PTL_RequestFileSource(m_fi->hash);
#endif
	m_iLastSearchTick = m_iCurrTick;
}


template<typename T>
int Download::send_PTL_packet(Peer* peer,uint16 cmd,T& inf,int iMaxSize)
{
	NEW_MBLOCK_RETURN_INT(block,iMaxSize,-1)
	PTLStream ss(block->buf,block->buflen,0);
	m_head.cmd = cmd;
	ss << m_head;
	ss << inf;
	ss.fitsize32(4);
	block->datalen = ss.length();
	return peer->send(block);
}



int Download::PTL_RequestFileBlockTable(Peer* peer)
{
	GET_PEERDATA_RETURN(data,peer,-1)
	if(data->state==PS_REQUESTING_FILE_BLOCK)
	{
		DEBUGMSG("#********* wrong state **** \n");
		//改有不间断分任务机制时可能发生到此情况
		return 0;
	}
	data->state = PS_REQUESTING_FILE_TABLE;

	//if(m_fi->size>0)
	//{
	//	//忽略请求BblockTable,在此假设对方已经有完整数据
	//	PTL_P2P_ResponseFileBlockTable inf;
	//	memcpy(inf.fhash,m_fi->hash.buffer(),HASHLEN);
	//	inf.result = 1;
	//	inf.fsize = m_fi->size;
	//	inf.blockSize = m_fi->block_size;
	//	inf.num = 0;
	//	inf.startBufI = 0;
	//	return ON_PTL_ResponseFileBlockTable(peer,inf);
	//}

	PTL_P2P_RequestFileBlockTable req;
	memcpy(req.fhash,m_fi->hash.buffer(),HASHLEN);
	req.blockSize = m_fi->block_size;
	req.maxnum = 1024;  //一次最多要1024个
	//考虑还没有文件大小时的情况
	////test:
	//req.maxnum = 1;
	req.startBufI = get_update_blocktable_index_offset(data)/8;
	return send_PTL_packet(peer,PTL_P2P_REQUEST_FILE_BLOCK_TABLE,req,1024);
}

int Download::PTL_RequestFileBlocks(Peer* peer)
{
	GET_PEERDATA_RETURN(data,peer,-1)
	if(data->blockListNew.empty()) return 0;
	data->state = PS_REQUESTING_FILE_BLOCK;

	PTL_P2P_RequestFileBlocks req;
	memcpy(req.fhash,m_fi->hash.buffer(),HASHLEN);
	req.blockSize = m_fi->block_size;
	req.num = data->blockListNew.size();
	int i=0;
	for(list<BlockInfo>::iterator it=data->blockListNew.begin();it!=data->blockListNew.end();++it,++i)
	{
		BlockInfo& bi = *it;
		if(!data->exist_job(bi.index))
		{
			req.indexs[i] = bi.index;
			req.offsets[i] = bi.pos;
			data->blockList.push_back(bi);
		}
		else
		{
			assert(false);
		}
	}
	data->blockListNew.clear();
	//if(data->blockList.size()<SettingSngl::instance()->get_assign_num())
	//{
	//	printf("#------ assign num = %d ----- \n",data->blockList.size());
	//}
	DOWNLOAD_PRT("%s request fileblocks.........................!\n", m_downInfo.path.c_str());
	return send_PTL_packet(peer,PTL_P2P_REQUEST_FILE_BLOCKS,req,1024);
}
//目前先支持取消一个
int Download::PTL_CancelFileBlocks(Peer* peer,int index)
{
	assert(index >= 0);
	PTL_P2P_CancelFileBlocks req;
	memcpy(req.fhash,m_fi->hash.buffer(),HASHLEN);
	req.num = 1;
	req.indexs[0] = index;
	return send_PTL_packet(peer,PTL_P2P_CANCEL_FILE_BLOCKS,req,1024);
}



unsigned int Download::get_update_blocktable_index_offset(PeerData* data)
{
	//返回的结果要与assign_job()逻辑相关，保证获取到的块表将是要assign_job()的块表
	if(0==m_fi->size)
		return 0;
	unsigned int need_i = (unsigned int)(m_fi->req_offset/m_fi->block_size);
	unsigned int index_up = m_fi->block_offset + m_fi->blocks;
	while(need_i<index_up)
	{
		if(!m_fi->bt_memfinished[need_i])
			return need_i;
		need_i++;
	}
	return m_fi->block_gap;
}
int Download::assign_job(Peer *peer,bool allowReqBlockTable/*=true*/)
{
	GET_PEERDATA_RETURN(data,peer,-1)
	data->state = PS_REQUESTING_FILE_TABLE;
	unsigned int MAX_BLOCK_NUM = SettingSngl::instance()->get_assign_num();
	////可能不为空,目前不知道为什么在有任务的时候，对方返回table
	if(data->blockList.size()>=MAX_BLOCK_NUM)
		return 0;
	
	assert(m_fi->size>0);
	unsigned int block_nums = MAX_BLOCK_NUM-data->blockList.size();

	const BlockTable& source_table = data->bt_finished;
	
	BlockInfo bi;
	bi.index = 0;
	int i=0,min_i=-1;
	int blocks = m_fi->block_offset + m_fi->blocks;

	list<int> ls;
	int need_i = (int)(m_fi->req_offset/m_fi->block_size);
	int speed = data->speedometer.get_speed(5);
	int urgent_win = need_i+SettingSngl::instance()->get_urgent_win();
	int smooth_win = need_i+SettingSngl::instance()->get_smoot_win();
	unsigned int random_win = SettingSngl::instance()->get_random_win()+block_nums;
	int pause_win = need_i + SettingSngl::instance()->get_memcache_win();
#ifdef SM_VOD
	//vod mode will download more blocks
	if(PLAYTYPE_VOD==get_playtype()) {
		urgent_win += 6;
		smooth_win += 8; 
	}
#endif 

	i = need_i;
	//注意窗口模式起作用的前提为播播放器端接收数据不能有几多收几多，保持播放速度才行，否则need_i总是一直快速推前
	//1.抢紧急块
	if(speed>0)
	{
		int need_i_speed =0;
		for(i=need_i;i<blocks&&i<urgent_win;++i)
		{
			if(-1==min_i && !m_fi->bt_memfinished[i] && source_table[i])
				min_i = i;

			if(m_fi->bt_memfinished[i] || !source_table[i]|| data->exist_job(i))
				continue;
			need_i_speed = get_block_speed(i,5);
			if(0==need_i_speed || (need_i_speed < (DEFAULT_STREAM_SPEED/3) && (need_i_speed*2)<speed))
			{
				DEBUGMSG("#:rob important_block => %d \n",i);
				ls.push_back(i);
				random_win = block_nums+1;
				if(ls.size()>=block_nums)
					break;
			}
		}
	}

	//2.平稳窗,随机窗为1
	for(;i<blocks && i<smooth_win && i<pause_win && ls.size()<random_win;++i)
	{
		if(-1==min_i && !m_fi->bt_memfinished[i] && source_table[i])
			min_i = i;
		if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i) && source_table[i] && !data->exist_job(i))
		{
			ls.push_back(i);
			random_win = block_nums+1; //让查找random_win 找有范围可以找
		}
	}

	//3.pause 窗
	for(;i<blocks && i<pause_win && ls.size()<random_win;++i)
	{
		if(-1==min_i && !m_fi->bt_memfinished[i] && source_table[i])
			min_i = i;
		if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i) && source_table[i] && !data->exist_job(i))
			ls.push_back(i);
	}

	//因为随机，取消长距离抢夺
	////3.分析一下最靠前未完成的块与当前分配到的块的距离，如果太远，就考虑抢块下载
	//if(!ls.empty() && min_i!=data->lagBlockI && min_i+30<ls.front())
	//{
	//	//距离10块以上
	//	int min_i_speed = get_block_speed(min_i,5);
	//	if(speed > 3*min_i_speed)
	//	{
	//		ls.front() = min_i;
	//		DEBUGMSG("#---Long Block Distance ,rob(%d) \n",min_i);
	//	}
	//}

	if(ls.empty() && m_fi->is_allow_pause())
	{
		//在此一定要抢速度慢的下载
		int tmp = 1000000,tmp2=0,index=-1;
		for(i=need_i;i<blocks && i<urgent_win && i<pause_win && ls.size()<block_nums;++i)
		{
			if(!m_fi->bt_memfinished[i] && source_table[i] && !data->exist_job(i))
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
		else
		{
			data->state = PS_ASSIGN_JOB_PAUSE;
			m_isAssignJobPause = true;
			m_lastPauseNeedI = need_i;
			DEBUGMSG("---AssignJobPause!--\n");
			return 0;
		}
	}
	if(ls.empty())
	{
		//一直向后寻找
		for(;i<blocks && ls.size()<random_win;++i)
		{
			if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i) && source_table[i] && !data->exist_job(i))
				ls.push_back(i);
		}

		if(ls.empty() && !m_fi->is_memcache_only())
		{
			//分配其它空闲块
			for(i=m_fi->block_gap;i<blocks && ls.size()<random_win;++i)
			{
				if(!m_fi->bt_memfinished[i] && 0==get_block_ref_num(i) && source_table[i] && !data->exist_job(i))
					ls.push_back(i);
			}
		}
	}

	//.抢速度慢的块
	if(ls.empty())
	{
		int tmp = 1000000,tmp2=0,index=-1;
		for(i=m_fi->block_gap; i<blocks && ls.size()<block_nums; ++i)
		{
			if(!m_fi->bt_memfinished[i] && source_table[i] && !data->exist_job(i))
			{
				tmp2 = get_block_speed(i,5);
				if(tmp2<=tmp)
				{
					index = i;
					tmp = tmp2;
				}
			}
		}
		if(index!=-1 && (speed<=0 || (speed>0 && tmp*2<speed)))
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
			if((*it)==data->lagBlockI)
				continue;
			////在此只加连续块任务
			//if(data->blockListNew.empty() || (i+1)==*it)
			{
				i = *it;
				bi.index = i;
				bi.pos = m_fi->get_block_downing_size(i);
				bi.blockSize = m_fi->get_block_size(i);
				if(bi.pos>=bi.blockSize)
				{
					on_block_done(i,0);
					if(m_isFinished)
						return -1;
				}
				else
				{
					add_block_ref(i,peer);
					data->blockListNew.push_back(bi);
				}
			}
			if(data->blockListNew.size()>=block_nums)
				break;
		}
	}

	if(data->blockList.empty() && data->blockListNew.empty())
	{
		data->bt_info.assign_fails++;
		if(allowReqBlockTable && 0==data->bt_info.state)
		{
			return PTL_RequestFileBlockTable(peer);
		}
		//大于5次分配失败，即断开
		if(data->bt_info.assign_fails>20)
		{
			DEBUGMSG("#*** assign job fail! kill peer \n");
			data->state = PS_NOTHING_TO_DOWNLOAD;
			detach_peer(peer);
			return -1;
		}
		if(1==data->bt_info.state)
		{
			detach_peer(peer,true);
			return -1;
		}
		data->state = PS_ASSIGN_FAIL_WAITING;
		m_assign_fails++;
		return 0; //等待5秒后再次开启
	}
	data->bt_info.assign_fails = 0;
	//落后不是永远
	data->lagBlockI = -1;
	PTL_RequestFileBlocks(peer);
	return 0;
}

void Download::resume_assign_job()
{
	if(!m_isAssignJobPause)
		return;
	int need_i = (int)(m_fi->req_offset/m_fi->block_size);
	if(need_i < m_lastPauseNeedI || (need_i-m_lastPauseNeedI)>(SettingSngl::instance()->get_memcache_win()/3) || !is_urgent_win_ok(need_i))
	{
		//当请求点前移或者可用窗口数据不足2/3时，启动下载,(注意:当指针移到文件尾倒数2/3窗口以内时，会不断执行此动作)
		m_isAssignJobPause = false;
		Peer *peer = NULL;
		PeerData *data = NULL;
		for(PeerIter it=m_peers.begin();it!=m_peers.end();++it)
		{
			peer = it->first;
			data = it->second;
			if(PS_ASSIGN_JOB_PAUSE==data->state)
			{
				//如果有删除节点，it指针会受到破坏
				if(-1==assign_job(peer))
					it=m_peers.begin();
			}
		}
		DEBUGMSG("...resume_assign_job...\n");
	}
}
void Download::check_assign_fails()
{
	if(0==m_assign_fails)
		return;
	unsigned int i=0;
	for(PeerIter it=m_peers.begin();it!=m_peers.end();++it)
	{
		if(PS_ASSIGN_FAIL_WAITING == it->second->state)
		{
			if(it->second->bt_info.last_update_tick+2<=m_iCurrTick)
			{
				m_assign_fails--;
				PTL_RequestFileBlockTable(it->first);
			}
			if(++i>=m_assign_fails)
				break;
		}
	}
}
int Download::cancel_job(Peer *peer,int index/*=-1*/,bool allowTryAssign/*=false*/,bool sendReqCancel/*=false*/)
{
	if(IPT_HTTP==peer->get_ip_type())
		return http_cancel_job_p(static_cast<HttpPeer*>(peer),index,allowTryAssign);
	GET_PEERDATA_RETURN(data,peer,-1)
	list<BlockInfo>::iterator it;
	for(it=data->blockList.begin();it!=data->blockList.end();)
	{
		if(-1==index || index==(int)(*it).index)
		{
			if(sendReqCancel)
				PTL_CancelFileBlocks(peer,(*it).index);
			del_block_ref((*it).index,peer);
			it = data->blockList.erase(it);
			if(-1!=index)
				break;
		}
		else
			++it;
	}
	data->lagBlockI = index;
	if(allowTryAssign && data->blockList.empty())
	{
		return assign_job(peer);
	}
	return 0;
}

int Download::cancel_job(int index,bool allowTryAssign/*=false*/,bool sendReqCancel/*=false*/)
{
	assert(-1 != index);
	if(-1==index)
		return 0;

	BlockRefIter it = m_blockRefs.find(index);
	if(it==m_blockRefs.end())
		return 0;
	list<Peer*> ls = it->second;
	for(list<Peer*>::iterator it2=ls.begin();it2!=ls.end();++it2)
		cancel_job(*it2,index,allowTryAssign,sendReqCancel);
	assert(m_blockRefs.find(index) == m_blockRefs.end());
	return 0;
}


int Download::auto_change_demand_point(bool isReAssign/*=true*/)
{
	if(m_fi->size<=0)
		return -1;
	int i = (int)(m_fi->req_offset / m_fi->block_size);
	//m_fi->last_req_offset==m_fi->req_offset 表示已经调整过下载点或者连接获取，!=表示跳跃
	if(m_fi->last_req_offset!=m_fi->req_offset && !m_fi->bt_memfinished[i] && 0==get_block_ref_num(i) && !m_peers.empty())
	{
		//DEBUGMSG("#:move new block (%d) down...\n",i);
		if(isReAssign)
		{
			PeerIter it = m_peers.begin();
			PeerIter curr_it = it;
			for(++it;it!=m_peers.end();++it)
			{
				if(!it->second->bt_finished.is_range(i))//搜索中的peer
					continue;
				else if(!curr_it->second->bt_finished.is_range(i)) //搜索中的peer
					curr_it = it;
				else if(it->second->bt_finished[i] && curr_it->second->speedometer.get_speed(5) < it->second->speedometer.get_speed(5))
					curr_it = it;
			}
			if(curr_it->second->bt_finished.is_range(i) && curr_it->second->bt_finished[i])
			{
				cancel_job(curr_it->first,-1,true,true);
			}
		}
		create_channel();
		if(!check_need_win_done())
			http_create_channel();
	}
	m_fi->last_req_offset=m_fi->req_offset;
	return 0;
}

bool Download::check_need_win_done()
{
	//查连接4个块窗口大小是否完成
	int i = (int)(m_fi->req_offset / m_fi->block_size);
	int blocks = m_fi->block_offset + m_fi->blocks;
	int n = 0;
	for(;i<blocks&&n<8;++i,++n)
	{
		if(!m_fi->bt_memfinished[i])
			return false;
	}
	return true;
}
