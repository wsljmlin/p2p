#include "SourceService.h"
#include "StatManager.h"
#include "Setting.h"
#include "AutoFileManager.h"

#ifdef SM_DBG
#define SOURCESERVICE_PRT(fmt, arg...) fprintf(stdout, "[%s(%s):%d] "fmt, __FUNCTION__, "SourceService", __LINE__, ##arg)
#else
#define SOURCESERVICE_PRT(fmt, arg...) 
#endif


//一维下标:自身的nat类型，二维下标:对方nat类型 
//7表未知类型,自己是6的,当自己是3类型，即只返回0，1，2，3
static char g_ConnectionType[7][7] = 
{
	{1,1,1,1,1,1,0},
	{1,1,1,1,1,0,0},
	{1,1,1,1,1,0,0},
	{1,1,1,1,0,0,0},
	{1,1,1,0,0,0,0},
	{1,0,0,0,0,0,0},
	{1,1,1,1,0,0,0}
};

//**************************************************************************************
void UserNodeList::clear()
{
	rlist_head_t *pos,*n;
	unsigned int count=0;

	rlist_for_each_safe(pos,n,&m_head)
	{
		delete rlist_entry(pos,UserNode,rlist);
		count++;
	}
	assert(count == m_size);
	rlist_init(&m_head);
	m_all_num -= m_size;
	m_size = 0;
	assert(0 == m_all_num);//保证所有取出去的块已经释放回来
}
void UserNodeList::reserve(unsigned int n)
{
	if(n>m_size)
	{
		int count = n-m_size;
		for(int i=0;i<count;++i)
		{
			UserNode *node = new UserNode();
			if(node) 
			{
				m_all_num++;
				rlist_add_tail(&node->rlist,&m_head);
				m_size++;
			}
		}
	}
	else
	{
		int count = m_size-n;
		rlist_head_t *ptr;
		for(int i=0;i<count;++i)
		{
			ptr = m_head.next;
			rlist_del(ptr);
			delete rlist_entry(ptr,UserNode,rlist);
			m_size--;
			m_all_num--;
		}
	}
}
UserNode *UserNodeList::get_node()
{
	if(rlist_empty(&m_head))
	{
		UserNode *node = new UserNode();
		if(node) m_all_num++;
		return node;
	}
	else
	{
		rlist_head_t *ptr = m_head.next;
		rlist_del(ptr);
		m_size--;
		return rlist_entry(ptr,UserNode,rlist);
	}
}
void UserNodeList::put_node(UserNode *node)
{
	if(!node)
		return;
	rlist_add_tail(&node->rlist,&m_head);
	m_size++;
}

//**************************************************************************************
SourceService::SourceService(void)
: m_binit(false)
{
	m_rsp_server_num = 0;
	m_rsp_center_num = 0;
	m_rsp_super_num = 0;
	m_vipfile_num = 0;
}

SourceService::~SourceService(void)
{
	assert(0==m_vipfile_num);
}
int SourceService::init()
{
	if(m_binit)
		return -1;
	m_binit = true;
	m_rsp_server_num = SettingSngl::instance()->get_rsp_server_num();
	m_rsp_center_num = SettingSngl::instance()->get_rsp_center_num();
	m_rsp_super_num = SettingSngl::instance()->get_rsp_super_num();
	m_free_peernodel.reserve(100000);//默认10W文件源次,只供服务器类型源使用
	DEBUGMSG("#-SourceService::init \n");
	return 0;
}
void SourceService::fini()
{
	if(!m_binit)
		return;
	m_binit = false;
	assert(m_sourcel.empty());
#ifdef SM_VOD
	source_clear_ex();
#else
	source_clear();
#endif /* end of SM_VOD */
	m_free_peernodel.clear();
	DEBUGMSG("#-SourceService::fini \n");
}
void SourceService::handle_timeout()
{
	//SourceIter it;
	//int f_count=0,fs_count=0;
	//f_count = m_sourcel.size();
	//for(it=m_sourcel.begin();it!=m_sourcel.end();++it)
	//{
	//	fs_count += it->second->user_num;
	//	fs_count += it->second->server_num;
	//	fs_count += it->second->center_num;
	//	fs_count += it->second->super_num;
	//}
	//DEBUGMSG("--------file count=%d,   file source count = %d ---\n",f_count,fs_count);
}

#ifdef SM_VOD
void SourceService::print_sources() {
	/* print vod information */
	SOURCESERVICE_PRT("+++++++++++++++++VOD++++++++++++++\n");
	for(SourceItervod itvod = m_sourcevod.begin(); itvod != m_sourcevod.end(); itvod++) {
		char bufurldl[45];
		hash_t tthdl = itvod->first;
		tthdl.to_string(bufurldl,45); 
		SOURCESERVICE_PRT("urldl:%s filenum=%d\n", bufurldl, itvod->second->size());
		//SourceMap *map = itvod->second;
		/*
		for(SourceIter itsource = map->begin(); itsource != map->end(); itsource++) {
			char bufurl2[45];
			hash_t tthpl = itsource->first;
			tthpl.to_string(bufurl2,45); 
			SOURCESERVICE_PRT("url2:%s\n", bufurl2);
		}*/
	}

	/* print live information */
	SOURCESERVICE_PRT("+++++++++++++++++LIVE++++++++++++++\n");
	for(SourceIter itlive = m_sourcel.begin(); itlive != m_sourcel.end(); itlive++) {
		char bufurldl[45];
		hash_t tthdl = itlive->first;
		tthdl.to_string(bufurldl,45); 
		SOURCESERVICE_PRT("urldl:%s\n", bufurldl);
	}
}

