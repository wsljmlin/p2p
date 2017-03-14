#include "RDBFile64.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#pragma warning(disable:4996)
#else
#define stricmp strcasecmp
#endif



//********************************************************

RDBFile64::RDBFile64(void)
: m_file_size(0)
, m_file_write_size(0)
, m_block_size(0)
, m_head_size(0)
, m_file_write_blocks(0)
, m_pos(0)
, m_logbs(NULL)
, m_logbs_size(0)
{
}

RDBFile64::~RDBFile64(void)
{
	close();
}

void RDBFile64::reset()
{
	m_file_size = 0;
	m_file_write_size = 0;
	m_block_size = 0;
	m_head_size = 0;
	m_file_write_blocks = 0;
	m_pos = 0;
	if(m_logbs)
	{
		delete[] m_logbs;
		m_logbs = NULL;
	}
	m_logbs_size = 0;

}
int RDBFile64::create_head()
{
	if(0!=check_resize_logbs_ok(1000)) //��ʼ��֧��2G���Ҵ�С
		return -1;
	m_block_size = RDB_BLOCK_SIZE;
	m_head_size =  RDB_HEAD_SIZE;
	m_file_size = 0;
	m_file_write_size = 0;
	m_file_write_blocks = 0;
	//m_mapbi.clear();
	m_pos = 0;
	char buf[128];
	int pos = 0;
	unsigned int edian = RDB_ENDIAN;
	int ver = 1;
	memcpy(buf+pos,RDB_STX,40);
	pos += 40;
	memcpy(buf+pos,(char*)&edian,4);
	pos += 4;
	memcpy(buf+pos,(char*)&ver,4);
	pos += 4;
	memcpy(buf+pos,(char*)&m_head_size,4);
	pos += 4;
	memcpy(buf+pos,(char*)&m_file_size,8);
	pos += 8;
	memcpy(buf+pos,(char*)&m_file_write_size,8);
	pos += 8;
	memcpy(buf+pos,(char*)&m_file_write_blocks,4);
	pos += 4;
	memcpy(buf+pos,(char*)&m_block_size,4);
	pos += 4;
	if(pos!=76)
	{
		assert(0);
		return -1;
	}
	_file.seek(0,SEEK_SET);
	if(pos!=_file.write(buf,pos))
		return -1;
	_file.seek(m_head_size-1,SEEK_SET);
	_file.write((char*)&ver,1);
	return 0;
}
int RDBFile64::load_head()
{
	unsigned int endian = 0;
	int ver=0;
	size64_t fsize = _file.seek(0,SEEK_END);
	if((size64_t)-1==fsize || fsize < 1024)  //������Ҫ����1K
		return -1;

	char buf[2048];
	int pos = 0;
	_file.seek(0,SEEK_SET);
	if(48!=_file.read(buf,48))
		return -1;
	pos = 40;
	memcpy((char*)&endian,buf+pos,4);
	pos += 4;
	memcpy((char*)&ver,buf+pos,4);
	//ע��:�ڴ��ж�endian��û�и�ԭ��д������෴,���෴ʵ������Ҫ������,��ʱ���ﲻ֧����ô���

	if(0!=strncmp(buf,RDB_STX,40))
		return -1;
	if(1==ver)
	{
		//ֻ֧�ְ汾1
		if(28!=_file.read(buf,28))
			return -1;
		pos = 0;
		memcpy((char*)&m_head_size,buf+pos,4);
		pos += 4;
		memcpy((char*)&m_file_size,buf+pos,8);
		pos += 8;
		memcpy((char*)&m_file_write_size,buf+pos,8);
		pos += 8;
		memcpy((char*)&m_file_write_blocks,buf+pos,4);
		pos += 4;
		memcpy((char*)&m_block_size,buf+pos,4);
		pos += 4;
		if(76!=_file.tell())
		{
			assert(0);
			return -1;
		}
		//���һ��block_size
		if(m_block_size<100)
			return -1;
		//ÿ��2M�Ļ�,100000����200G����,100000���800K�����ֽ�
		assert(m_file_write_blocks<=100000);
		if(m_file_write_blocks>100000)
			return -1;

		int bsize = 0;
		unsigned int logic_index;
		size64_t file_write_size = 0;
		size64_t file_size = 0 , size_tmp=0;

		int blocks = m_file_write_blocks;
		int read_len=0;
		int n=0,inc=0;
		unsigned int max_index = (int)(m_file_size/m_block_size);
		if(0!=check_resize_logbs_ok( max_index<1000?1000:(max_index+10) )) //��ʼ��2G����
			return -1;
		while(blocks>0)
		{
			n = blocks>100?100:blocks;
			read_len = n * sizeof(BlockInfo); //100��Ϊ800���ֽڣ�����10K
			if(read_len != _file.read(buf,read_len))
				return -1;
			for(int i=0;i<n;++i)
			{
				memcpy((char*)&logic_index,buf+i*8,4);
				memcpy((char*)&bsize,buf+i*8+4,4);

				if(bsize<0) 
				{
					assert(0);
					bsize = 0;
				}
				if(logic_index > max_index)
				{
					assert(0);
					return -1;
				}
				//���벻�ظ�
				if(m_logbs[logic_index].index!=-1)
				{
					assert(0);
					return -1;
				}
				m_logbs[logic_index].index = inc++;
				m_logbs[logic_index].size = bsize;

				//m_mapbi[logic_index] = bi;
				file_write_size += bsize;
				size_tmp = logic_index * m_block_size + bsize;
				if(file_size < size_tmp)
					file_size = size_tmp;
			}
			blocks -= n;
		}
		
		//���һ����д�ļ���С
		assert(file_write_size==m_file_write_size);
		if(m_file_write_size != file_write_size)
		{
			m_file_write_size = file_write_size;
			return -1;
		}
		//���һ���ļ��ܴ�С,���ڿ��ڲ�һ������д������file_size��һ����m_file_size
	}
	else
	{
		assert(0);
		return -1;
	}
	return 0;
}
int RDBFile64::check_resize_logbs_ok(int blocks)
{
	if(blocks>m_logbs_size)
	{
		BlockInfo *pb = new BlockInfo[blocks];
		if(!pb)
			return -1;
		int i = 0;
		for(;i<m_logbs_size;++i)
		{
			pb[i].index = m_logbs[i].index;
			pb[i].size = m_logbs[i].size;
		}
		for(;i<blocks;++i)
		{
			pb[i].index = -1;
			pb[i].size = 0;
		}
		BlockInfo *tmp = m_logbs;
		m_logbs = pb;
		m_logbs_size = blocks;
		if(tmp)
			delete[] tmp;
	}
	return 0;
}
int RDBFile64::open(const char* path,int mode)
{
	if(0!=_file.open(path,mode))
		return -1;
	reset();
	if(mode & F64_TRUN)
	{
		if(0!=create_head())
		{
			this->close();
			assert(0);
			File64::remove_file(path);
			return -1;
		}
	}
	else
	{
		if(0!=load_head())
		{
			this->close();
			return -1;
		}
	}
	return 0;
}
int RDBFile64::close()
{
	_file.close();
	reset();
	return 0;
}
ssize64_t RDBFile64::seek(ssize64_t distance,int smode)
{
	if(!_file.is_open())
	{
		assert(0);
		return -1;
	}
	if(SEEK_SET==smode)
		m_pos = distance;
	else if(SEEK_END==smode)
		m_pos = m_file_size + distance;
	else if(SEEK_CUR==smode)
		m_pos += distance;
	else
	{
		assert(0);
		return -1;
	}
	return m_pos;
}
int RDBFile64::write(char *buf,int len)
{
	if(!_file.is_open())
	{
		assert(0);
		return 0;
	}
	if(len<=0)
		return 0;
	char *ptr = (char*)buf;
	int size = 0;
	int tmp = 0;
	int logic_index = 0;
	int logic_index_pos = 0;
	int write_len = 0;
	ssize64_t phy_pos = 0;
	BlockInfo *pbi = NULL;
	while(len>0)
	{
		//����m_block_size һ���������0
		logic_index = (int)(m_pos / m_block_size);
		logic_index_pos = (int)(m_pos % m_block_size);

		if(logic_index >= m_logbs_size)
		{
			//������鲻��,����1G����
			if(0!=check_resize_logbs_ok(logic_index+1000))
				return size;
		}
		pbi = &m_logbs[logic_index];
		if(-1==pbi->index)
		{
			pbi->index = m_file_write_blocks++;
			pbi->size = 0;
		}

		//����ڲ�����P2P�Ĺ����
		write_len = m_block_size - logic_index_pos;
		if(write_len > len)
			write_len = len;

		phy_pos = pbi->index * (ssize64_t)m_block_size + logic_index_pos + m_head_size;
		if(phy_pos != _file.seek(phy_pos,SEEK_SET))
		{
			assert(0);
			return size;
		}
		tmp = _file.write(ptr,write_len);
		if(tmp <= 0)
			return size;

		pbi->size += tmp;
		ptr += tmp;
		len -= tmp;
		m_pos += tmp;
		m_file_write_size += tmp;
		if(m_file_size<m_pos)
			m_file_size = m_pos;
		size += tmp;

		//save block info
		phy_pos = pbi->index * 8 + 76;        //��������������ַȥ
		_file.seek(phy_pos,SEEK_SET);
		_file.write((char*)&logic_index,4);   //д���߼����
		_file.write((char*)&pbi->size,4);
		_file.seek(52,SEEK_SET);   //
		_file.write((char*)&m_file_size,8);
		_file.write((char*)&m_file_write_size,8);
		_file.write((char*)&m_file_write_blocks,4);

	}
	return size;
}
int RDBFile64::read(char *buf,int len)
{
	if(!_file.is_open())
	{
		assert(0);
		return -1;
	}

	if(len<=0)
		return 0;
	if(m_pos>=m_file_size)
		return 0;
	char *ptr = buf;
	int tmp = 0;
	int size = 0;
	int logic_index = 0;
	int logic_index_pos = 0;
	int read_len = 0;
	ssize64_t phy_pos = 0;
	BlockInfo *pbi = NULL;
	while(len>0 && m_pos<m_file_size)
	{
		if(m_pos >= m_file_size)
			return size;
		//����m_block_size һ���������0
		logic_index = (int)(m_pos / m_block_size);
		logic_index_pos = (int)(m_pos % m_block_size);

		//
		read_len = m_block_size - logic_index_pos;
		if(read_len > len)
			read_len = len;
		//���ܶ���logic�ļ���С�������ڴ˿���
		if(read_len + m_pos > m_file_size)
			read_len = (int)(m_file_size - m_pos);

		pbi = &m_logbs[logic_index];
		if(-1==pbi->index)
		{
			assert(0);
			return size;
		}

		phy_pos = pbi->index * (ssize64_t)m_block_size + logic_index_pos + m_head_size;
		if(phy_pos!=_file.seek(phy_pos,SEEK_SET))
			return size;
		tmp = _file.read(ptr,read_len);
		if(tmp <= 0)
			return size;

		size += tmp;
		ptr += tmp;
		len -= tmp;
		m_pos += tmp;
	}
	return size;
}
int RDBFile64::flush()
{
	return _file.flush();// flushӰ���д���ܼ��������ݼ����׿�Ƭ��
}
ssize64_t RDBFile64::tell()
{
	return m_pos;
}
int RDBFile64::resize(ssize64_t size)
{
	if(!_file.is_open())
	{
		return -1;
	}
	if(size>0)
	{
		return BFile64::resize(m_head_size + size);
	}
	return 0;
}

