#include "RDBEifFile64.h"
#include <assert.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#else
#define stricmp strcasecmp
#endif


ssize64_t rdbeif_get_beg_pos_base(File64& file,size64_t pos)
{
	if(pos<8)
		return pos;
	char buf[32];
	size64_t size,file_size;
	unsigned int head_size;
	file.seek(pos-8,SEEK_SET);
	if(8!=file.read((char*)&size,8)) //读最后8字节大小
		return pos;
	if(size+8>(size64_t)pos || size<20) //越界
		return pos;
	file.seek(pos-8-size,SEEK_SET);
	if(20!=file.read(buf,20))
		return pos;
	
	memcpy((char*)&head_size,buf+8,4);
	memcpy((char*)&file_size,buf+12,8);
	if(0==memcmp(buf,EIF_STX_BEG,8) && head_size+file_size == size)
		return rdbeif_get_beg_pos_base(file,pos-8-size);
	return pos;
}
ssize64_t rdbeif_get_beg_pos(const char* path)
{
	File64 file;
	if(0!=file.open(path,F64_READ))
		return 0;
	size64_t size = file.seek(0,SEEK_END);
	if((size64_t)-1==size || size < 1024)
	{
		return rdbeif_get_beg_pos_base(file,size);
	}

	char buf[128];
	int endian = 0;
	int ver = 0;
	int pos = 0;
	file.seek(0,SEEK_SET);
	if(48!=file.read(buf,48))
		return size;
	pos = 40;
	memcpy((char*)&endian,buf+pos,4);
	pos += 4;
	memcpy((char*)&ver,buf+pos,4);
	//注意:在此判断endian有没有跟原来写入的序相反,若相反实际上是要交换的,暂时这里不支持那么快吧

	if(0==strncmp(buf,RDB_STX,40))
	{
		int head_size=0;
		int file_write_blocks=0,block_size=0;
		file.read((char*)&head_size,4);
		file.seek(16,SEEK_CUR);
		file.read((char*)&file_write_blocks,4);
		file.read((char*)&block_size,4);
		size = ((size64_t)file_write_blocks)*block_size + head_size;
	}
	else if(0==strncmp(buf,RDBS_STX,40))
	{
		int head_size=0;
		size64_t file_size = 0;
		file.read((char*)&head_size,4);
		file.read((char*)&file_size,8);
		size = file_size + head_size;
	}
	return rdbeif_get_beg_pos_base(file,size);
}
int rdbeif_read_node_info(File64& file,ssize64_t bpos,rdbeif_node_t& inf)
{
	char buf[256];
	char buf_end[16];
	if(bpos!=file.seek(bpos,SEEK_SET))
		return -1;
	if(EIF_HEAD_SIZE!=file.read(buf,EIF_HEAD_SIZE))
		return -1;
	if(0!=strncmp(buf,EIF_STX_BEG,8))
		return -1;
	int pos = 8;
	memcpy((char*)&inf.head_size,buf+pos,4);
	pos += 4;
	memcpy((char*)&inf.file_size,buf+pos,8);
	pos += 8;
	memcpy((char*)&inf.id,buf+pos,4);
	pos += 4;
	memcpy((char*)inf.name,buf+pos,128);
	pos += 128;
	assert(pos == EIF_HEAD_SIZE);
	assert(inf.head_size == EIF_HEAD_SIZE);

	file.seek(inf.head_size-EIF_HEAD_SIZE+inf.file_size,SEEK_CUR);
	if(8!=file.read(buf_end,8))
		return -1;
	size64_t len = *(size64_t*)buf_end;
	if(0!=strncmp(buf_end,EIF_STX_END,8) && len!=(inf.file_size+inf.head_size))
		return -1;
	inf.file_beg_offset = bpos + inf.head_size;
	inf.file_rpos = 0;
	return 0;
}
int rdbeif_write_node_info(File64& file,ssize64_t bpos,rdbeif_node_t& inf)
{
	char buf[256];
	int pos = 0;
	pos = 0;
	memcpy(buf+pos,EIF_STX_BEG,8);
	pos += 8;
	memcpy(buf+pos,&inf.head_size,4);
	pos += 4;
	memcpy(buf+pos,&inf.file_size,8);
	pos += 8;
	memcpy(buf+pos,&inf.id,4);
	pos += 4;
	memcpy(buf+pos,inf.name,128);
	pos += 128;
	assert(pos == EIF_HEAD_SIZE);
	assert(inf.head_size == EIF_HEAD_SIZE);
	file.seek(bpos,SEEK_SET);
	if(pos!=file.write(buf,pos))
		return -1;

	size64_t len = inf.file_size + inf.head_size;
	memcpy(buf,&len,8);
	file.seek(inf.file_size,SEEK_CUR);
	if(8!=file.write(buf,8))
		return -1;
	return 0;
}

