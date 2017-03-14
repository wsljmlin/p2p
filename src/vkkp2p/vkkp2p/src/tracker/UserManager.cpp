#include "UserManager.h"
#include "PeerManager.h"
#include "SourceService.h"
#include "StatManager.h"
#include "Setting.h"
#include "DBManager.h"
#include "AutoFileManager.h"

#ifdef SM_DBG
#define USERMAGER_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "UserManager",__LINE__, ##arg)
#else
#define USERMAGER_PRT(fmt, arg...)
#endif


UserManager::UserManager(void)
: m_binit(false)
, m_userl(NULL)
, m_userl_size(0)
, m_user_num(0)
, tmp_ss((uint32)1)
{
	m_curr_sec = time(NULL);
	m_last_clear_dead_sec = m_curr_sec;
	m_stat_last_sec = m_curr_sec;
	m_src_last_sec = m_curr_sec;
	m_max_user_num = 0;
	m_max_pack_num = 0;
	for(int i=0;i<7;++i)
		m_nat_type_num[i] = 0;
}
UserManager::~UserManager(void)
{
}

int UserManager::init()
{
	if(m_binit)
		return -1;
	m_binit = true;
	DBManagerSngl::instance()->run();
	SourceServiceSngl::instance()->init();
	m_user_num   = 0;
	m_userl_size = PEER_MAX_SIZE;
	m_userl = new UserInfo[m_userl_size];
#ifdef SM_MODIFY
	m_packs.resize(10000);
#else
	m_packs.resize(2000);
#endif
	DEBUGMSG("#-UserManager::open \n");
	return 0;
}
void UserManager::fini()
{
	if(!m_binit)
		return;
	if(m_userl)
	{
		for(unsigned int i=0;i<m_userl_size;++i)
		{
			if(m_userl[i].flag)
			{
				logout(i);
			}
		}
		assert(0==m_user_num);
		delete[] m_userl;
		m_userl = NULL;
		m_userl_size = 0;
		m_user_num = 0;
	}
	Packet inf;
	while(m_packs.get_packet(inf))
		inf.block->free(2);
	m_packs.resize(0);
	SourceServiceSngl::instance()->fini();
	DBManagerSngl::instance()->end();
	SourceServiceSngl::destroy();
	DBManagerSngl::destroy();
	DEBUGMSG("#-UserManager::stop \n");
}


void UserManager::handle_root()//处理命令包线程
{
	//DEBUGMSG("+UserManager::handle_root() \n");
	static time_t curr_sec = 0;
	static int pack_count=0;
	static int max_pack_count=0;

	pack_count = m_packs.get_blocking_count();
	if(pack_count > m_max_pack_num) 
		m_max_pack_num = pack_count;
	if(pack_count>=max_pack_count)
		max_pack_count = pack_count;

	handle_command();

	curr_sec = time(NULL);
	if(curr_sec!=m_curr_sec)
	{
		m_curr_sec = curr_sec;
		handle_timer();

		if(max_pack_count >= 10)
			DEBUGMSG("...user_count=%d : pack_count=%d...\n",m_user_num,max_pack_count);
		max_pack_count = 0;
	}

}