//const char* get_name_by_path(const char* path)
//{
//	const char* p1 = strrchr(path,'\\');
//	const char* p2 = strrchr(path,'/');
//	if(p1)
//	{
//		if(p2)
//		{
//			if(p1>p2)
//				return p1+1;
//			return p2+1;
//		}
//		return p1+1;
//	} 
//	else if(p2)
//	{
//		return p2+1;
//	}
//	return path;
//}
//
////��չ��Ϣ�ļ���ؽӿ�--Extended information file
//int RDBFile64::eif_zip_file(const char* path,int id)
//{
//	if(!_file.is_open())
//	{
//		return -1;
//	}
//	//��¼��ǰλ��curr_pos
//	//����ZIP��ʼλ��zip_begin_pos
//	//SEEK����zip_begin_pos
//	//ʧ�ܷ���
//	//�ɹ�:
//	//����־���ļ���С
//	//�����ǰ��־Ϊ�ļ�,����SEEK��һ����ʼλ�ö���־
//	//��ǰ��Ϊ�ļ�,д��ͷ�������ļ�
//	//fflush
//	//��ָ���ƻ�curr_pos
//	RDBEIFNode_t inf;
//	ssize64_t zip_begin_pos;
//	ssize64_t curr_pos;
//	File64 file2;
//	char buf[256];
//	const char* name = get_name_by_path(path);
//	int pos;
//
//	if(0!=file2.open(path,F64_READ))
//		return -1;
//
//	curr_pos = _file.tell();
//	zip_begin_pos = (ssize64_t)m_file_write_blocks*m_block_size + m_head_size;
//	while(0==eif_get_node(_file,zip_begin_pos,inf))
//	{
//		if(0==stricmp(name,inf.name))
//			return 1; //ѹ���ļ��Ѿ�����
//		zip_begin_pos += (inf.head_size + inf.file_size + 8);
//	}
//	if(zip_begin_pos!=_file.seek(zip_begin_pos,SEEK_SET))
//		return -1;
//
//	inf.head_size = EIF_HEAD_SIZE;
//	inf.id = id;
//	memset(inf.name,0,128);
//	strcpy(inf.name,name);
//	inf.file_size = File64::get_file_size(path);
//	inf.begin_pos = inf.head_size + zip_begin_pos;
//
//	pos = 0;
//	memcpy(buf+pos,EIF_STX_BEG,8);
//	pos += 8;
//	memcpy(buf+pos,&inf.head_size,4);
//	pos += 4;
//	memcpy(buf+pos,&inf.file_size,8);
//	pos += 8;
//	memcpy(buf+pos,&inf.id,4);
//	pos += 4;
//	memcpy(buf+pos,inf.name,128);
//	pos += 128;
//	assert(pos == EIF_HEAD_SIZE);
//	assert(inf.head_size == EIF_HEAD_SIZE);
//	if(pos!=_file.write(buf,pos))
//		return -1;
//
//	//copy file
//	eif_copy_file(file2,0,_file,inf.begin_pos,inf.file_size);
//	
//	zip_begin_pos += (inf.head_size + inf.file_size);
//	if(zip_begin_pos!=_file.seek(zip_begin_pos,SEEK_SET))
//		return -1;
//	if(8!=_file.write(EIF_STX_END,8))
//		return -1;
//	file2.close();
//	_file.flush();
//	_file.seek(curr_pos,SEEK_SET);
//	return 0;
//}
//int RDBFile64::eif_get_zip_info(RDBEIFInfo_t& zi)
//{
//	if(!_file.is_open())
//	{
//		return -1;
//	}
//	RDBEIFNode_t inf;
//	size64_t zip_begin_pos;
//	size64_t curr_pos;
//	//int n = 0;
//
//	curr_pos = _file.tell();
//	zi.rdb_file_size = m_file_size;
//	zi.rdb_block_size = m_block_size;
//	zi.all_file_num = 0;
//	zi.real_get_file_num = 0;
//	zip_begin_pos = (size64_t)m_file_write_blocks*m_block_size + m_head_size;
//	while(0==eif_get_node(_file,zip_begin_pos,inf))
//	{
//		zi.all_file_num++;
//		if(zi.real_get_file_num<zi.try_get_file_num)
//			zi.files[zi.real_get_file_num++] = inf;
//		zip_begin_pos += (inf.head_size + inf.file_size + 8);
//	}
//	
//	_file.seek(curr_pos,SEEK_SET);
//	return 0;
//}
//int RDBFile64::eif_get_zip_infoi(const char* name,RDBEIFNode_t& node)
//{
//	if(!_file.is_open())
//	{
//		return -1;
//	}
//	RDBEIFNode_t inf;
//	size64_t zip_begin_pos;
//	size64_t curr_pos;
//	int ret = -1;
//	//int n = 0;
//
//	curr_pos = _file.tell();
//	zip_begin_pos = (size64_t)m_file_write_blocks*m_block_size + m_head_size;
//	while(0==eif_get_node(_file,zip_begin_pos,inf))
//	{
//		if(0==strcmp(name,inf.name))
//		{
//			node = inf;
//			ret = 0;
//			break;
//		}
//		zip_begin_pos += (inf.head_size + inf.file_size + 8);
//	}
//	
//	_file.seek(curr_pos,SEEK_SET);
//	return ret;
//}
//int RDBFile64::eif_unzip_all_file(const char* dir)
//{
//	if(!_file.is_open())
//	{
//		return -1;
//	}
//	char adir[256];
//	char path[256];
//	strcpy(adir,dir);
//	size_t len = strlen(adir);
//	if(adir[len-1]=='\\' || adir[len-1]=='/')
//		adir[len-1] = '\0';
//
//	RDBEIFNode_t inf;
//	size64_t zip_begin_pos;
//	size64_t curr_pos;
//	//int n = 0;
//	File64 file2;
//
//	curr_pos = _file.tell();
//	zip_begin_pos = (size64_t)m_file_write_blocks*m_block_size + m_head_size;
//	while(0==eif_get_node(_file,zip_begin_pos,inf))
//	{
//		sprintf(path,"%s/%s",adir,inf.name);
//		if(0==file2.open(path,F64_RDWR|F64_TRUN))
//		{
//			//copy file
//			if(0==eif_copy_file(_file,inf.begin_pos,file2,0,inf.file_size))
//			{
//				printf("# RDB unzip [%s] ok! \n",path);
//			}
//			else
//			{
//				printf("# RDB *** unzip [%s] fail! \n",path);
//			}
//			file2.close();
//		}
//
//		zip_begin_pos += (inf.head_size + inf.file_size + 8);
//	}
//	
//	_file.seek(curr_pos,SEEK_SET);
//	return 0;
//}

