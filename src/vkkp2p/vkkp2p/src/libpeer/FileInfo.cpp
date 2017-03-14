#include "FileInfo.h"
#include "Setting.h"
#include "FileStorage.h"

FileInfo::FileInfo(void)
:size(0)
,ftype(0)
,ref(0)
,ctime(0)
,mtime(0)
,flag(0)
,block_size(SettingSngl::instance()->get_block_size())
,block_offset(0)
,blocks(0)
,block_gap(0)
,down_blocks(0)
,req_offset(0)
,last_req_offset(0)
{
}

FileInfo::~FileInfo(void)
{
	free_memcache_all();
}

bool FileInfo::open_file(int mode,int rdbftype/*=RDBF_AUTO*/)
{
	if(_file.is_open())
		return true;
	//不保存硬盘的，只要新创建文件的才会不实际打开文件
	if((mode&F64_TRUN) && is_memcache_only())
		return true;
	return (0==_file.open(path.c_str(),mode,rdbftype));
}
void FileInfo::close_file()
{
	_file.close();
}
int FileInfo::flush_file(unsigned int index)
{
	int ret=-1;
	BlockDataTable::BlockData_t& bd = bdt[index];
	FileBlock *block = (FileBlock*)bd.data;
	if(!block)
		return -1;
	//点播不cache到硬盘的不写
	if(!is_memcache_only())
	{
		if(open_file(F64_RDWR))
		{
			unsigned int index_pos = bd.size;
			assert(block->begin <= index_pos);
			int write_size = block->end - index_pos;
			if(write_size>0)
			{
				size64_t pos = index*(size64_t)block_size + index_pos;
				_file.seek(pos,SEEK_SET);
				if(write_size==_file.write(block->buf+index_pos,write_size))
				{
					bd.size += write_size;
					ret = 0;
				}
				else
				{
					DEBUGMSG("# *** write file failed! --> SetIsWriteDisk(0) \n");
					SettingSngl::instance()->set_memcache_only();
				}
				_file.flush();
			}
		}
	}
	//不管写不写成功，都要清理
	FileStorageSngl::instance()->put_fileblock(block);
	bd.data = 0;
	return ret;
}
void FileInfo::flush_file_all()
{
	for(unsigned int i=block_offset;i<blocks;++i)
		flush_file(i);
}
unsigned int FileInfo::get_block_size(unsigned int index)
{
	if(UINT64_INFINITE==size)
		return block_size;
	unsigned int n = (int)((size-1)/block_size + 1);
	assert(index < n);
	if (index < n-1)
		return block_size;
	else
		return (int)(size - block_size * (size64_t)index);
}

bool FileInfo::is_memcache_only() const
{
#ifdef SM_VOD
	if(HT_URL2==hash.hash_type()) {
		return ((FTYPE_VOD==ftype) && (!((1==SettingSngl::instance()->get_cache_flag()) && (PLAYTYPE_VOD==filetype))));
	} else {
		return ((FTYPE_VOD==ftype) && (!((1==SettingSngl::instance()->get_cache_flag_vod()) && (PLAYTYPE_VOD==filetype))));
	}
#else
	if(HT_URL2==hash.hash_type())
		return (FTYPE_VOD==ftype && 0==SettingSngl::instance()->get_cache_flag());
	else
		return (FTYPE_VOD==ftype && 0==SettingSngl::instance()->get_cache_flag_vod());
#endif /* end of SM_VOD */
}
bool FileInfo::is_allow_pause()
{
	if(HT_URL2==hash.hash_type())
		return (FTYPE_VOD==ftype && 1!=SettingSngl::instance()->get_cache_flag() && size>SettingSngl::instance()->get_memcache_size());
	else
		return (FTYPE_VOD==ftype && 1!=SettingSngl::instance()->get_cache_flag_vod() && size>SettingSngl::instance()->get_memcache_size());
}
bool FileInfo::try_add_memcache(unsigned int index,FileBlock* block)
{
	if(!block)
		return false;
	int win = SettingSngl::instance()->get_memcache_win();
	if(win<=0)
		return false;
	unsigned int need_i = (unsigned int)(req_offset/block_size);

	//因为可能lsp会整块预取数据，导致预取完后夸块
	//所以保存多两块
	int tmp = need_i>=2?2:need_i;
	need_i -= tmp;
	win += tmp;
	if(index<need_i || index>=(need_i+win))
	{
		//DEBUGMSG("#***try_add_memcache(%d) need_i=%d size=%d\n",index,need_i,fbmp_mem.size());
		return false;
	}
	if(fbmp_mem.find(index)!=fbmp_mem.end())
	{
		assert(false); 
		return true;
	}
	fbmp_mem[index] = block;
	block->ref++;  //增加引用，与FileStorageSngl::instance()->put_fileblock()对应
	if(bt_memfinished.get_block_num()>0)  //注意可能是readyfile
		bt_memfinished.set_block(index);
	//清理win之外的
	FileBlockMapIter it;
	if((int)fbmp_mem.size()>win)
	{
		for(it=fbmp_mem.begin();(int)fbmp_mem.size()>win && it!=fbmp_mem.end();)
		{
			if(it->first<need_i || it->first>=(need_i+win))
			{
				//释放
				FileStorageSngl::instance()->put_fileblock(it->second);
				if(bt_memfinished.get_block_num()>0 && bt_finished.get_block_num()>0 && !bt_finished[it->first])
					bt_memfinished.set_block(it->first,false);
				fbmp_mem.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}
	return true;
}
void FileInfo::free_memcache_all()
{
	FileBlockMapIter it;
	for(it=fbmp_mem.begin();it!=fbmp_mem.end();++it)
	{
		//如果没有写入硬盘，要将tableMBFinished对应块清0
		FileStorageSngl::instance()->put_fileblock(it->second);
		if(bt_memfinished.get_block_num()>0 && bt_finished.get_block_num()>0 && !bt_finished[it->first])
			bt_memfinished.set_block(it->first,false);
	}
	fbmp_mem.clear();
}
unsigned int FileInfo::get_block_downing_size(unsigned int index)
{
	BlockDataTable::BlockData_t& bd = bdt[index];
	if(!bd.data)
		return bd.size;
	return ((const FileBlock*)bd.data)->end;
}
bool FileInfo::check_memcache_done()
{
	//目前只有playlist 类型的才检查memcache_done功能，因为实时检查开销比较大，vod暂不检查
	if(HT_URL2==hash.hash_type())
	{
		if(1!=SettingSngl::instance()->get_cache_flag() && bt_memfinished.is_all_set()) 
			return true;
	}
	return false;
}