void SourceService::print_user_source(UserInfo *user) {
	//user->sourceMapvod_mux.lock();
	map<hash_t,map<hash_t,uint32> >::iterator itdl=user->sourceMapvod.begin();
	SOURCESERVICE_PRT("playlist:%d\n", user->downloadlist_num);
	SOURCESERVICE_PRT("user vod %s:%d information:\n", user->uid,user->sessionID);	

	/*print vod information */
	for(itdl=user->sourceMapvod.begin(); itdl != user->sourceMapvod.end(); itdl++) {
		char bufurldl[45];
		hash_t tthdl = itdl->first;
		tthdl.to_string(bufurldl,45); 
		SOURCESERVICE_PRT("urldl:%s filenum=%d\n", bufurldl, itdl->second.size());
		/*map<hash_t,uint32> ::iterator itmap;
		for(itmap = itdl->second.begin(); itmap != itdl->second.end(); itmap++) {
			char bufurl2[45];
			hash_t tthpl = itmap->first;
			tthpl.to_string(bufurl2,45); 
			SOURCESERVICE_PRT("url2:%s\n", bufurl2);
		}*/
	}
	//user->sourceMapvod_mux.unlock();
	
	/*print live information */
	map<hash_t,uint32>::iterator itdllive=user->sourceMap.begin();
	SOURCESERVICE_PRT("user live %s:%d information:\n", user->uid,user->sessionID);
	for(itdllive=user->sourceMap.begin(); itdllive != user->sourceMap.end(); itdllive++) {
		char bufurldl[45];
		hash_t tthdl = itdllive->first;
		tthdl.to_string(bufurldl,45); 
		SOURCESERVICE_PRT("urldl:%s\n", bufurldl);
	}
	


}

void SourceService::source_clear_ex()
{
	//此接口由最后程序退出清理用,并不对user内信息同步
	SourceIter itlive;
	SourceItervod itvod;
	rlist_head_t *pos, *n;
	
	/* clear live nodes */
	for(itlive=m_sourcel.begin();itlive!=m_sourcel.end();++itlive) {
		rlist_for_each_safe(pos,n,&itlive->second->server_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));
		
		rlist_for_each_safe(pos,n,&itlive->second->center_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));
		
		rlist_for_each_safe(pos,n,&itlive->second->super_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));

		itlive->second->user_map.clear();
		delete itlive->second;
	}
	m_sourcel.clear();

	/* clear vod nodes */
	SourceIter itsource;
	//m_sourcevod_mux.lock();
	for(itvod=m_sourcevod.begin();itvod!=m_sourcevod.end();) {
		for(itsource=itvod->second->begin(); itsource!=itvod->second->end();) {
			rlist_for_each_safe(pos,n,&itsource->second->server_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));
			
		rlist_for_each_safe(pos,n,&itsource->second->center_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));
		
		rlist_for_each_safe(pos,n,&itsource->second->super_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));
		
			itsource->second->user_map.clear();
			delete itsource->second;
			itsource=itvod->second->begin();
		}
		itvod->second->clear();
		delete itvod->second;
		itvod=m_sourcevod.begin();
	}
	m_sourcevod.clear();
	//m_sourcevod_mux.unlock();
}

int SourceService::source_delete_vod(UserInfo* user)                   //删除user所有共享文件信息
{
	//在此不能用for循环
	hash_t tthlive;
	map<hash_t,uint32>::iterator itlive = user->sourceMap.begin();
	/* remove live source */
	while(itlive!=user->sourceMap.end())
	{
		tthlive = itlive->first;
		source_delete(tthlive,user);
		itlive = user->sourceMap.begin();
	}

	/* remove vod source */
	hash_t tthvod;
	//user->sourceMapvod_mux.lock();
	map<hash_t,map<hash_t,uint32> >::iterator itvod = user->sourceMapvod.begin();
	/* remove live source */
	while(itvod != user->sourceMapvod.end())
	{
		tthvod = itvod->first;
		urldlsource_delete(tthvod,user);
		itvod = user->sourceMapvod.begin();
	}
	//user->sourceMapvod_mux.unlock();
	return 0;
}

int SourceService::source_delete_vod(const hash_t &tth,UserInfo* user)                   //删除user所有共享文件信息
{
	int ret = -1;
	if(HT_URLDL == tth.hash_type()) {
		ret = urldlsource_delete(tth, user);
	} else {
		ret = url2source_delete(tth, user);
	}
	return ret;
}

//删除tth文件的特定user信息
int SourceService::urldlsource_delete(const hash_t &tth,UserInfo* user) 
{
	/* check type */
	assert(HT_URLDL==tth.hash_type());
	if(HT_URLDL!=tth.hash_type()) 
		return -1;
	
	/* first find the urldl */
	map<hash_t,map<hash_t,uint32> >::iterator itdl=user->sourceMapvod.find(tth);
	if(itdl==user->sourceMapvod.end())
		return 0;

	/* deleter all files of user programe */
	map<hash_t,uint32>::iterator itp = itdl->second.begin();
	while (itp != itdl->second.end()) {
		url2source_delete(itp->first, user);
		itp = itdl->second.begin();
	}
	
	return 0;
}

