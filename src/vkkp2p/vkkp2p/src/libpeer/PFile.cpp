#include "PFile.h"
#include "Interface.h"
#include "Setting.h"


PFile::PFile(void)
:m_size(0)
,m_offset(0)
{
}

PFile::~PFile(void)
{
}
//PFile* PFile::open_url(const char* url)
//{
//	//以url的sha1作为hash种子
//	if(!url||0==strlen(url))
//		return NULL;
//	hash_t hash;
//	hash.set_url_string(url);
//	return open(hash,url);
//}
//PFile* PFile::open(const char* strsha1,const char* url/*=NULL*/)
//{
//	if(!strsha1)
//		return NULL;
//	hash_t hash;
//	if(0!=hash.set_string_hash(strsha1))
//		return NULL;
//	return open(hash,NULL,url);
//}

PFile* PFile::open(const hash_t& hash,const char* path/*=NULL*/,const char* url/*=NULL*/,int ftype/*=0*/)
{
	PFile* file = new PFile();
	if(!file)
		return NULL;
	if(0!=PIF::instance()->create_download(hash,path,url,0,0,SettingSngl::instance()->get_block_size(),ftype))
	{
		delete file;
		return NULL;
	}
	PIF::instance()->read_refer(hash);
	file->m_hash = hash;
	return file;
}

#ifdef SM_VOD
PFile* PFile::open(int playtype,const hash_t& hash, const char* path/*=NULL*/,const char* url/*=NULL*/,int ftype/*=0*/)
{
	PFile* file = new PFile();
	if(!file)
		return NULL;
	if(0!=PIF::instance()->create_download(hash, playtype, path,url,0,0,SettingSngl::instance()->get_block_size(),ftype))
	{
		delete file;
		return NULL;
	}
	PIF::instance()->read_refer(hash);
	file->m_hash = hash;
	return file;
}
#endif /* end of SM_VOD */

void PFile::close()
{
	PIF::instance()->read_release(m_hash);
	delete this;
}
uint64 PFile::size()
{
	if(0==m_size)
		m_size = PIF::instance()->get_filesize(m_hash);
	return m_size;
}
bool PFile::seek(uint64 offset)
{
	if(0==m_size)
		return false;
	m_offset = offset;
	if(m_offset<m_size)
		PIF::instance()->get_filedata(m_hash,NULL,0,m_offset);
	return true;
}
uint64 PFile::tell()
{
	return m_offset;
}
int PFile::read(char* buf,int size)
{
	if(m_offset>=m_size)
		return -1;
	if(m_offset+size>m_size)
		size = (int)(m_size-m_offset);
	int n = PIF::instance()->get_filedata(m_hash,buf,size,m_offset);
	if(n>0) m_offset += n;
	return n;
}
