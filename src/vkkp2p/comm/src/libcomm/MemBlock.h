#pragma once
#include "Singleton.h"
#include <assert.h>
#include <string.h>

#define NEW_MBLOCK_RETURN_INT(name,size,ret)  MemBlock *name = MemBlock::allot(size); if(NULL==name) {return ret;}

class MemBlock
{
public:
	char *buf;
	int buflen,datapos,datalen;
	MemBlock *next,*prev; //供tracker使用，避免list的反复new DataType内存
	int __i;
public:
	MemBlock(int len=0,int i=-1):buf(len>0?(new char[len]):0),buflen(len>=0?len:0),datapos(0),datalen(0),next(0),prev(0),__i(i)
	{
		if(buf) memset(buf,0,len);
	}
	~MemBlock(void){if(buf) delete[] buf;}
	static MemBlock* allot(int size,int token=1); //考虑只是tracker使用token，而且通信程序都使用线程1的内在列，所以这里默认值给1吧
	void free(int token=1);

	int remaining() const {return datalen-datapos;}
	char* pointer() const {return buf+datapos;}
	void position(int pos) {datapos = pos;}
	int position() const {return datapos;}
	int limit()const {return datalen;}
	void limit(int size){datalen = size;}
	void increase(int size){ datapos += size;}

	MemBlock* dup(int token=1);
};

class MemBlockPoolI
{
public:
	virtual ~MemBlockPoolI(void){}
public:
	virtual int init(){return 0;}
	virtual void fini(){}
	//token 只供tracker的设计使用
	virtual MemBlock* get_block(int size,int token=0){assert(false);return 0;}
	virtual void put_block(MemBlock* b,int token=0){assert(false);}
};


//基础的只是简单new delete,仅实现最简单的使用演示
class MemBlockPoolBase : public MemBlockPoolI
{
public:
	MemBlockPoolBase(void){}
	virtual ~MemBlockPoolBase(void){}
public:

	//token 只供tracker的设计使用
	virtual MemBlock* get_block(int size,int token=0) {return new MemBlock(size);}
	virtual void put_block(MemBlock* b,int token=0) { if(b) delete b;}
};

typedef Singleton<MemBlockPoolI> MemBlockPoolSngl;