void UserManager::handle_command()
{
	static Packet pack;
	static PTLStream ss;
	static PTL_Head  head;
	while(m_packs.get_packet(pack))
	{
		switch(pack.cmd)
		{
		case PACK_CMD_DATA:
			{
				ss.attach(pack.block->buf,pack.block->datalen,pack.block->datalen);
				if(0!=(ss>>head))
					break;
				switch(head.cmd)
				{
				case PTL_P2T_REQUEST_LOGIN:
					{
						if(0==ss>>request_login)
							on_ptl_request_login(head,request_login,pack);
					}
					break;
				case PTL_P2T_REPORT_NAT:
					{
						if(0==ss>>report_nat)
							on_ptl_report_nat(head,report_nat,pack);
					}
					break;
				case PTL_P2T_REPORT_DOWNLOADLIST_MAXNUM:
					{
						if(0==ss>>report_downloadlist_maxnum)
							on_ptl_report_downloadlist_maxnum(head,report_downloadlist_maxnum,pack);
					}
					break;
				case PTL_P2T_REPORT_SHARE_FILE:
					{
						if(0==ss>>report_sharefile)
							on_ptl_report_sharefile(head,report_sharefile,pack);
					}
					break;
				case PTL_P2T_REPORT_REMOVE_FILE:
					{
						if(0==ss>>report_removefile)
							on_ptl_report_removefile(head,report_removefile,pack);
					}
					break;
				case PTL_P2T_REPORT_START_DOWNLOAD_FILE:
					{
						if(0==ss>>report_startdownloadfile)
							on_ptl_report_startdownloadfile(head,report_startdownloadfile,pack);
					}
					break;
				case PTL_P2T_REPORT_STOP_DOWNLOAD_FILE:
					{
						if(0==ss>>report_stopdownloadfile)
							on_ptl_report_stopdownloadfile(head,report_stopdownloadfile,pack);
					}
					break;
				case PTL_P2T_REQUEST_FILE_SOURCE:
					{
						if(0==ss>>request_filesource)
							on_ptl_request_filesource(head,request_filesource,pack);
					}
					break;
				case PTL_P2T_REPORT_DOWNLOAD_WRONG:
					{
						if(0==ss>>report_downloadwrong)
							on_ptl_report_downloadwrong(head,report_downloadwrong,pack);
					}
					break;
				case PTL_P2T_REPORT_DOWNLOAD_FILE_SPEED:
					{
						if(0==ss>>report_downloadfilespeed)
							on_ptl_report_downloadfilespeed(head,report_downloadfilespeed,pack);
					}
					break;
				case PTL_P2T_REPORT_STAT:
					{
						if(0==ss>>report_stat)
							on_ptl_report_stat(head,report_stat,pack);
					}
					break;
				case PTL_P2T_REPORT_ERROR:
					{
						if(0==ss>>report_error)
							on_ptl_report_error(head,report_error,pack);
					}
					break;
				case PTL_P2T_REQUEST_CONN_TURN:
					{
						if(0==ss>>request_connturn)
							on_ptl_request_connturn(head,request_connturn,pack);
					}
					break;
				case PTL_P2T_REQUEST_KEEPLIVE:
					{
						//无实体数据
						on_ptl_request_keeplive(head,pack);
					}
					break;
				case PTL_P2T_REQUEST_SERVER_LIST:
					{
						if(0==ss>>request_serverlist)
							on_ptl_request_serverlist(head,request_serverlist,pack);
					}
					break;
				case PTL_P2T_REPORT_START_DOWNLOAD_LIST:
					{
						if(0==ss>>report_startdownloadlist)
							on_ptl_report_startdownloadlist(head,report_startdownloadlist,pack);
					}
					break;
				case PTL_P2T_REPORT_DOWNLOAD_FILE_INFO:
					{
						if(0==ss>>report_downloadfileinfo)
							on_ptl_report_downloadfileinfo(head,report_downloadfileinfo,pack);
					}
					break;
				default:
					{
						assert(0);
						DEBUGMSG("***unkown packet cmd = %d\n",head.cmd);
					}
					break;
				}
				assert(0==ss.length());
			}
			break;
		case PACK_CMD_TCP_DISCONNECT:
			{
				//DEBUGMSG("PACKET--PACK_CMD_TCP_DISCONNECT \n");
				if(m_userl[pack.sid].flag && m_userl[pack.sid].userType!=UT_CLIENT)
				{
					char buf[1024];
					sprintf(buf,"#-- server disconnect sid=%d: %s:%d -- %d sec",pack.sid,Util::ip_htoas(m_userl[pack.sid].tcpRealIP).c_str(),
						m_userl[pack.sid].tcpRealPort,(int)(m_curr_sec-m_userl[pack.sid].lastActiveTime));
					LOG_disconnect(buf);
				}
				USERMAGER_PRT("receive logout command!!!!!\n");
				logout(pack.sid);
			}
			break;
		default:
			assert(0);
			break;
		}
		if(pack.block)
			pack.block->free(2);
	}
}