int SourceService::url2source_delete(const hash_t &tthpl,  UserInfo* user) 
{
	/* check type */
	assert(HT_URL2=tthpl.hash_type());
	if(HT_URL2!=tthpl.hash_type()) 
		return -1;

	/* get urldl */
	hash_t tthdl;
	tthpl.url2hash_to_urldlhash(tthdl);

	/* 1. firtst find  search in user */
	map<hash_t,map<hash_t,uint32> >::iterator itdl=user->sourceMapvod.find(tthdl);
	if(itdl == user->sourceMapvod.end()) {	
		SOURCESERVICE_PRT("no such programe, cannot to delete\n");
		return -1;
	}
	
	map<hash_t,uint32>::iterator itpl=itdl->second.find(tthpl);
	if(itpl==user->sourceMapvod[tthdl].end())  {
		SOURCESERVICE_PRT("no such file in programe, cannot to delete\n");
		return -1;
	}

	/* 2. second search source info */
	SourceItervod sourceitvod = m_sourcevod.find(tthdl);
	SourceMap *mapsource;
	SourceInfo *inf;
	if(sourceitvod==m_sourcevod.end()) {
		return -1;
	}  

	mapsource = sourceitvod->second;
	SourceIter souceIt = mapsource->find(tthpl);
	if(souceIt==mapsource->end()) {
		return -1;
	} 
	inf = souceIt->second;
	
	int urlflag = itpl->second;
	if(urlflag)
		inf->urlflag_num--;
	bool bfind=false;
	
	if(UT_SERVER!=user->userType && UT_CENTER!=user->userType && UT_SUPER!=user->userType)
	{
		PeerIter it=inf->user_map.find(user->ip);
		PeerIter it_i = it;
		while(it_i!=inf->user_map.end())
		{
			if(it_i->second == user)
			{
				inf->user_map.erase(it_i);
				inf->user_num--;
				if(user->menu & MENU_VIP)
				{
					inf->vip_num--;
					if(0==inf->vip_num)
					{
						m_vipfile_num--;
						//AutoFileManagerSngl::instance()->on_vipfile_stop(tth);
					}
				}
				bfind = true;
				break;
			}
			if(it_i->first != user->ip)
				break;
			++it_i;
		}
		it_i = it;
		while(!bfind && it_i!=inf->user_map.begin() && it_i!=inf->user_map.end())
		{
			if(it_i->second == user)
			{
				inf->user_map.erase(it_i);
				inf->user_num--;
				if(user->menu & MENU_VIP)
				{
					inf->vip_num--;
					if(0==inf->vip_num)
					{
						m_vipfile_num--;
					}
				}
				bfind = true;
				break;
			}
			if(it_i->first != user->ip)
				break;
			--it_i;
		}
	}
	else
	{
		UserNode *node=0;
		rlist_head_t *head = NULL;
		if(UT_SERVER==user->userType)
			head = &inf->server_head;
		else if(UT_CENTER==user->userType)
			head = &inf->center_head;
		else
			head = &inf->super_head;
		
		rlist_head_t *ptr;
		rlist_for_each_prefetch(ptr,head)
		{
			node = rlist_entry(ptr,UserNode,rlist);
			if(node->user == user)
			{
				rlist_del(ptr);
				m_free_peernodel.put_node(node);
				if(UT_SERVER==user->userType)
					inf->server_num--;
				else if(UT_CENTER==user->userType)
					inf->center_num--;
				else
					inf->super_num--;
				bfind = true;
				break;
			}
		}
	}

	if(0==(inf->user_num+inf->server_num+inf->center_num+inf->super_num))
	{
		assert(rlist_empty(&inf->center_head) && rlist_empty(&inf->server_head) && rlist_empty(&inf->super_head) && 2==inf->user_map.size());
		assert(0==inf->vip_num);
		delete inf;
		m_sourcevod[tthdl]->erase(souceIt);

		if(!m_sourcevod[tthdl]->size()) {
			m_sourcevod.erase(sourceitvod);
			delete mapsource;
		}
	}

	/* 3. finally erase in user */
	user->sourceMapvod[tthdl].erase(itpl); 
	if(! user->sourceMapvod[tthdl].size()) {
		user->sourceMapvod.erase(itdl);
		//update
		//user->downloadlist_num--;
		user->downloadlist_num = user->sourceMapvod.size();
	}

	print_user_source(user);
	assert(bfind);
	return 0;
}