int rdbeif_copy_file(File64& from,ssize64_t from_pos,File64& to,ssize64_t to_pos,size64_t size)
{
	size64_t write_size=0;
	char buf[1204];
	int len;
	if(from_pos!=from.seek(from_pos,SEEK_SET))
		return -1;
	if(to_pos!=to.seek(to_pos,SEEK_SET))
		return -1;

	while(write_size<size)
	{
		len = 1024;
		if(write_size+len>size)
			len = (int)(size-write_size);
		len = from.read(buf,len);
		if(len<=0) break;
		if(len != to.write(buf,len))
			break;
		write_size += len;
	}
	if(write_size==size)
		return 0;
	assert(false);
	return -1;
}
const char* rdbeif_get_name_by_path(const char* path)
{
	const char* p1 = strrchr(path,'\\');
	const char* p2 = strrchr(path,'/');
	if(p1)
	{
		if(p2)
		{
			if(p1>p2)
				return p1+1;
			return p2+1;
		}
		return p1+1;
	} 
	else if(p2)
	{
		return p2+1;
	}
	return path;
}
//*************************************************************************
//

rdbeif_handle_t* rdbeif_open(const char* cvipath,int mode)
{
	rdbeif_handle_t * h = new rdbeif_handle_t();
	h->file.open(cvipath,mode);
	if(!h->file.is_open())
	{
		delete h;
		return NULL;
	}
	strcpy(h->path,cvipath);
	h->file_type = ERDBFile64::get_filetype(cvipath);
	h->eif_beg_offset = rdbeif_get_beg_pos(cvipath);
	
	//获取所有节点信息
	rdbeif_node_t *node = new rdbeif_node_t();
	ssize64_t next_pos = h->eif_beg_offset;
	while(0==rdbeif_read_node_info(h->file,next_pos,*node))
	{
		h->nl.push_back(node);
		next_pos = node->file_beg_offset + node->file_size + 8;
		node = new rdbeif_node_t();
	}
	delete node;
	h->eif_end_offset = next_pos;
	return h;
}
int rdbeif_close(rdbeif_handle_t* h)
{
	h->file.close();
	h->eif_beg_offset = 0;
	h->eif_end_offset = 0;
	for(cyclist<rdbeif_node_t*>::iterator it=h->nl.begin();it!=h->nl.end();++it)
		delete (rdbeif_node_t*)(*it);
	h->nl.clear();
	delete h;
	return 0;
}
int rdbeif_zip_file(rdbeif_handle_t* h,const char* path,int id,bool bcover)
{
	const char* name = rdbeif_get_name_by_path(path);
	rdbeif_node_t *inf;
	for(cyclist<rdbeif_node_t*>::iterator it=h->nl.begin();it!=h->nl.end();++it)
	{
		if(0==stricmp(name,(*it)->name))
		{
			if(!bcover)
				return 1;
			else
			{
				//暂时先通过修改ID值来决定将原文件变成垃圾
				inf = *it;
				inf->id = -1;
				ssize64_t pos = inf->file_beg_offset - inf->head_size + 20;
				if(pos==h->file.seek(pos,SEEK_SET))
				{
					h->file.write((char*)&inf->id,4);
					h->file.flush();
				}
			}
		}
	}
	File64 file;
	if(0!=file.open(path,F64_READ))
		return -1;
	inf = new rdbeif_node_t();
	inf->head_size = EIF_HEAD_SIZE;
	inf->file_rpos = 0;
	inf->file_size = File64::get_file_size(path);
	inf->file_beg_offset = h->eif_end_offset + inf->head_size;
	inf->id = id;
	strcpy(inf->name,name);
	if(0==rdbeif_write_node_info(h->file,h->eif_end_offset,*inf))
	{
		if(0==rdbeif_copy_file(file,0,h->file,inf->file_beg_offset,inf->file_size))
		{
			h->nl.push_back(inf);
			h->eif_end_offset = inf->file_beg_offset + inf->file_size + 8;
			return 0;
		}
	}
	delete inf;
	return -1;
}
int rdbeif_unzip_all(rdbeif_handle_t* h,const char* dir)
{
	if(!h->file.is_open())
	{
		return -1;
	}
	char adir[256];
	char path[256];
	strcpy(adir,dir);
	size_t len = strlen(adir);
	if(adir[len-1]=='\\' || adir[len-1]=='/')
		adir[len-1] = '\0';

	rdbeif_node_t *node;
	size64_t curr_pos;
	//int n = 0;
	File64 file2;

	curr_pos = h->file.tell();
	for(cyclist<rdbeif_node_t*>::iterator it=h->nl.begin();it!=h->nl.end();++it)
	{
		node = *it;
		if(-1==node->id)
			continue;
		sprintf(path,"%s/%s",adir,node->name);
		if(0==file2.open(path,F64_RDWR|F64_TRUN))
		{
			//copy file
			if(0==rdbeif_copy_file(h->file,node->file_beg_offset,file2,0,node->file_size))
			{
				printf("# RDBEIF unzip [%s] ok! \n",path);
			}
			else
			{
				printf("# RDBEIF *** unzip [%s] fail! \n",path);
			}
			file2.close();
		}
	}
	h->file.seek(curr_pos,SEEK_SET);
	return 0;
}
int rdbeif_clean(const char* cvipath)
{
	size64_t eif_beg_offset = (size64_t)rdbeif_get_beg_pos(cvipath);
	size64_t size = File64::get_file_size(cvipath);
	if(eif_beg_offset<size)
	{
		return File64::resize(cvipath,eif_beg_offset);
	}
	return 0;
}

