#pragma once

#include "File64.h"
#include <string.h>
#include <assert.h>


//typedef struct tagRDBEIFNode
//{
//	int head_size;
//	ssize64_t file_size;
//	ssize64_t begin_pos;
//	int id;
//	char name[128];
//	tagRDBEIFNode& operator=(const tagRDBEIFNode& inf)
//	{
//		head_size = inf.head_size;
//		file_size = inf.file_size;
//		begin_pos = inf.begin_pos;
//		id = inf.id;
//		memcpy(name,inf.name,128);
//		return *this;
//	}
//}RDBEIFNode_t;
//typedef struct tagRDBEIFInfo
//{
//	size64_t rdb_file_size;
//	int rdb_block_size;
//	int all_file_num;
//	int try_get_file_num;
//	int real_get_file_num;
//	RDBEIFNode_t *files;
//}RDBEIFInfo_t;


//基本文件（常规文件）
class BFile64
{
public:
	BFile64(void) {}
	virtual ~BFile64(void) {}
	virtual int open(const char* path,int mode){ return _file.open(path,mode);}
	virtual int close() { return _file.close(); }
	virtual ssize64_t seek(ssize64_t distance,int smode) { return _file.seek(distance,smode); }
	virtual int write(char *buf,int len) { return _file.write(buf,len); }
	virtual int read(char *buf,int len) { return _file.read(buf,len); }
	virtual int flush() { return _file.flush(); }
	virtual ssize64_t tell() { return _file.tell(); }
	virtual bool is_open() const { return _file.is_open(); } 
	virtual int resize(ssize64_t size);
	virtual size64_t get_file_size(){return _file.get_file_size();}

	////***********************************************************************************************************
	////扩展信息文件相关接口--Extended information file
	//virtual int eif_zip_file(const char* path,int id){return -1;}
	//virtual int eif_get_zip_info(RDBEIFInfo_t& zi){return -1;}
	//virtual int eif_get_zip_infoi(const char* name,RDBEIFNode_t& inf){return -1;}
	//virtual int eif_unzip_all_file(const char* dir){return -1;}
	//
	////扩展信息文件相关接口--Extended information file
	//static int eif_get_node(File64& file,ssize64_t bpos,RDBEIFNode_t& inf);
	//static int eif_copy_file(File64& from,ssize64_t from_pos,File64& to,ssize64_t to_pos,size64_t size);
protected:
	File64 _file;
};


//******************************************************************************

#define RDB_ENDIAN        0xCCCCFFFF
#define RDBS_STX           ".scf.$CREATE.BY.CLHE.TIME_20130110.777$."
class RDBFile64Simple : public BFile64
{
public:
	RDBFile64Simple(void);
	~RDBFile64Simple(void);
public:
	virtual int open(const char* path,int mode);
	virtual int close();
	virtual ssize64_t seek(ssize64_t distance,int smode);
	virtual int write(char *buf,int len) {assert(false);return 0;}
	virtual int read(char *buf,int len);
	virtual int flush() { assert(false);return 0; }
	virtual ssize64_t tell(){return m_pos;}
	virtual bool is_open() const { return _file.is_open(); } 
	virtual int resize(ssize64_t size){assert(false);return 0;}
	virtual size64_t get_file_size(){return m_file_size;}

	static int normal_to_rdbs_file(const char* path,const char* to_path);
	static int rdbs_to_normal_file(const char* path,const char* to_path);
private:
	int m_head_size;
	size64_t m_file_size;
	ssize64_t m_pos;
};