int SourceService::source_add(const hash_t &tth,int sourcetype, uint64 size,uint32 block_size,UserInfo *user)
{
	int ret = -1;

	/* this should convert from url2 to urldl */
	hash_t dlhash;
	tth.url2hash_to_urldlhash(dlhash);
#ifdef SM_DBG
	char bufurl2[45];
	tth.to_string(bufurl2,45); /*convert url2 to string */	
	char bufdl[45];
	dlhash.to_string(bufdl,45); /* get urldl string */
	SOURCESERVICE_PRT("add sourcehash=%s urdl=%s type=%d\n", bufurl2, bufdl, sourcetype);
#endif

	/* live will invoke old dealment */
	if(PLAYTYPE_VOD != sourcetype) {
		ret = source_add(tth,size,block_size,user);
		print_user_source(user);
		return ret;
	}

	/* vod will not receive playlist report */
	if(HT_URL2 != tth.hash_type()) {
		SOURCESERVICE_PRT("current playlist will not added!\n");
		return 0;
	}
	
	/* first search in user */
	map<hash_t,map<hash_t,uint32> >::iterator userItvod = user->sourceMapvod.find(dlhash);
	if(user->sourceMapvod.end() != userItvod) {
		/* the file of one programe have reported for current user */
		if(userItvod->second.end() != userItvod->second.find(tth)) {
			SOURCESERVICE_PRT("current file have reported for current user!\n");
			return -1;
		}
	}

	/* use lock to prevent */
	//m_sourcevod_mux.lock();
	/* second search source info */
	SourceItervod sourceitvod = m_sourcevod.find(dlhash);
	SourceMap *map;
	SourceInfo *inf;
	if(sourceitvod==m_sourcevod.end()) {
		/* new map for one programe */
		SOURCESERVICE_PRT("new add one programe and one file %s!\n", bufurl2);
		map = new SourceMap;
		inf = new SourceInfo();
		inf->size = size;
		inf->block_size = block_size;
		(*map)[tth] = inf;
		m_sourcevod[dlhash]  =map;
	} else {
		/* find file of one programe */
		map = sourceitvod->second;
		SourceIter souceIt = map->find(tth);
		if(souceIt==map->end()) {
			SOURCESERVICE_PRT("new add one file of programe!\n");
			inf = new SourceInfo();
			inf->size = size;
			inf->block_size = block_size;
			(*map)[tth] = inf;
		} else {
			SOURCESERVICE_PRT("file is exist, will add user in relative file!\n");
			inf = souceIt->second;
		}
		
	}
	//m_sourcevod_mux.unlock();


	//user->sourceMapvod_mux.lock();
	/* update information */
	if(UT_SERVER!=user->userType && UT_CENTER!=user->userType && UT_SUPER!=user->userType) {
		if(inf->user_map.insert(UserMap::value_type(user->ip,user))!=inf->user_map.end()) {
			SOURCESERVICE_PRT("add one user for file!\n");
			inf->user_num++;
			//添加到user自身的队列中
			int urlflag = 0;
			//少于500个源的时候，这些源都连接http源
			if(inf->urlflag_num<500)
				urlflag = 1; 
			else if(0==user->natType||1==user->natType)
				urlflag = 1;
			else if(inf->urlflag_num*2<inf->user_num)
				urlflag = 1;
			
			/* update in user */
			user->sourceMapvod[dlhash][tth] = urlflag;
			/* update downloadlist */
			user->downloadlist_num = user->sourceMapvod.size();
			if(urlflag)
				inf->urlflag_num++;
			if(user->menu & MENU_VIP) {
				inf->vip_num++;
				if(1==inf->vip_num)
					m_vipfile_num++;
			}
		}
	} else { 
		UserNode *node = m_free_peernodel.get_node();
		if(node) {
			node->user = user;
			if(UT_SERVER==user->userType) {
				rlist_add_tail(&node->rlist,&inf->server_head);
				inf->server_num++;
			}
			else if(UT_CENTER==user->userType) {
				rlist_add_tail(&node->rlist,&inf->center_head);
				inf->center_num++;
			}
			else {
				rlist_add_tail(&node->rlist,&inf->super_head);
				inf->super_num++;
			}
			user->sourceMapvod[dlhash][tth]=0;
		}
	}
	//user->sourceMapvod_mux.unlock();
	
	print_user_source(user);

	return 0;
}

