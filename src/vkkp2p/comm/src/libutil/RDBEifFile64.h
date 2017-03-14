#pragma once
#include "RDBFile64.h"
#include "cyclist.h"

#define EIF_HEAD_SIZE 152
#define	EIF_STX_END_SIZE 8
#define EIF_STX_BEG ".eif.beg"
#define EIF_STX_END ".eif.end"
//1个文件总长为 160 + file_size;

//************************************************************************
typedef struct tag_rdbeif_node
{
	File64 file;
	int id;
	char name[128];
	int head_size;

	size64_t file_size;
	size64_t file_beg_offset; //eif文件的起始位置
	size64_t file_rpos; //eif文件相对读位置

	tag_rdbeif_node(void)
	{
		
	}
}rdbeif_node_t;


typedef struct tag_rdbeif_handle
{
	File64 file;
	int file_type;
	char path[256];
	size64_t eif_beg_offset;
	size64_t eif_end_offset;
	cyclist<rdbeif_node_t*> nl;
}rdbeif_handle_t;

//*************************************************************************
//
rdbeif_handle_t* rdbeif_open(const char* cvipath,int mode);
int rdbeif_close(rdbeif_handle_t* h);
int rdbeif_zip_file(rdbeif_handle_t* h,const char* path,int id,bool bcover);
int rdbeif_unzip_all(rdbeif_handle_t* h,const char* dir);
int rdbeif_clean(const char* cvipath);

//*************************************************************************
//eif单文件读访问接口
rdbeif_node_t* rdbeif_node_open(const char* rdbpath,const char* eif_name);
void rdbeif_node_close(rdbeif_node_t* h);
int rdbeif_node_seek(rdbeif_node_t* h,size64_t pos);
int rdbeif_node_read(rdbeif_node_t* h,char* buf,int size);
int rdbeif_node_eof(rdbeif_node_t* h);