//********************************************************************************************
//eif单文件读访问接口
rdbeif_node_t* rdbeif_node_open(const char* rdbpath,const char* eif_name)
{
	rdbeif_handle_t* h = rdbeif_open(rdbpath,F64_READ);
	if(NULL==h)
		return NULL;
	rdbeif_node_t* node=NULL;
	for(cyclist<rdbeif_node_t*>::iterator it=h->nl.begin();it!=h->nl.end();++it)
	{
		if(0==stricmp(eif_name,(*it)->name) && -1!=(*it)->id)
			node = *it;
		else
			delete *it;
	}
	h->file.close();
	delete h;
	if(NULL==node)
		return NULL;
	if(0!=node->file.open(rdbpath,F64_READ))
	{
		delete node;
		return NULL;
	}
	node->file.seek(node->file_beg_offset+node->file_rpos,SEEK_SET);
	return node;
}
void rdbeif_node_close(rdbeif_node_t* h)
{
	h->file.close();
	delete h;
}
int rdbeif_node_seek(rdbeif_node_t* h,size64_t pos)
{
	if(pos>h->file_size)
		pos = h->file_size;
	if(pos==(size64_t)-1)
		pos = 0;
	h->file_rpos = pos;
	h->file.seek(h->file_beg_offset+h->file_rpos,SEEK_SET);
	return 0;
}
int rdbeif_node_read(rdbeif_node_t* h,char* buf,int size)
{
	if(size<=0)
		return 0;
	if(h->file_rpos+size > h->file_size)
		size = (int)(h->file_size - h->file_rpos);
	size = h->file.read(buf,size);
	if(size>0)
		h->file_rpos += size;
	return size;
}
int rdbeif_node_eof(rdbeif_node_t* h)
{
	if(h->file_rpos >= h->file_size)
		return 1;
	return 0;
}