int SourceService::source_find(const hash_t &tth,int sourcetype, UserInfo *user,UserInfo *arr[],uint64& size,uint32& block_size,int max_count/* = 100*/)
{
	int ret = -1;

	/* source type check */
	if(PLAYTYPE_VOD != sourcetype) {
		assert(HT_URLDL==tth.hash_type());
		ret = source_find(tth, user,arr,size,block_size,max_count);
		return ret;
	}
	/* find vod source */
	assert(HT_URL2==tth.hash_type());
	hash_t dlhash;
	tth.url2hash_to_urldlhash(dlhash);
#ifdef SM_DBG
	char bufurl2[45];
	tth.to_string(bufurl2,45); /*convert url2 to string */
	SOURCESERVICE_PRT("search source url2=%s\n", bufurl2);
#endif
	
	SourceItervod itdl;
	SourceIter itpl;
	//m_sourcevod_mux.lock();
	itdl=m_sourcevod.find(dlhash);
	if(itdl != m_sourcevod.end())
	{
		itpl=m_sourcevod[dlhash]->find(tth);
		if(itpl == m_sourcevod[dlhash]->end()) {
				SOURCESERVICE_PRT("cannot find source %s, the file have not reported\n", bufurl2);
				StatManagerSngl::instance()->on_nonfile_seach(tth);
				return -1;
		}
	} else {
		SOURCESERVICE_PRT("cannot find source %s, the programe have not created\n", bufurl2);
		StatManagerSngl::instance()->on_nonfile_seach(tth);
		return -1;
	}
	//m_sourcevod_mux.unlock();

	UserNode *node = 0;
	SourceInfo *inf = itpl->second;
	rlist_head_t *head;
	size = inf->size;
	block_size = inf->block_size;
	if(max_count<=0)
		return 0;

	//在此每搜到一个源,就将它移到最后,总是从前往后找,保持最均分配
	rlist_head_t *pos,*next;
	int n=0,m_u=0,m_s=0,m_c=0,m_super=0;
	int i=0;
	int num=0;
	//搜索super
	head = &inf->super_head;
	num = inf->super_num;
	for(i=0,pos = head->next;i<num && pos != head && n<max_count && m_super<m_rsp_super_num;++i)
	{
		node = rlist_entry(pos,UserNode,rlist);
		assert(node->user->flag);
		if(node->user->bshare && user!=node->user && (g_ConnectionType[(int)user->natType][(int)node->user->natType] || user->tcpRealIP==node->user->tcpRealIP))
		{
			m_super++;
			arr[n++] = node->user;
			//称到尾部
			next = pos->next;
			rlist_del(pos);
			rlist_add_tail(pos,head);
			pos = next;
		}
		else
		{
			pos = pos->next;
		}
	}
	//搜索server
	head = &inf->server_head;
	num = inf->server_num;
	for(i=0,pos = head->next;i<num && pos != head && n<max_count && m_s<m_rsp_server_num;++i)
	{
		node = rlist_entry(pos,UserNode,rlist);
		assert(node->user->flag);
		if(node->user->bshare && user!=node->user && (g_ConnectionType[(int)user->natType][(int)node->user->natType] || user->tcpRealIP==node->user->tcpRealIP))
		{
			m_s++;
			arr[n++] = node->user;
			//称到尾部
			next = pos->next;
			rlist_del(pos);
			rlist_add_tail(pos,head);
			pos = next;
		}
		else
		{
			pos = pos->next;
		}
	}

	//搜索user队列
	//注意multimap的linux下的结果： begin()-- == begin() ； end()++ == end()-- ；
	UserMap& pm = inf->user_map;
	PeerIter pm_it = pm.upper_bound(user->ip);
	if(pm_it==pm.end())
	{
		pm_it=pm.begin();
		assert(0);
	}
	PeerIter it_up,it_down;
	it_up = pm_it;
	it_down = pm_it;
	it_down--;
	UserInfo *des_peer=NULL;
	while(UT_CLIENT==user->userType && (it_up!=pm.end()||it_down!=pm.begin()))
	{
		if(n>=max_count)
			break;
		if(it_down!=pm.begin())
		{
			des_peer = it_down->second;
			//DEBUGMSG("#des_peer local port:%d \n",des_peer->tcpLocalPort);
			SOURCESERVICE_PRT("nodeuid %s, nattype=%d\n", des_peer->uid, g_ConnectionType[(int)user->natType][(int)des_peer->natType]);
			if(des_peer/* && des_peer->bshare*/ && user!=des_peer &&(g_ConnectionType[(int)user->natType][(int)des_peer->natType] || user->tcpRealIP==des_peer->tcpRealIP))
			{
				SOURCESERVICE_PRT("find source %s\n", des_peer->uid);
				m_u++;
				arr[n++] = des_peer;
			}
			it_down--;
		}
		if(n>=max_count)
			break;
		
		if(it_up!=pm.end())
		{
			des_peer = it_up->second;
			if(des_peer/* && des_peer->bshare */&& user!=des_peer &&(g_ConnectionType[(int)user->natType][(int)des_peer->natType] || user->tcpRealIP==des_peer->tcpRealIP))
			{
				m_u++;
				arr[n++] = des_peer;
			}
			it_up++;
		}
	}

	//UT_CENTER只考虑供下载服务器使用，或者server源过或者普通user过少也供其它使用
	//m_s<2 最少尽量返回2个以上服务器，尽量3个源以上
	//if(n<max_count && (m_s<2 || n<3 || UT_SERVER==user->userType))
	if(UT_SUPER==user->userType || UT_SERVER==user->userType)
	{
		head = &inf->center_head;
		num = inf->center_num;
		for(i=0,pos = head->next;i<num && pos != head && n<max_count && m_c<m_rsp_center_num;++i)
		{
			node = rlist_entry(pos,UserNode,rlist);
			assert(node->user->flag);
			if(node->user->bshare && user!=node->user && (g_ConnectionType[(int)user->natType][(int)node->user->natType] || user->tcpRealIP==node->user->tcpRealIP))
			{
				m_c++;
				arr[n++] = node->user;
				//称到尾部
				next = pos->next;
				rlist_del(pos);
				rlist_add_tail(pos,head);
				pos = next;
			}
			else
			{
				pos = pos->next;
			}
		}
	}
	
	if(0==n)
	{
		StatManagerSngl::instance()->on_nonsource_seach(tth);
	}
	else if(0==(m_u+m_s))
	{
		StatManagerSngl::instance()->on_noncssource_seach(tth);
	}
	return n;
}


int SourceService::get_source_num(int& file_num_live,int& source_num_live, int& file_num_vod,int& source_num_vod)
{
	/* get live information */
	SourceIter it;
	file_num_live = m_sourcel.size();
	source_num_live = 0;
	for(it=m_sourcel.begin();it!=m_sourcel.end();++it)
	{
		source_num_live += it->second->user_num;
		source_num_live += it->second->server_num;
		source_num_live += it->second->center_num;
		source_num_live += it->second->super_num;
	}

	/* get vod information */
	SourceItervod itvod ;
	//SourceMap *mapsource;
	//m_sourcevod_mux.lock();
	file_num_vod = m_sourcevod.size();
	source_num_vod = 0;
	for(itvod = m_sourcevod.begin(); itvod!=m_sourcevod.end(); ++itvod) {
		/*
		mapsource = itvod->second;
		int source_num_tmp = 0;
		for(it=mapsource->begin(); it!=mapsource->end(); ++it) {
			source_num_tmp += it->second->user_num;
			source_num_tmp += it->second->server_num;
			source_num_tmp += it->second->center_num;
			source_num_tmp += it->second->super_num;
		} 
		if(mapsource->size())source_num_vod += source_num_tmp/mapsource->size();
		*/
		source_num_vod +=  itvod->second->begin()->second->user_num;
		source_num_vod +=  itvod->second->begin()->second->server_num;
		source_num_vod +=  itvod->second->begin()->second->center_num;
		source_num_vod +=  itvod->second->begin()->second->super_num;
		
	}
	//m_sourcevod_mux.unlock();

	SOURCESERVICE_PRT("--------livefile count=%d,   livefile source count = %d ---\n",file_num_live,source_num_live);
	SOURCESERVICE_PRT("--------vodfile count=%d,   vodfile source count = %d ---\n",file_num_vod,source_num_vod);
	
	return 0;
}

