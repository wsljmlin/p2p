#pragma once
//*****************************
//*
//*@function:
//*@[create,review]:[hechunlong(2011-5-12),*-*|*-*]
//*@[modify,review]:[hechunlong(2011-5-12),*-*|*-*]
//*@remark: 
//* 2011-5-12 优化节目源的分配算法,将普通用户源根据IP排序分配。
//*
//*****************************

#include "User.h"
#include "rlist.h"
#include <map>
#include "TrackerMessage.h"
#ifdef SM_VOD
#include "SynchroObj.h"
#endif

typedef struct tagUserNode{
	UserInfo *user;
	rlist_head_t rlist;
}UserNode;

class UserNodeList
{
public:
	UserNodeList(void) 
	{
		rlist_init(&m_head);
		m_size = 0;
		m_all_num = 0;
	}
	~UserNodeList(void)
	{
		clear();
	}
	void clear();
	void reserve(unsigned int n);
	UserNode *get_node();
	void put_node(UserNode *node);
	int size()const {return m_size;}
private:
	rlist_head_t m_head;
	unsigned int m_all_num;
	unsigned int m_size;
};


typedef std::multimap<uint32,UserInfo*> UserMap;
typedef UserMap::iterator PeerIter;
typedef struct tagSourceInfo{
	uint64 size;
	uint32 block_size;
	uint32 user_num;
	uint32 server_num;
	uint32 center_num;
	uint32 super_num;
	uint32 urlflag_num; //记录允许http连接的个数
	uint32 vip_num; //VIP源计数，0表示不是VIP节目，非0表示VIP节目
	rlist_head_t server_head;  //peer_type_server列
	rlist_head_t center_head; //中心服务器源，后备源
	rlist_head_t super_head;
	UserMap user_map;

	tagSourceInfo(void)
		:size(0)
		,block_size(0)
		,user_num(0)
		,server_num(0)
		,center_num(0)
		,super_num(0)
		,urlflag_num(0)
		,vip_num(0)
	{
		rlist_init(&server_head);
		rlist_init(&center_head);
		rlist_init(&super_head);
		//使upper_bound正常IP总能定位
		user_map.insert(UserMap::value_type(0,0));
		user_map.insert(UserMap::value_type(0xffffffff,0));
	}
}SourceInfo;


class SourceService
{
public:
	SourceService(void);
	~SourceService(void);

	typedef map<hash_t,SourceInfo*> SourceMap;
	typedef SourceMap::iterator SourceIter;
#ifdef SM_VOD
	/* second index, first key is programe name, second key is slice file */
	typedef map<hash_t,SourceMap*> SourceMapvod;
	typedef SourceMapvod::iterator SourceItervod;
#endif

public:
	int init();
	void fini();

	void handle_timeout();
#ifdef SM_VOD
	void source_clear_ex();
	int urldlsource_delete(const hash_t &tth,UserInfo* user);
	int url2source_delete(const hash_t &tthpl, UserInfo* user);
	int source_delete_vod(UserInfo* user);
	int source_delete_vod(const hash_t &tth,UserInfo* user);
	int source_add(const hash_t &tth,int sourcetype, uint64 size,uint32 block_size,UserInfo *user);
	int source_find(const hash_t &tth,int sourcetype, UserInfo *user,UserInfo *arr[],uint64& size,uint32& block_size,int max_count = 100);
	int get_source_num(int& file_num_live,int& source_num_live, int& file_num_vod,int& source_num_vod);
	void print_user_source(UserInfo *user);
	void print_sources();
	int get_sourcetype(const hash_t &tth);
#endif
	void source_clear();
	int source_delete(UserInfo* user);                   //删除user所有共享文件信息
	int source_delete(const hash_t &tth,UserInfo* user); //删除tth文件的特定user信息
	int source_add(const hash_t &tth,uint64 size,uint32 block_size,UserInfo *user);

	int source_find(const hash_t &tth,UserInfo *user,UserInfo *arr[],uint64& size,uint32& block_size,int max_count = 100);
	//int source_peer_count(size_t& peer_count,size_t &max_peersource_count);

	int get_file_num(){return m_sourcel.size();}
	int get_source_num(int& file_num,int& source_num);
	int get_msg_file_info(MsgFileInfo& inf);
	int get_msg_allfile_info(list<MsgFileInfo*>& ls);
	int get_super_source_num(const hash_t& tth);
	int get_super_by_hash(const hash_t& hash,list<UserInfo*>& ls);
private:
	int source_delete(const hash_t& tth);                //删除tth的所有所有共享信息

private:
	bool m_binit;
	SourceMap m_sourcel; /* used only for live */
#ifdef SM_VOD
	SourceMapvod m_sourcevod;
	Simple_Mutex m_sourcevod_mux;
#endif
	UserNodeList m_free_peernodel;
	int m_rsp_server_num;
	int m_rsp_center_num;
	int m_rsp_super_num;
	int m_vipfile_num; //记录当前在线的VIP节目个数
};
typedef Singleton<SourceService> SourceServiceSngl;