int UserManager::get_nat_type(int arr[])
{
	for(int i=0;i<7;++i)
	{
		arr[i] = m_nat_type_num[i];
	}
	return 0;
}
int UserManager::get_server_list(UserInfo *arr[],int max_count)
{
	int n=0;
	if(max_count<=0)
		return 0;
	for(unsigned int i=0;i<m_userl_size;++i)
	{
		if(m_userl[i].flag)
		{
			if(UT_SERVER==m_userl[i].userType || UT_CENTER==m_userl[i].userType || UT_SUPER==m_userl[i].userType)
			{
				arr[n++] = &m_userl[i];
				if(n>=max_count)
					break;
			}
		}
	}
	return n;
}
void UserManager::handle_timer()
{
	if(m_last_clear_dead_sec+125 < m_curr_sec)
	{
		m_last_clear_dead_sec = m_curr_sec;
		for(int i=0;i<7;++i)
			m_nat_type_num[i] = 0;
		for(unsigned int i=0;i<m_userl_size;++i)
		{
			if(m_userl[i].flag)
				m_nat_type_num[(int)m_userl[i].natType]++;
			if(m_userl[i].flag && m_userl[i].lastActiveTime + 180 < m_curr_sec)
			{
				char buf[1024];
				sprintf(buf,"#--- close dead User : %s:%d -- %d sec",Util::ip_htoas(m_userl[i].tcpRealIP).c_str(),
					m_userl[i].tcpRealPort,(int)(m_curr_sec-m_userl[i].lastActiveTime));
				LOG_disconnect(buf);
				USERMAGER_PRT("long time no active, tracker will take you logout!!!!\n");
				logout(i,true);
			}
			//add 20150128:
			if(m_userl[i].flag && UT_CLIENT == m_userl[i].userType)
			{
#ifdef SM_VOD
				if((m_userl[i].sourceMap.empty()) && (m_userl[i].sourceMapvod.empty()))++m_userl[i].source_empty_count;
				else m_userl[i].source_empty_count = 0;
#else
				if(m_userl[i].sourceMap.empty()) ++m_userl[i].source_empty_count;
				else m_userl[i].source_empty_count = 0;
#endif
				if(m_userl[i].source_empty_count > 3)
				{
					//连续4次空计数: 4*125秒=500秒 = 8分钟左右，继线
					char buf[1024];
					sprintf(buf,"#--- close source_empty User : %s:%d -- %d sec",Util::ip_htoas(m_userl[i].tcpRealIP).c_str(),
						m_userl[i].tcpRealPort,(int)(m_curr_sec-m_userl[i].lastActiveTime));
					LOG_disconnect(buf);
					USERMAGER_PRT("empty source will take you logout!!!!\n");
					logout(i,true);
				}
			}
		}
	}
	////test;
	//if(m_stat_last_sec + 30 < m_curr_sec)
	if(m_stat_last_sec + 600 < m_curr_sec)
	{
		m_stat_last_sec = m_curr_sec;
		StatManagerSngl::instance()->handle_timeout();
	}
	if(m_src_last_sec + 30 < m_curr_sec)
	{
		m_src_last_sec = m_curr_sec;
		DEBUGMSG("-------- user count = %d ---\n",m_user_num);
		SourceServiceSngl::instance()->handle_timeout();
	}
}
//*****************************
//*
//*@function:用户登陆命令处理
//*@[create,review]:[hechunlong(2008-4),*-*|*-*]
//*@[modify,review]:[hechunlong(2008-6-23),*-*|*-*]
//*@remark: 
//* 2008-6-23修改增加对用户标识号的分配。
//*
//*****************************
void UserManager::on_ptl_request_login                 (const PTL_Head& head,PTL_P2T_RequestLogin& req,Packet& inf)
{
	DEBUGMSG("->PACKET--request_login \n");
	sid = inf.sid;
	assert(!m_userl[sid].flag);
	if(m_userl[sid].flag)
	{
		assert(inf.ip == m_userl[sid].tcpRealIP && inf.port == m_userl[sid].tcpRealPort);
		return;//暂不处理
	}
	tmp_block = MemBlock::allot(1024,2);
	if(!tmp_block)
		return;//开配内存失败几乎不可能
	
	m_user_num++;
	if(m_user_num > m_max_user_num)
		m_max_user_num = m_user_num;
	m_userl[sid].reset();
	m_userl[sid].flag          = 1;
	m_userl[sid].beginTime     = m_curr_sec;
	m_userl[sid].lastActiveTime= m_userl[sid].beginTime;
	m_userl[sid].natType       = 6;
	m_userl[sid].userType      = req.utype;
	m_userl[sid].sessionID     = sid;
	m_userl[sid].ip            = inf.ip;
	m_userl[sid].menu          = req.menu;
	m_userl[sid].bshare        = req.menu & 0x1;
	m_userl[sid].tcpLocalIP    = 0;
	m_userl[sid].tcpLocalPort  = 0;
	m_userl[sid].tcpRealIP     = 0;
	m_userl[sid].tcpRealPort   = 0;
	m_userl[sid].version       = req.ver;
	m_userl[sid].sysversion    = req.sysver;
	if(UT_SUPER==m_userl[sid].userType)
		AutoFileManagerSngl::instance()->add_super(&m_userl[sid]);
	memcpy(m_userl[sid].mac,req.mac,sizeof(req.mac));
	if(0==strlen(req.uid))
	{
		SettingSngl::instance()->get_new_uid(m_userl[sid].uid,req.mac);
		//DEBUGMSG("new user uid = %s \n",m_userl[sid].uid);
	}
	else
	{
		memcpy(m_userl[sid].uid,req.uid,PUIDLEN);
	}
	//DBlog
	DBUserLogin *db_inf = new DBUserLogin();
	if(db_inf)
	{
		memcpy(db_inf->uid,m_userl[sid].uid,PUIDLEN);
		db_inf->peer_type = m_userl[sid].userType;
		db_inf->ip = m_userl[sid].ip;
		db_inf->prog_ver = req.ver;
		db_inf->os_ver = req.sysver;
		tmp_pack.cmd = DBC_USER_LOGIN;
		tmp_pack.data = (void*)db_inf;
		if(!DBManagerSngl::instance()->put_packet(tmp_pack))
			delete db_inf;
	}
	StatManagerSngl::instance()->on_login_in(m_userl[sid].userType);
	DEBUGMSG("---user login: uid=%s, sessionID=%d ,userType=%d\n",m_userl[sid].uid,m_userl[sid].sessionID,m_userl[sid].userType);

	response_login.result       = 0;
	response_login.sessionID    = m_userl[sid].sessionID;
	response_login.eyeIP        = m_userl[sid].ip;
	memcpy(response_login.uid,m_userl[sid].uid,PUIDLEN);
	response_login.trackVer     = TRACKER_VERSION;
	response_login.trackID      = SettingSngl::instance()->get_tracker_id();

	//response_login
	cmd_head.cmd = PTL_P2T_RESPONSE_LOGIN;
	tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
	tmp_ss << cmd_head;
	tmp_ss << response_login;
	tmp_ss.fitsize32(4);
	tmp_block->datalen = tmp_ss.length();
	
	tmp_pack.block = tmp_block;
	tmp_pack.cmd = PACK_CMD_DATA;
	tmp_pack.sid = sid;
	if(!PeerManagerSngl::instance()->put_packet(tmp_pack))
		tmp_block->free(2);
}
void UserManager::on_ptl_report_nat                    (const PTL_Head& head,PTL_P2T_ReportNat& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_nat \n");
	sid = inf.sid;
	if(!m_userl[sid].flag)
	{
		DEBUGMSG("***old packet***\n");
		return;
	}
	if(req.ntype>=0&&req.ntype<6)
		m_userl[sid].natType       = req.ntype;
	m_userl[sid].tcpLocalIP    = req.tcpLocalIP;
	m_userl[sid].tcpLocalPort  = req.tcpLocalPort;
	m_userl[sid].tcpRealIP     = req.tcpRealIP;
	m_userl[sid].tcpRealPort   = req.tcpRealPort;
	m_userl[sid].udpLocalIP    = req.udpLocalIP;
	m_userl[sid].udpLocalPort  = req.udpLocalPort;
	m_userl[sid].udpRealIP     = req.udpRealIP;
	m_userl[sid].udpRealPort   = req.udpRealPort;
}