int SourceService::get_sourcetype(const hash_t &tth) {
	int sourcetype = -1;
	
	//must be download type
	assert(tth.hash_type()==HT_URLDL);

	if(m_sourcel.find(tth) == m_sourcel.end()) {
		if(m_sourcevod.find(tth) != m_sourcevod.end()) {
			//vod mode
			sourcetype = 1;
		} else {
			//default, we think is live,maybe current source have not add in sourceservice
			sourcetype = 0;
		}
	} else {
		//live mode
		sourcetype = 0;
	}
	return sourcetype;
}

#endif

void SourceService::source_clear()
{
	//此接口由最后程序退出清理用,并不对user内信息同步
	SourceIter it;
	rlist_head_t *pos, *n;
	for(it=m_sourcel.begin();it!=m_sourcel.end();++it)
	{
		rlist_for_each_safe(pos,n,&it->second->server_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));
		rlist_for_each_safe(pos,n,&it->second->center_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));
		rlist_for_each_safe(pos,n,&it->second->super_head)
			m_free_peernodel.put_node(rlist_entry(pos,UserNode,rlist));

		it->second->user_map.clear();
		delete it->second;
	}
	m_sourcel.clear();
}
int SourceService::source_delete(UserInfo* user)                   //删除user所有共享文件信息
{
	//在此不能用for循环
	hash_t tth;
	map<hash_t,uint32>::iterator it = user->sourceMap.begin();
	while(it!=user->sourceMap.end())
	{
		tth = it->first;
		source_delete(tth,user);
		it = user->sourceMap.begin();
	}
	return 0;
}
//删除tth文件的特定user信息
int SourceService::source_delete(const hash_t &tth,UserInfo* user) 
{
	//1.删除user成员里面的信息
	map<hash_t,uint32>::iterator it2=user->sourceMap.find(tth);
	if(it2==user->sourceMap.end())
		return 0;
	if(HT_URLDL==it2->first.hash_type())
		user->downloadlist_num--;
	int urlflag = it2->second;
	user->sourceMap.erase(it2); //如果source_delete(UserInfo* user) 直接参使用it->first传入参数的话,在此会被释放掉,下面再使用tth会出错

	//2.删除source service里面的信息
	SourceIter it=m_sourcel.find(tth);
	assert(it!=m_sourcel.end());
	if(it==m_sourcel.end())
		return 0;

	SourceInfo *inf = it->second;
	if(urlflag)
		inf->urlflag_num--;
	bool bfind=false;
	if(UT_SERVER!=user->userType && UT_CENTER!=user->userType && UT_SUPER!=user->userType)
	{
		//head = &inf->head;
		//双向查找保证找到相同点
		//注意multimap的linux下的结果： begin()-- == begin() ； end()++ == end()-- ；
		PeerIter it=inf->user_map.find(user->ip);
		PeerIter it_i = it;
		while(it_i!=inf->user_map.end())
		{
			if(it_i->second == user)
			{
				inf->user_map.erase(it_i);
				inf->user_num--;
				if(user->menu & MENU_VIP)
				{
					inf->vip_num--;
					if(0==inf->vip_num)
					{
						m_vipfile_num--;
						AutoFileManagerSngl::instance()->on_vipfile_stop(tth);
					}
				}
				bfind = true;
				break;
			}
			//一直搜索到IP不相等为止（IP不相等的肯定不会是USER节点了）
			if(it_i->first != user->ip)
				break;
			++it_i;
		}
		it_i = it;
		while(!bfind && it_i!=inf->user_map.begin() && it_i!=inf->user_map.end())
		{
			if(it_i->second == user)
			{
				inf->user_map.erase(it_i);
				inf->user_num--;
				if(user->menu & MENU_VIP)
				{
					inf->vip_num--;
					if(0==inf->vip_num)
					{
						m_vipfile_num--;
					}
				}
				bfind = true;
				break;
			}
			if(it_i->first != user->ip)
				break;
			--it_i;
		}
	}
	else
	{
		UserNode *node=0;
		rlist_head_t *head = NULL;
		if(UT_SERVER==user->userType)
			head = &inf->server_head;
		else if(UT_CENTER==user->userType)
			head = &inf->center_head;
		else
			head = &inf->super_head;
		//注意:以下使用不安全循环,并且预取入cpu优化效率,所以删除一个点后只能立即跳出循环.
		rlist_head_t *ptr;
		rlist_for_each_prefetch(ptr,head)
		{
			node = rlist_entry(ptr,UserNode,rlist);
			if(node->user == user)
			{
				rlist_del(ptr);
				m_free_peernodel.put_node(node);
				if(UT_SERVER==user->userType)
					inf->server_num--;
				else if(UT_CENTER==user->userType)
					inf->center_num--;
				else
					inf->super_num--;
				bfind = true;
				break;
			}
		}
	}
	if(0==(inf->user_num+inf->server_num+inf->center_num+inf->super_num))
	{
		assert(rlist_empty(&inf->center_head) && rlist_empty(&inf->server_head) && rlist_empty(&inf->super_head) && 2==inf->user_map.size());
		assert(0==inf->vip_num);
		delete inf;
		m_sourcel.erase(it);
	}
	assert(bfind);
	return 0;
}

