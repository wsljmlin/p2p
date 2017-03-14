#pragma once
#include "commons.h"
#include "BlockTable.h"
#include "BlockDataTable.h"
#include "FileBlock.h"

//注意：一般地，FileInfo的所有接口只能由FileStorage调用比较安全
//其它地方直接调用的话，外部访问FileStorage时可能会出现线程安全问题。

class FileStorage;
class FileInfo
{
public:
	typedef map<unsigned int,FileBlock*> FileBlockMap;
	typedef FileBlockMap::iterator FileBlockMapIter;
	FileInfo(void);
	~FileInfo(void);
	friend class FileStorage;

	unsigned int get_block_size(unsigned int index);

	bool is_memcache_only() const;
	bool is_allow_pause();
	bool is_finished()const { return (size>0 && size!=UINT64_INFINITE && down_blocks==blocks);}
	bool is_memfinished() const{ return (size>0 && size!=UINT64_INFINITE && bt_memfinished.is_all_set());}
	unsigned int get_block_downing_size(unsigned int index);
	bool check_memcache_done();
private:
	bool open_file(int mode,int rdbftype=RDBF_AUTO);
	void close_file();

	int flush_file(unsigned int index);
	void flush_file_all();
	bool try_add_memcache(unsigned int index,FileBlock* block);
	void free_memcache_all();

public:
	string			tth;
	hash_t			hash;
	size64_t		size;
	string			path;
	string			despath;
	string			info_path;
	string			url; //记录一个主URL，这个URL不保存到本地，只是开始下载时产生
	int				ftype;
	int				ref;
	int				ctime; //创建时间
	int				mtime; //最后使用时间
	int				flag;  //暂时未使用，保留，例如以后用于恢复原格式文件等标志使用

	unsigned int	block_size;
	unsigned int	block_offset; //指定从第几块开始下载
	unsigned int	blocks; //一共需要下载块数
	unsigned int	block_gap; //下载缺口,指从block_offset块起，第个出现未下载的块号
	unsigned int	down_blocks; //已经下载完的块数

	BlockTable		bt_finished;
	BlockTable		bt_memfinished;
	BlockDataTable	bdt;
	size64_t		req_offset;
	size64_t		last_req_offset;
	
	FileBlockMap	fbmp_mem;
	ERDBFile64		_file;
#ifdef SM_VOD
	int filetype;
#endif
};