void UserManager::on_ptl_report_downloadlist_maxnum    (const PTL_Head& head,PTL_P2T_ReportDownloadListMaxnum& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_downloadlist_maxnum = %d \n",req.downloadlist_maxnum);
	sid = inf.sid;
	if(!m_userl[sid].flag)
	{
		DEBUGMSG("***old packet***\n");
		return;
	}
	m_userl[sid].downloadlist_maxnum = req.downloadlist_maxnum;
}

void UserManager::on_ptl_report_sharefile              (const PTL_Head& head,PTL_P2T_ReportShareFile& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_sharefile \n");
	sid = inf.sid;
	if(!m_userl[sid].flag)
	{
		DEBUGMSG("***old packet***\n");
		return;
	}
	for(unsigned int i=0;i<req.num;++i)
	{
		tmp_hash.set_buffer((uchar*)req.files[i].fhash);
#ifdef SM_VOD
		char buftmp[45];
		tmp_hash.to_string(buftmp,45);
		USERMAGER_PRT("receive share file type=%d name=%s\n", req.filetype, buftmp);
		SourceServiceSngl::instance()->source_add(tmp_hash,req.filetype,req.files[i].fsize,0,&m_userl[sid]);
#else
		SourceServiceSngl::instance()->source_add(tmp_hash,req.files[i].fsize,0,&m_userl[sid]);
#endif /* end of SM_VOD */
	}
}
void UserManager::on_ptl_report_removefile             (const PTL_Head& head,PTL_P2T_ReportRemoveFile& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_removefile \n");
	sid = inf.sid;
	if(!m_userl[sid].flag)
	{
		DEBUGMSG("***old packet***\n");
		return;
	}
	for(unsigned int i=0;i<req.num;++i)
	{
		tmp_hash.set_buffer((uchar*)req.files[i].fhash);
#ifdef SM_VOD
		if(PLAYTYPE_VOD!=req.filetype) {
			SourceServiceSngl::instance()->source_delete(tmp_hash,&m_userl[sid]);
		} else {
			SourceServiceSngl::instance()->source_delete_vod(tmp_hash,&m_userl[sid]);
		}
#else
		SourceServiceSngl::instance()->source_delete(tmp_hash,&m_userl[sid]);
#endif /* end of SM_VOD */
	}
}
void UserManager::on_ptl_report_startdownloadfile     (const PTL_Head& head,PTL_P2T_ReportStartDownloadFile& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_startdownload_file \n");
	if(m_userl[inf.sid].flag)
	{
		m_userl[inf.sid].totalRequestNum++;
#ifdef SM_VOD
		StatManagerSngl::instance()->on_req_file(req.fhash,&m_userl[inf.sid],req.filetype);
#else
		StatManagerSngl::instance()->on_req_file(req.fhash,&m_userl[inf.sid]);
#endif /* end of SM_VOD */
	}
}
void UserManager::on_ptl_report_startdownloadlist    (const PTL_Head& head,PTL_P2T_ReportStartDownloadList& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_startdownloadlist \n");
	if(m_userl[inf.sid].flag)
	{	
#ifdef SM_VOD
		StatManagerSngl::instance()->on_req_file(req.fhash,&m_userl[inf.sid],req.filetype);
#else
		StatManagerSngl::instance()->on_req_file(req.fhash,&m_userl[inf.sid]);
#endif /* end of SM_VOD */
		m_userl[inf.sid].totalRequestNum++;
		if(UT_CLIENT == m_userl[inf.sid].userType && (m_userl[inf.sid].menu & MENU_VIP))
		{
			AutoFileManagerSngl::instance()->on_vipfile_start(req);
		}
	}
}
void UserManager::on_ptl_report_stopdownloadfile      (const PTL_Head& head,PTL_P2T_ReportStopDownloadFile& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_stopdownload_file \n");
	////todo:
	//if(m_userl[inf.sid].flag)
	//{
	//	m_userl[inf.sid].totalRequestNum++;
	//	if(UT_CLIENT == m_userl[inf.sid].userType && MENU_VIP==m_userl[inf.sid].menu)
	//	{
	//		//
	//	}
	//}
}
void UserManager::on_ptl_request_filesource            (const PTL_Head& head,PTL_P2T_RequestFileSource& req,Packet& inf)
{
	DEBUGMSG("->PACKET--request_filesource \n");
	int n=0;
	sid=inf.sid;
	response_filesource.result    = (uint32)-1;
	response_filesource.fsize  = 0;
	memcpy(response_filesource.fhash,req.fhash,HASHLEN);
	response_filesource.urlflag = 1;
	uint32 block_size = 0;
	tmp_hash.set_buffer((uchar*)req.fhash);
	
	if(m_userl[sid].flag)
	{
#ifdef SM_VOD
		char buftmp[45];
		tmp_hash.to_string(buftmp,45);
		USERMAGER_PRT("user %s requet filesource playtype=%d name=%s\n", m_userl[sid].uid,req.filetype, buftmp);
		n=SourceServiceSngl::instance()->source_find(tmp_hash,req.filetype,&m_userl[sid],tmp_user
			,response_filesource.fsize,block_size,req.maxnum>60?60:req.maxnum);
#else
		n=SourceServiceSngl::instance()->source_find(tmp_hash,&m_userl[sid],tmp_user
			,response_filesource.fsize,block_size,req.maxnum>60?60:req.maxnum);
#endif
		if(n>=0) 
		{
			response_filesource.result = 0;
		}
		map<hash_t,uint32>::iterator it = m_userl[sid].sourceMap.find(tmp_hash);
		if(it!=m_userl[sid].sourceMap.end())
			response_filesource.urlflag = it->second;
	}
	
	int j=0;
	int trackID = SettingSngl::instance()->get_tracker_id();
	for(int i=0;i<n;++i)
	{
		response_filesource.peers[j].ntype          = tmp_user[i]->natType;
		response_filesource.peers[j].utype          = tmp_user[i]->userType;
		response_filesource.peers[j].sessionID      = tmp_user[i]->sessionID;
		response_filesource.peers[j].menu           = tmp_user[i]->menu;
		response_filesource.peers[j].trackID        = trackID;

		response_filesource.peers[j].tcpLocalIP     = tmp_user[i]->tcpLocalIP;
		response_filesource.peers[j].tcpLocalPort   = tmp_user[i]->tcpLocalPort;
		response_filesource.peers[j].tcpRealIP      = tmp_user[i]->tcpRealIP;
		response_filesource.peers[j].tcpRealPort    = tmp_user[i]->tcpRealPort;

		response_filesource.peers[j].udpLocalIP     = tmp_user[i]->udpLocalIP;
		response_filesource.peers[j].udpLocalPort   = tmp_user[i]->udpLocalPort;
		response_filesource.peers[j].udpRealIP      = tmp_user[i]->udpRealIP;
		response_filesource.peers[j].udpRealPort    = tmp_user[i]->udpRealPort;
		j++;
		if(j>=20)
		{
			response_filesource.num = j;
			tmp_block = MemBlock::allot(1024,2);
			if(tmp_block)
			{
				//response_filesource
				cmd_head.cmd = PTL_P2T_RESPONSE_FILE_SOURCE;
				tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
				tmp_ss << cmd_head;
				tmp_ss << response_filesource;
				tmp_ss.fitsize32(4);
				tmp_block->datalen = tmp_ss.length();
				
				tmp_pack.block = tmp_block;
				tmp_pack.cmd = PACK_CMD_DATA;
				tmp_pack.sid = sid;
				PeerManagerSngl::instance()->put_packet(tmp_pack);
			}
			j=0;
		}
	}

	//没有可用源，即使有小都返回
	if(j>0||n<=0)
	{
		response_filesource.num = j;
		tmp_block = MemBlock::allot(1024,2);
		if(tmp_block)
		{
			//response_filesource
			cmd_head.cmd = PTL_P2T_RESPONSE_FILE_SOURCE;
			tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
			tmp_ss << cmd_head;
			tmp_ss << response_filesource;
			tmp_ss.fitsize32(4);
			tmp_block->datalen = tmp_ss.length();
			
			tmp_pack.block = tmp_block;
			tmp_pack.cmd = PACK_CMD_DATA;
			tmp_pack.sid = sid;
			PeerManagerSngl::instance()->put_packet(tmp_pack);
		}
	}
}
void UserManager::on_ptl_report_downloadwrong          (const PTL_Head& head,PTL_P2T_ReportDownloadWrong& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_downloadwrong \n");
	StatManagerSngl::instance()->on_download_wrong(req,m_userl[inf.sid]);
}
void UserManager::on_ptl_report_downloadfilespeed      (const PTL_Head& head,PTL_P2T_ReportDownloadFileSpeed& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_downloadfilespeed \n");
	//if(m_userl[inf.sid].flag)
	//{
	//	DBSpeedInfo *db_inf = new DBSpeedInfo();
	//	if(db_inf)
	//	{
	//		memcpy(db_inf->uid,m_userl[inf.sid].uid,PUIDLEN);
	//		db_inf->ip          = m_userl[inf.sid].ip;
	//		db_inf->size        = req.size;
	//		hash_t hash;
	//		hash.set_buffer((uchar*)req.fhash);
	//		hash.to_string(db_inf->str_hash,48);
	//		db_inf->hash_type = hash.hash_type();
	//		db_inf->download_duration = req.downSeconds;
	//		db_inf->wait_num    = req.cacheTimes;
	//		db_inf->speed       = req.speed;

	//		tmp_pack.cmd = DBC_SPEED_INFO;
	//		tmp_pack.data = (void*)db_inf;
	//		if(!DBManagerSngl::instance()->put_packet(tmp_pack))
	//			delete db_inf;
	//	}
	//}
	if(m_userl[inf.sid].flag)
	{
		DBDownloadFileInfo *db_inf = new DBDownloadFileInfo();
		if(db_inf)
		{
			memcpy(db_inf->uid,m_userl[inf.sid].uid,PUIDLEN);
			db_inf->ip          = m_userl[inf.sid].ip;
			memset(&db_inf->df,0,sizeof(db_inf->df));
			db_inf->df.flag = 0; //旧版数据用0表示
			memcpy(db_inf->df.fhash,req.fhash,HASHLEN);
			db_inf->df.size = req.size;
			db_inf->df.speed_KB = req.speed;
			db_inf->df.downSeconds = req.downSeconds;
			db_inf->df.cacheTimes = req.cacheTimes;
			db_inf->df.cacheSenconds = req.cacheSenconds;
			db_inf->df.dragTimes = req.dragTimes;
#ifdef SM_MODIFY
			db_inf->nattype = m_userl[inf.sid].natType;
#endif /* end of SM_MODIFY */

			tmp_pack.cmd = DBC_DOWNLOADFILE_INFO;
			tmp_pack.data = (void*)db_inf;
			if(!DBManagerSngl::instance()->put_packet(tmp_pack))
				delete db_inf;
		}
	}
}
void UserManager::on_ptl_report_stat                   (const PTL_Head& head,PTL_P2T_ReportStat& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_stat \n");
	if(m_userl[inf.sid].flag)
	{
		m_userl[inf.sid].totalDown_KB += req.downBytesPerIPT_KB[0]+req.downBytesPerIPT_KB[1] + req.downBytesPerIPT_KB[2];
		m_userl[inf.sid].totalUpload_KB += req.shareBytesPerIPT_KB[0]+req.shareBytesPerIPT_KB[1]+req.shareBytesPerIPT_KB[2];
		StatManagerSngl::instance()->on_req_stat(req);
	}
}
void UserManager::on_ptl_report_error                  (const PTL_Head& head,PTL_P2T_ReportError& req,Packet& inf)
{
	DEBUGMSG("->PACKET--report_error \n");
	//todo:
}
void UserManager::on_ptl_request_connturn              (const PTL_Head& head,PTL_P2T_RequestConnTurn& req,Packet& inf)
{
	DEBUGMSG("->PACKET--request_connturn \n");
	//目前不考虑不同trackID的情况
	if(req.desSessionID>=m_userl_size)
		return;
	sid = inf.sid;
	if(m_userl[sid].flag && m_userl[req.desSessionID].flag)
	{
		response_connturn.desPeerInfo.ntype        = m_userl[sid].natType;
		response_connturn.desPeerInfo.utype        = m_userl[sid].userType;
		response_connturn.desPeerInfo.sessionID    = m_userl[sid].sessionID;
		response_connturn.desPeerInfo.menu         = m_userl[sid].menu;
		response_connturn.desPeerInfo.trackID      = req.desTrackID;

		response_connturn.desPeerInfo.tcpLocalIP   = m_userl[sid].tcpLocalIP;
		response_connturn.desPeerInfo.tcpLocalPort = m_userl[sid].tcpLocalPort;
		response_connturn.desPeerInfo.tcpRealIP    = m_userl[sid].tcpRealIP;
		response_connturn.desPeerInfo.tcpRealPort  = m_userl[sid].tcpRealPort;

		response_connturn.desPeerInfo.udpLocalIP   = m_userl[sid].udpLocalIP;
		response_connturn.desPeerInfo.udpLocalPort = m_userl[sid].udpLocalPort;
		response_connturn.desPeerInfo.udpRealIP    = m_userl[sid].udpRealIP;
		response_connturn.desPeerInfo.udpRealPort  = m_userl[sid].udpRealPort;

		tmp_block = MemBlock::allot(1024,2);
		if(tmp_block)
		{
			//response_connturn
			cmd_head.cmd = PTL_P2T_RESPONSE_CONN_TURN;
			tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
			tmp_ss << cmd_head;
			tmp_ss << response_connturn;
			tmp_ss.fitsize32(4);
			tmp_block->datalen = tmp_ss.length();
			
			tmp_pack.block = tmp_block;
			tmp_pack.cmd = PACK_CMD_DATA;
			tmp_pack.sid = req.desSessionID;
			PeerManagerSngl::instance()->put_packet(tmp_pack);
		}
	}
}

