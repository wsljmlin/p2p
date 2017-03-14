#pragma once
#include "nhash.h"
#include "ntypes.h"


//enum {FTYPE_VOD=0,FTYPE_DOWNLOAD,FTYPE_SHAREONLY};


class PFile
{
private:
	PFile(void);
	~PFile(void);
public:
	//static PFile* open_url(const char* url);
	//static PFile* open(const char* strsha1,const char* url=NULL);
	static PFile* open(const hash_t& hash,const char* path=NULL,const char* url=NULL,int ftype=0);
#ifdef SM_VOD
	static PFile* open( int playtype, const hash_t& hash,const char* path=NULL,const char* url=NULL,int ftype=0);
#else
	
#endif
	void close();
	uint64 size();
	bool seek(uint64 offset);
	uint64 tell();
	int read(char* buf,int size);
private:
	hash_t m_hash;
	uint64 m_size;
	uint64 m_offset;
};