int SourceService::source_add(const hash_t &tth,uint64 size,uint32 block_size,UserInfo *user)
{
	if(user->sourceMap.end()!=user->sourceMap.find(tth))
		return -1;

	SourceIter it = m_sourcel.find(tth);
	SourceInfo *inf;
	if(it==m_sourcel.end())
	{
		inf = new SourceInfo();
		inf->size = size;
		inf->block_size = block_size;
		m_sourcel[tth] = inf;
	}
	else
	{
		inf = it->second;
	}

	//bool badd = false;
	if(UT_SERVER!=user->userType && UT_CENTER!=user->userType && UT_SUPER!=user->userType)
	{
		if(inf->user_map.insert(UserMap::value_type(user->ip,user))!=inf->user_map.end())
		{
			inf->user_num++;
			//添加到user自身的队列中
			int urlflag = 0;
			//少于500个源的时候，这些源都连接http源
			if(inf->urlflag_num<500)
				urlflag = 1; 
			else if(0==user->natType||1==user->natType)
				urlflag = 1;
			else if(inf->urlflag_num*2<inf->user_num)
				urlflag = 1;
			user->sourceMap[tth] = urlflag; //1表示此用户会从http源下载
			if(HT_URLDL==tth.hash_type())
				user->downloadlist_num++;
			if(urlflag)
				inf->urlflag_num++;
			if(user->menu & MENU_VIP)
			{
				inf->vip_num++;
				if(1==inf->vip_num)
					m_vipfile_num++;
			}
		}
	}
	else
	{
		UserNode *node = m_free_peernodel.get_node();
		if(node)
		{
			node->user = user;
			if(UT_SERVER==user->userType)
			{
				rlist_add_tail(&node->rlist,&inf->server_head);
				inf->server_num++;
			}
			else if(UT_CENTER==user->userType)
			{
				rlist_add_tail(&node->rlist,&inf->center_head);
				inf->center_num++;
			}
			else
			{
				rlist_add_tail(&node->rlist,&inf->super_head);
				inf->super_num++;
			}
			user->sourceMap[tth] = 0; //添加到user自身的队列中
			if(HT_URLDL==tth.hash_type())
				user->downloadlist_num++;
		}
	}
	return 0;
}