void UserManager::on_ptl_request_keeplive              (const PTL_Head& head,/*PTL_P2T_RequestKeeplive& req,*/Packet& inf)
{
#ifdef SM_MODIFY
#else
	DEBUGMSG("->PACKET--request_keeplive \n");
#endif
	sid = inf.sid;
	if(m_userl[sid].flag)
	{
		m_userl[sid].lastActiveTime = m_curr_sec;
		tmp_block = MemBlock::allot(1024,2);
		if(tmp_block)
		{
			//request_keeplive
			cmd_head.cmd = PTL_P2T_RESPONSE_KEEPLIVE;
			tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
			tmp_ss << cmd_head;
			//tmp_ss << response_keeplive;
			tmp_ss.fitsize32(4);
			tmp_block->datalen = tmp_ss.length();
			
			tmp_pack.block = tmp_block;
			tmp_pack.cmd = PACK_CMD_DATA;
			tmp_pack.sid = sid;
			PeerManagerSngl::instance()->put_packet(tmp_pack);
		}
	}
}
void UserManager::on_ptl_request_serverlist            (const PTL_Head& head,PTL_P2T_RequestServerList& req,Packet& inf)
{
	DEBUGMSG("->PACKET--request_serverlist \n");
	//todo:
	//不考虑登陆
	int n = 0;
	sid = inf.sid;
	response_serverlist.trackID = SettingSngl::instance()->get_tracker_id();
	response_serverlist.beginTime = (uint32)StatManagerSngl::instance()->get_begin_time();
	response_serverlist.trackVer = TRACKER_VERSION;
	response_serverlist.userNum = m_user_num;
	response_serverlist.allNum = 0;
	response_serverlist.startNum = 0;
	response_serverlist.num = 0;
	n = get_server_list(tmp_user,1200);
	response_serverlist.allNum = n;
	int i=0,j=0;
	int start_num=0;
	for(i=0;i<n;++i)
	{
		memcpy(response_serverlist.servers[j].uid,tmp_user[i]->uid,PUIDLEN);
		response_serverlist.servers[j].ver        = tmp_user[i]->version ;
		response_serverlist.servers[j].sessionID  = tmp_user[i]->sessionID ;
		response_serverlist.servers[j].beginTime  = (uint32)tmp_user[i]->beginTime ;
		response_serverlist.servers[j].sourceNum  = tmp_user[i]->sourceMap.size() ;
		response_serverlist.servers[j].menu       = tmp_user[i]->menu ;
		response_serverlist.servers[j].utype      = tmp_user[i]->userType ;
		response_serverlist.servers[j].ntype      = tmp_user[i]->natType ;
		if(0==tmp_user[i]->tcpRealIP)
		{
			response_serverlist.servers[j].tcpRealIP  = tmp_user[i]->tcpLocalIP ;
			response_serverlist.servers[j].tcpRealPort  = tmp_user[i]->tcpLocalPort ;
		}
		else
		{
			response_serverlist.servers[j].tcpRealIP  = tmp_user[i]->tcpRealIP ;
			response_serverlist.servers[j].tcpRealPort  = tmp_user[i]->tcpRealPort ;
		}

		if(0==tmp_user[i]->udpRealIP)
		{
			response_serverlist.servers[j].udpRealIP  = tmp_user[i]->udpLocalIP ;
			response_serverlist.servers[j].udpRealPort  = tmp_user[i]->udpLocalPort ;
		}
		else
		{
			response_serverlist.servers[j].udpRealIP  = tmp_user[i]->udpRealIP ;
			response_serverlist.servers[j].udpRealPort  = tmp_user[i]->udpRealPort ;
		}
		j++;
		if(j>=10)
		{
			response_serverlist.num = j;
			response_serverlist.startNum = start_num;
			tmp_block = MemBlock::allot(1024,2);
			if(tmp_block)
			{
				//response_serverlist
				cmd_head.cmd = PTL_P2T_RESPONSE_SERVER_LIST;
				tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
				tmp_ss << cmd_head;
				tmp_ss << response_serverlist;
				tmp_ss.fitsize32(4);
				tmp_block->datalen = tmp_ss.length();
				
				tmp_pack.block = tmp_block;
				tmp_pack.cmd = PACK_CMD_DATA;
				tmp_pack.sid = sid;
				PeerManagerSngl::instance()->put_packet(tmp_pack);
			}
			j=0;
			start_num = i+1;
		}
	}
	//没有可用源，即使有小都返回
	if(j>0||n<=0)
	{
		response_serverlist.num = j;
		response_serverlist.startNum = start_num;
		tmp_block = MemBlock::allot(1024,2);
		if(tmp_block)
		{
			//response_serverlist
			cmd_head.cmd = PTL_P2T_RESPONSE_SERVER_LIST;
			tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
			tmp_ss << cmd_head;
			tmp_ss << response_serverlist;
			tmp_ss.fitsize32(4);
			tmp_block->datalen = tmp_ss.length();
			
			tmp_pack.block = tmp_block;
			tmp_pack.cmd = PACK_CMD_DATA;
			tmp_pack.sid = sid;
			PeerManagerSngl::instance()->put_packet(tmp_pack);
		}
	}
}
void UserManager::on_ptl_report_downloadfileinfo       (const PTL_Head& head,PTL_P2T_ReportDownloadFileInfo& req,Packet& inf)
{
	DEBUGMSG("->PACKET--on_ptl_report_downloadfileinfo \n");
	if(m_userl[inf.sid].flag)
	{
		DBDownloadFileInfo *db_inf = new DBDownloadFileInfo();
		if(db_inf)
		{
			memcpy(db_inf->uid,m_userl[inf.sid].uid,PUIDLEN);
			db_inf->ip          = m_userl[inf.sid].ip;
			memcpy(&db_inf->df,&req,sizeof(req));
#ifdef SM_MODIFY
			db_inf->nattype = m_userl[inf.sid].natType;
#endif /* end of SM_MODIFY */

			tmp_pack.cmd = DBC_DOWNLOADFILE_INFO;
			tmp_pack.data = (void*)db_inf;
			if(!DBManagerSngl::instance()->put_packet(tmp_pack))
				delete db_inf;
		}
	}
}
void UserManager::logout(int sid,bool tell/*=false*/)
{
	if(m_userl[sid].flag)
	{
		DEBUGMSG("UserManager::logout \n");
		if(tell)
		{
			tmp_pack.block = NULL;
			tmp_pack.cmd = PACK_CMD_TCP_DISCONNECT;
			tmp_pack.sid = sid;
			PeerManagerSngl::instance()->put_packet(tmp_pack);
		}
		//DBlog
		DBUserLoginOut *db_inf = new DBUserLoginOut();
		if(db_inf)
		{
			memcpy(db_inf->uid,m_userl[sid].uid,PUIDLEN);
			db_inf->file_num = m_userl[sid].totalRequestNum;
			db_inf->dump_num = m_userl[sid].dump;
			db_inf->download_flow = m_userl[sid].totalDown_KB;
			db_inf->upload_flow = m_userl[sid].totalUpload_KB;
			tmp_pack.cmd = DBC_USER_LOGIN_OUT;
			tmp_pack.data = (void*)db_inf;
			if(!DBManagerSngl::instance()->put_packet(tmp_pack))
				delete db_inf;
		}
#ifdef SM_VOD
		SourceServiceSngl::instance()->source_delete_vod(&m_userl[sid]);
		SourceServiceSngl::instance()->source_delete(&m_userl[sid]);
#else
		SourceServiceSngl::instance()->source_delete(&m_userl[sid]);
#endif /* end of SM_VOD */
		StatManagerSngl::instance()->on_login_out(m_userl[sid].userType);

		if(UT_SUPER==m_userl[sid].userType)
			AutoFileManagerSngl::instance()->del_super(&m_userl[sid]);
		m_userl[sid].reset();
		m_user_num--;
	}
}
int UserManager::ptl_request_startdownloadlist(UserInfo* user,const hash_t& hash,const char* url)
{
	assert(user->flag);
	tmp_block = MemBlock::allot(1024,2);
	if(!tmp_block)
		return -1;
	memcpy(request_startdownloadlist.fhash,hash.buffer(),HASHLEN);
	strcpy(request_startdownloadlist.url,url);

	cmd_head.cmd = PTL_P2T_REQUEST_START_DOWNLOAD_LIST;
	tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
	tmp_ss << cmd_head;
	tmp_ss << request_startdownloadlist;
	tmp_ss.fitsize32(4);
	tmp_block->datalen = tmp_ss.length();
	
	tmp_pack.block = tmp_block;
	tmp_pack.cmd = PACK_CMD_DATA;
	tmp_pack.sid = user->sessionID;
	PeerManagerSngl::instance()->put_packet(tmp_pack);
	//同时直接共享源，以保证VIP立即找到
#ifdef SM_VOD
	//SourceServiceSngl::instance()->source_add(hash,PLAYTYPE_LIVE,0,0,user);
#else
	SourceServiceSngl::instance()->source_add(hash,0,0,user);
#endif
	return 0;
}
int UserManager::ptl_request_stopdownloadlist(UserInfo* user,const hash_t& hash)
{
	assert(user->flag);
	tmp_block = MemBlock::allot(1024,2);
	if(!tmp_block)
		return -1;
	memcpy(request_stopdownloadlist.fhash,hash.buffer(),HASHLEN);

	cmd_head.cmd = PTL_P2T_REQUEST_STOP_DOWNLOAD_LIST;
	tmp_ss.attach(tmp_block->buf,tmp_block->buflen);
	tmp_ss << cmd_head;
	tmp_ss << request_stopdownloadlist;
	tmp_ss.fitsize32(4);
	tmp_block->datalen = tmp_ss.length();
	
	tmp_pack.block = tmp_block;
	tmp_pack.cmd = PACK_CMD_DATA;
	tmp_pack.sid = user->sessionID;
	PeerManagerSngl::instance()->put_packet(tmp_pack);
	return 0;
}
int UserManager::get_msg_server_info(MsgServerInfo& msg)
{
	MsgServerNode* node = NULL;
	int n = get_server_list(tmp_user,1200);
	for(int i=0;i<n;++i)
	{
		node = new MsgServerNode();
		memcpy(node->uid,tmp_user[i]->uid,PUIDLEN);
		node->ver        = tmp_user[i]->version ;
		node->sessionID  = tmp_user[i]->sessionID ;
		node->beginTime  = (uint32)tmp_user[i]->beginTime ;
		node->sourceNum  = tmp_user[i]->sourceMap.size() ;
		node->menu       = tmp_user[i]->menu ;
		node->utype      = tmp_user[i]->userType ;
		node->ntype      = tmp_user[i]->natType ;
		if(0==tmp_user[i]->tcpRealIP)
		{
			node->tcpRealIP  = tmp_user[i]->tcpLocalIP ;
			node->tcpRealPort  = tmp_user[i]->tcpLocalPort ;
		}
		else
		{
			node->tcpRealIP  = tmp_user[i]->tcpRealIP ;
			node->tcpRealPort  = tmp_user[i]->tcpRealPort ;
		}

		if(0==tmp_user[i]->udpRealIP)
		{
			node->udpRealIP  = tmp_user[i]->udpLocalIP ;
			node->udpRealPort  = tmp_user[i]->udpLocalPort ;
		}
		else
		{
			node->udpRealIP  = tmp_user[i]->udpRealIP ;
			node->udpRealPort  = tmp_user[i]->udpRealPort ;
		}
		msg.svrs.push_back(node);
	}
	return 0;
}
