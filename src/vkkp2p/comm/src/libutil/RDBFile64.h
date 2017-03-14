#pragma once
#include "RDBFile64Base.h"

//随机块文件
//前40分为标志
//注意:字节序在我写入时为0xCCFF,如果读入一个unsigned short时为0xFFFFCCCC,则后面读的大小必须做相应swap()交换
//1024*1024=1048576, 2048*1024=2097152
#define RDB_STX           ".RDB.$CREATE.BY.CLHE.TIME_20100306.777$."
#define RDB_HEAD_SIZE     1048576
#define RDB_BLOCK_SIZE    2097152

class RDBFile64 : public BFile64
{
public:
	RDBFile64(void);
	virtual ~RDBFile64(void);

	typedef struct tagBlockInfo{
		int index; //指物理文件索引位置
		unsigned int size;
	}BlockInfo;

public:
	virtual int open(const char* path,int mode);
	virtual int close();
	virtual ssize64_t seek(ssize64_t distance,int smode);
	virtual int write(char *buf,int len);
	virtual int read(char *buf,int len);
	virtual int flush();
	virtual ssize64_t tell();
	virtual int resize(ssize64_t size);
	virtual size64_t get_file_size(){return m_file_size;}

	////扩展信息文件相关接口--Extended information file
	//virtual int eif_zip_file(const char* path,int id);
	//virtual int eif_get_zip_info(RDBEIFInfo_t& zi); 
	//virtual int eif_get_zip_infoi(const char* name,RDBEIFNode_t& node);
	//virtual int eif_unzip_all_file(const char* dir);
private:
	void reset();
	int create_head();
	int load_head();
	int check_resize_logbs_ok(int blocks);

private:
	size64_t m_file_size;
	size64_t m_file_write_size;
	int m_block_size;
	int m_head_size;
	int m_file_write_blocks;  //指定数组已经占用到第几块
	size64_t m_pos;

	BlockInfo *m_logbs; //数组下标表示逻辑块号(即原文件的块号),BlockInfo::index表示物理块号,即块文件的绝对块号
	int m_logbs_size;   //数组大小
};

enum {RDBF_UNKNOW=-1,RDBF_AUTO=0,RDBF_BASE=1,RDBF_RDB=2,RDBF_RDBS=3};


class ERDBFile64
{
public:
	ERDBFile64(void);
	~ERDBFile64(void);
public:
	int open(const char* path,int mode,int ftype=RDBF_AUTO);
	int close();
	ssize64_t seek(ssize64_t distance,int smode);
	int write(char *buf,int len);
	int read(char *buf,int len);
	int flush();
	ssize64_t tell();
	bool is_open() const { return m_pfile!=NULL; } 
	int resize(ssize64_t size) { if(m_pfile) return m_pfile->resize(size); return -1;}
	ssize64_t get_file_size() { if(m_pfile) return m_pfile->get_file_size(); return 0;}

	////扩展信息文件相关接口--Extended information file
	//int eif_zip_file(const char* path,int id){ if(m_pfile) return m_pfile->eif_zip_file(path,id); return -1;}
	////zi结构预分配最多需要读取的文件数的内存空间zi.try_get_file_num 指定最多预读个数
	//int eif_get_zip_info(RDBEIFInfo_t& zi){ if(m_pfile) return m_pfile->eif_get_zip_info(zi); return -1;}
	//int eif_get_zip_infoi(const char* name,RDBEIFNode_t& node){if(m_pfile) return m_pfile->eif_get_zip_infoi(name,node); return -1;}
	//int eif_unzip_all_file(const char* dir){ if(m_pfile) return m_pfile->eif_unzip_all_file(dir);  return -1;}

	int get_filetype() const { return m_file_type;}


	static size64_t get_filesize(const char* path);
	static int get_filetype(const char* path);
protected:
	BFile64 *m_pfile;
	int m_file_type;
};