int SourceService::source_find(const hash_t &tth,UserInfo *user,UserInfo *arr[],uint64& size,uint32& block_size,int max_count/* = 100*/)
{
	SourceIter it=m_sourcel.find(tth);
	if(it==m_sourcel.end())
	{
		StatManagerSngl::instance()->on_nonfile_seach(tth);
		return -1;
	}

	UserNode *node = 0;
	SourceInfo *inf = it->second;
	rlist_head_t *head;
	size = inf->size;
	block_size = inf->block_size;
	if(max_count<=0)
		return 0;

	//在此每搜到一个源,就将它移到最后,总是从前往后找,保持最均分配
	rlist_head_t *pos,*next;
	int n=0,m_u=0,m_s=0,m_c=0,m_super=0;
	int i=0;
	int num=0;
	//搜索super
	head = &inf->super_head;
	num = inf->super_num;
	for(i=0,pos = head->next;i<num && pos != head && n<max_count && m_super<m_rsp_super_num;++i)
	{
		node = rlist_entry(pos,UserNode,rlist);
		assert(node->user->flag);
		if(node->user->bshare && user!=node->user && (g_ConnectionType[(int)user->natType][(int)node->user->natType] || user->tcpRealIP==node->user->tcpRealIP))
		{
			m_super++;
			arr[n++] = node->user;
			//称到尾部
			next = pos->next;
			rlist_del(pos);
			rlist_add_tail(pos,head);
			pos = next;
		}
		else
		{
			pos = pos->next;
		}
	}
	//搜索server
	head = &inf->server_head;
	num = inf->server_num;
	for(i=0,pos = head->next;i<num && pos != head && n<max_count && m_s<m_rsp_server_num;++i)
	{
		node = rlist_entry(pos,UserNode,rlist);
		assert(node->user->flag);
		if(node->user->bshare && user!=node->user && (g_ConnectionType[(int)user->natType][(int)node->user->natType] || user->tcpRealIP==node->user->tcpRealIP))
		{
			m_s++;
			arr[n++] = node->user;
			//称到尾部
			next = pos->next;
			rlist_del(pos);
			rlist_add_tail(pos,head);
			pos = next;
		}
		else
		{
			pos = pos->next;
		}
	}

	//搜索user队列
	//注意multimap的linux下的结果： begin()-- == begin() ； end()++ == end()-- ；
	UserMap& pm = inf->user_map;
	PeerIter pm_it = pm.upper_bound(user->ip);
	if(pm_it==pm.end())
	{
		pm_it=pm.begin();
		assert(0);
	}
	PeerIter it_up,it_down;
	it_up = pm_it;
	it_down = pm_it;
	it_down--;
	UserInfo *des_peer=NULL;
	while(UT_CLIENT==user->userType && (it_up!=pm.end()||it_down!=pm.begin()))
	{
		if(n>=max_count)
			break;
		if(it_down!=pm.begin())
		{
			des_peer = it_down->second;
			//DEBUGMSG("#des_peer local port:%d \n",des_peer->tcpLocalPort);
			if(des_peer/* && des_peer->bshare*/ && user!=des_peer &&(g_ConnectionType[(int)user->natType][(int)des_peer->natType] || user->tcpRealIP==des_peer->tcpRealIP))
			{
				m_u++;
				arr[n++] = des_peer;
			}
			it_down--;
		}
		if(n>=max_count)
			break;
		
		if(it_up!=pm.end())
		{
			des_peer = it_up->second;
			if(des_peer/* && des_peer->bshare */&& user!=des_peer &&(g_ConnectionType[(int)user->natType][(int)des_peer->natType] || user->tcpRealIP==des_peer->tcpRealIP))
			{
				m_u++;
				arr[n++] = des_peer;
			}
			it_up++;
		}
	}

	//UT_CENTER只考虑供下载服务器使用，或者server源过或者普通user过少也供其它使用
	//m_s<2 最少尽量返回2个以上服务器，尽量3个源以上
	//if(n<max_count && (m_s<2 || n<3 || UT_SERVER==user->userType))
	if(UT_SUPER==user->userType || UT_SERVER==user->userType)
	{
		head = &inf->center_head;
		num = inf->center_num;
		for(i=0,pos = head->next;i<num && pos != head && n<max_count && m_c<m_rsp_center_num;++i)
		{
			node = rlist_entry(pos,UserNode,rlist);
			assert(node->user->flag);
			if(node->user->bshare && user!=node->user && (g_ConnectionType[(int)user->natType][(int)node->user->natType] || user->tcpRealIP==node->user->tcpRealIP))
			{
				m_c++;
				arr[n++] = node->user;
				//称到尾部
				next = pos->next;
				rlist_del(pos);
				rlist_add_tail(pos,head);
				pos = next;
			}
			else
			{
				pos = pos->next;
			}
		}
	}
	
	if(0==n)
	{
		StatManagerSngl::instance()->on_nonsource_seach(tth);
	}
	else if(0==(m_u+m_s))
	{
		StatManagerSngl::instance()->on_noncssource_seach(tth);
	}
	return n;
}
int SourceService::get_source_num(int& file_num,int& source_num)
{
	SourceIter it;
	file_num = m_sourcel.size();
	source_num = 0;
	for(it=m_sourcel.begin();it!=m_sourcel.end();++it)
	{
		source_num += it->second->user_num;
		source_num += it->second->server_num;
		source_num += it->second->center_num;
		source_num += it->second->super_num;
	}
	DEBUGMSG("--------file count=%d,   file source count = %d ---\n",file_num,source_num);
	return 0;
}
int SourceService::get_msg_file_info(MsgFileInfo& inf)
{
	rlist_head_t *ptr = NULL;
	SourceIter it = m_sourcel.find(inf.hash);
	SourceInfo *si = NULL;
#ifdef SM_VOD
	SourceItervod itvod = m_sourcevod.find(inf.hash);
#endif
	if(it!=m_sourcel.end())
	{
		si = it->second;
		inf.result = 1;
		inf.user_num = si->user_num;
		inf.server_num = si->server_num;
		inf.center_num = si->center_num;
		inf.super_num = si->super_num;
		
		rlist_for_each_prefetch(ptr,&si->server_head)
		{
			inf.svrs.push_back(rlist_entry(ptr,UserNode,rlist)->user->ip);
		}
		rlist_for_each_prefetch(ptr,&si->center_head)
		{
			inf.svrs.push_back(rlist_entry(ptr,UserNode,rlist)->user->ip);
		}
		rlist_for_each_prefetch(ptr,&si->super_head)
		{
			inf.svrs.push_back(rlist_entry(ptr,UserNode,rlist)->user->ip);
		}
		return 0;
	}
#ifdef SM_VOD
	else if(itvod!=m_sourcevod.end()) {
			/*SourceMap *mapsource;
			mapsource = itvod->second;
			int source_num_tmp = 0;
			for(it=mapsource->begin(); it!=mapsource->end(); ++it) {
				inf.user_num += it->second->user_num;
				inf.server_num += it->second->server_num;
				inf.center_num += it->second->center_num;
				inf.super_num += it->second->super_num;
			}
			if(mapsource->size()) {
				inf.user_num  /= mapsource->size();
				inf.server_num  /= mapsource->size();
				inf.center_num  /= mapsource->size();
				inf.super_num  /= mapsource->size();
			}*/
			inf.result = 1;
			si = itvod->second->begin()->second;
			inf.user_num = si->user_num;
			inf.server_num = si->server_num;
			inf.center_num = si->center_num;
			inf.super_num = si->super_num;
		
			rlist_for_each_prefetch(ptr,&si->server_head)
			{
				inf.svrs.push_back(rlist_entry(ptr,UserNode,rlist)->user->ip);
			}
			rlist_for_each_prefetch(ptr,&si->center_head)
			{
				inf.svrs.push_back(rlist_entry(ptr,UserNode,rlist)->user->ip);
			}
			rlist_for_each_prefetch(ptr,&si->super_head)
			{
				inf.svrs.push_back(rlist_entry(ptr,UserNode,rlist)->user->ip);
			}
			return 0;
	}
#endif /* end of SM_VOD */
	return -1;
}
int SourceService::get_msg_allfile_info(list<MsgFileInfo*>& ls)
{
	MsgFileInfo* inf;
	SourceInfo *si = NULL;
	for(SourceIter it=m_sourcel.begin();it!=m_sourcel.end();++it)
	{
		inf = new MsgFileInfo();
		si = it->second;
		inf->hash = it->first;
		inf->user_num = si->user_num;
		inf->server_num = si->server_num;
		inf->center_num = si->center_num;
		inf->super_num = si->super_num;

		ls.push_back(inf);
	}
	return 0;
}

int SourceService::get_super_source_num(const hash_t& tth)
{
	SourceIter it=m_sourcel.find(tth);
	if(it!=m_sourcel.end())
	{
		return it->second->super_num;
	}
	return 0;
}
int SourceService::get_super_by_hash(const hash_t& hash,list<UserInfo*>& ls)
{
	SourceIter it=m_sourcel.find(hash);
	if(it==m_sourcel.end())
		return 0;
	//注意:以下使用不安全循环,并且预取入cpu优化效率
	rlist_head_t *head = &it->second->super_head;
	rlist_head_t *ptr = NULL;
	rlist_for_each_prefetch(ptr,head)
	{
		ls.push_back(rlist_entry(ptr,UserNode,rlist)->user);
	}
	return 0;
}
