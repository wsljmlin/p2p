#pragma once
#include "commons.h"
#include "BlockTable.h"
#include "BlockDataTable.h"
#include "FileBlock.h"

//ע�⣺һ��أ�FileInfo�����нӿ�ֻ����FileStorage���ñȽϰ�ȫ
//�����ط�ֱ�ӵ��õĻ����ⲿ����FileStorageʱ���ܻ�����̰߳�ȫ���⡣

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
	string			url; //��¼һ����URL�����URL�����浽���أ�ֻ�ǿ�ʼ����ʱ����
	int				ftype;
	int				ref;
	int				ctime; //����ʱ��
	int				mtime; //���ʹ��ʱ��
	int				flag;  //��ʱδʹ�ã������������Ժ����ڻָ�ԭ��ʽ�ļ��ȱ�־ʹ��

	unsigned int	block_size;
	unsigned int	block_offset; //ָ���ӵڼ��鿪ʼ����
	unsigned int	blocks; //һ����Ҫ���ؿ���
	unsigned int	block_gap; //����ȱ��,ָ��block_offset���𣬵ڸ�����δ���صĿ��
	unsigned int	down_blocks; //�Ѿ�������Ŀ���

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