//******************************************************
ERDBFile64::ERDBFile64(void)
: m_pfile(NULL)
, m_file_type(RDBF_UNKNOW)
{
}
ERDBFile64::~ERDBFile64(void)
{
	close();
}
int ERDBFile64::open(const char* path,int mode,int ftype/*=RDBF_AUTO*/)
{
	if(m_pfile)
	{
		assert(0);
		return -1;
	}
	if(RDBF_AUTO==ftype)
	{
		if(mode & F64_TRUN)
			ftype = RDBF_RDB;
		else
		{
			ftype = get_filetype(path);
		}
	}

	BFile64 *pfile = NULL;
	if(RDBF_BASE==ftype)
		pfile = new BFile64();
	else if(RDBF_RDB==ftype)
		pfile = new RDBFile64();
	else if(RDBF_RDBS==ftype)
	{
		//ֻ����ֻ��
		if(F64_READ!=mode)
			return -1;
		pfile = new RDBFile64Simple();
	}

	if(NULL==pfile)
		return -1;
	if(0==pfile->open(path,mode))
	{
		m_file_type = ftype;
		m_pfile = pfile;
		return 0;
	}
	else
	{
		delete pfile;
		return -1;
	}
}
int ERDBFile64::close()
{
	if(m_pfile)
	{
		m_pfile->close();
		delete m_pfile;
		m_pfile = NULL;
	}
	return 0;
}
ssize64_t ERDBFile64::seek(ssize64_t distance,int smode)
{
	if(m_pfile)
		return m_pfile->seek(distance,smode);
	assert(0);
	return 0;
}
int ERDBFile64::write(char *buf,int len)
{
	if(m_pfile)
		return m_pfile->write(buf,len);
	assert(0);
	return 0;
}
int ERDBFile64::read(char *buf,int len)
{
	if(m_pfile)
		return m_pfile->read(buf,len);
	assert(0);
	return 0;
}
int ERDBFile64::flush()
{
	if(m_pfile)
		return m_pfile->flush();
	//assert(0);
	return 0;
}
ssize64_t ERDBFile64::tell()
{
	if(m_pfile)
		return m_pfile->tell();
	assert(0);
	return 0;
}


size64_t ERDBFile64::get_filesize(const char* path)
{
	File64 file;
	if(0!=file.open(path,F64_READ))
		return 0;
	size64_t size = file.seek(0,SEEK_END);
	if((size64_t)-1==size || size < 1024)
		return size;

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
	//ע��:�ڴ��ж�endian��û�и�ԭ��д������෴,���෴ʵ������Ҫ������,��ʱ���ﲻ֧����ô���

	if(0==strncmp(buf,RDB_STX,40)||0==strncmp(buf,RDBS_STX,40))
	{
		if(1==ver)
		{
			//ֻ֧�ְ汾1
			file.seek(4,SEEK_CUR);
			file.read((char*)&size,8);
		}
		else
		{
			assert(0);
		}
	}

	return size;
}
int ERDBFile64::get_filetype(const char* path)
{
	File64 file;
	if(0!=file.open(path,F64_READ))
		return RDBF_UNKNOW;
	size64_t size = file.seek(0,SEEK_END);
	if((size64_t)-1==size || size < 1024)
		return RDBF_BASE;

	char buf[128];
	int endian = 0;
	int ver = 0;
	int pos = 0;
	file.seek(0,SEEK_SET);
	if(48!=file.read(buf,48))
		return RDBF_BASE;
	pos = 40;
	memcpy((char*)&endian,buf+pos,4);
	pos += 4;
	memcpy((char*)&ver,buf+pos,4);
	//ע��:�ڴ��ж�endian��û�и�ԭ��д������෴,���෴ʵ������Ҫ������,��ʱ���ﲻ֧����ô���

	if(0==strncmp(buf,RDB_STX,40)&&1==ver)
		return RDBF_RDB;
	if(0==strncmp(buf,RDBS_STX,40)&&1==ver)
		return RDBF_RDBS;
	return RDBF_BASE;
}


