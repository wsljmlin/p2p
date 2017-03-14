#include "SerialStream.h"
#include <string.h>

#define SS_OVERFLOW_READ_N_RETURN(n)    assert(m_rpos+n <= m_wpos);				  \
										if(m_rpos+n > m_wpos){ m_state=-1; return m_state;} 


#define SS_OVERFLOW_WRITE_N_RETURN(n)   if(-1==check_resize(n)) return m_state;


char SerialStream::__localEndianType=99;
SerialStream::SerialStream(char endian/*=SS_LITTLE_ENDIAN*/)
{
	//m_size = 4;
	//m_buf = new char[m_size];
	m_size = 0;
	m_buf = NULL;
	m_mynew = true;
	m_rpos = 0;
	m_wpos = 0;
	m_state = 0;
	m_endian = endian;
}
SerialStream::SerialStream(int buflen,char endian/*=SS_LITTLE_ENDIAN*/)
{
	assert(buflen>0);
	m_size = buflen;
	m_buf = new char[m_size];
	m_mynew = true;
	m_rpos = 0;
	m_wpos = 0;
	m_state = 0;
	m_endian = endian;
}
SerialStream::SerialStream(char* buf,int buflen,int datalen/*=0*/,char endian/*=SS_LITTLE_ENDIAN*/)
{
	assert(buf && datalen>=0 && buflen >= datalen);
	m_buf = buf;
	m_size = buflen;
	m_rpos = 0;
	m_wpos = datalen;
	m_state = 0;
	m_endian = endian;
	m_mynew = false;
}
SerialStream::~SerialStream(void)
{
	if(m_mynew && m_buf)
		delete[] m_buf;
}

int SerialStream::localLittleEndian()
{
	if(SS_LITTLE_ENDIAN ==__localEndianType)
		return 1;
	else if(SS_BIG_ENDIAN == __localEndianType)
		return 0;
	else
	{
		unsigned short i = (unsigned short)0x9911;
		if(*(uchar*)&i == 0x11)
		{
			__localEndianType = SS_LITTLE_ENDIAN;
			return 1;
		}
		else
		{
			__localEndianType = SS_BIG_ENDIAN;
			return 0;
		}
	}
}
void SerialStream::swap(void *arr,int len)
{
	assert(len>1);
	char *ptr = (char*)arr;
	char tmp = 0;
	for(int i=0;i<len/2;++i)
	{
		tmp = ptr[i];
		ptr[i] = ptr[len-1-i];
		ptr[len-1-i] = tmp;
	}
}

//h:host endian; b:big endian; l:little endian ;
void SerialStream::htol(void *arr,int len)
{
	if(!localLittleEndian()) 
		swap(arr,len);
}
void SerialStream::ltoh(void *arr,int len)
{
	if(!localLittleEndian()) 
		swap(arr,len);
}
void SerialStream::htob(void *arr,int len)
{
	if(localLittleEndian()) 
		swap(arr,len);
}
void SerialStream::btoh(void *arr,int len)
{
	if(localLittleEndian()) 
		swap(arr,len);
}

short SerialStream::htol16(short val)
{
	htol(&val,sizeof(short));
	return val;
}  
short SerialStream::ltoh16(short val)
{
	ltoh(&val,sizeof(short));
	return val;
}  
short SerialStream::htob16(short val)
{
	htob(&val,sizeof(short));
	return val;
} 
short SerialStream::btoh16(short val)
{
	btoh(&val,sizeof(short));
	return val;
} 

sint32 SerialStream::htol32(sint32 val)
{
	htol(&val,sizeof(sint32));
	return val;
}
sint32 SerialStream::ltoh32(sint32 val)
{
	ltoh(&val,sizeof(sint32));
	return val;
}
sint32 SerialStream::htob32(sint32 val)
{
	htob(&val,sizeof(sint32));
	return val;
}
sint32 SerialStream::btoh32(sint32 val)
{
	btoh(&val,sizeof(sint32));
	return val;
}  

sint64 SerialStream::htol64(sint64 val)
{
	htol(&val,sizeof(sint64));
	return val;
}
sint64 SerialStream::ltoh64(sint64 val)
{
	ltoh(&val,sizeof(sint64));
	return val;
}
sint64 SerialStream::htob64(sint64 val)
{
	htob(&val,sizeof(sint64));
	return val;
} 
sint64 SerialStream::btoh64(sint64 val)
{
	btoh(&val,sizeof(sint64));
	return val;
} 


int SerialStream::seekr(int pos)
{
	if(0!=m_state)
		return m_state;
	if(pos<0 || pos>m_wpos)
		return -1;
	m_rpos = pos;
	return m_state;
}
int SerialStream::seekw(int pos)
{
	if(0!=m_state)
		return m_state;
	if(pos<m_rpos || pos>m_size)
		return -1;
	m_wpos = pos;
	return m_state;
}
int SerialStream::skipr(int len)
{
	SS_OVERFLOW_READ_N_RETURN(len)
	m_rpos += len;
	return m_state;
}
int SerialStream::skipw(int len)
{
	SS_OVERFLOW_WRITE_N_RETURN(len)
	m_wpos += len;
	return m_state;
}
int SerialStream::read(void *arr,int len)
{
	SS_OVERFLOW_READ_N_RETURN(len)
	memcpy(arr,m_buf+m_rpos,len);
	m_rpos += len;
	return m_state;
}
int SerialStream::write(const void *arr,int len)
{
	SS_OVERFLOW_WRITE_N_RETURN(len)
	memcpy(m_buf+m_wpos,arr,len);
	m_wpos += len;
	return m_state;
}
int SerialStream::set_memery(int pos,void *arr,int len)
{
	if(pos+len>m_size)
		return -1;
	memcpy(m_buf+pos,arr,len);
	return 0;
}
int SerialStream::get_memery(int pos,void *arr,int len)
{
	if(pos+len>m_size)
		return -1;
	memcpy(arr,m_buf+pos,len);
	return 0;
}

int SerialStream::read_string(char* str,int maxsize)
{
	SS_OVERFLOW_READ_N_RETURN(4)
	uint32 n = 0;
	*this >> n;
	if(n==0)
	{
		str[0] = '\0';
		return m_state;
	}
	SS_OVERFLOW_READ_N_RETURN((int)n)
	if(n>=(uint32)maxsize)
	{
		skipr(n);
		str[0] = '\0';
		return m_state;
	}
	this->read(str,n);
	str[n] = '\0';
	return m_state;
}
int SerialStream::write_string(const char* str)
{
	uint32 n = 0;
	if(str) n=(uint32)strlen(str);
	SS_OVERFLOW_WRITE_N_RETURN((int)(n+4))
	*this << n;
	if(n) this->write(str,n);
	return m_state;
}

int SerialStream::operator >> (char& val)
{
	SS_OVERFLOW_READ_N_RETURN(1)
	val = m_buf[m_rpos++];
	return m_state;
}
int SerialStream::operator >> (uchar& val)
{
	SS_OVERFLOW_READ_N_RETURN(1)
	val = (uchar)m_buf[m_rpos++];
	return m_state;
}
int SerialStream::operator >> (short& val)
{
	read(&val,sizeof(short));
	mytoh(&val,sizeof(short));
	return m_state;
}
int SerialStream::operator >> (ushort& val)
{
	read(&val,sizeof(ushort));
	mytoh(&val,sizeof(ushort));
	return m_state;
}
int SerialStream::operator >> (sint32& val)
{
	read(&val,sizeof(sint32));
	mytoh(&val,sizeof(sint32));
	return m_state;
}
int SerialStream::operator >> (uint32& val)
{
	read(&val,sizeof(uint32));
	mytoh(&val,sizeof(uint32));
	return m_state;
}
int SerialStream::operator >> (sint64& val)
{
	read(&val,sizeof(sint64));
	mytoh(&val,sizeof(sint64));
	return m_state;
}
int SerialStream::operator >> (uint64& val)
{
	read(&val,sizeof(uint64));
	mytoh(&val,sizeof(uint64));
	return m_state;
}


int SerialStream::operator << (char val)
{
	SS_OVERFLOW_WRITE_N_RETURN(1)
	m_buf[m_wpos]=val;
	m_wpos++;
	return m_state;
}
int SerialStream::operator << (uchar val)
{
	SS_OVERFLOW_WRITE_N_RETURN(1)
	m_buf[m_wpos]=(char)val;
	m_wpos++;
	return m_state;
}
int SerialStream::operator << (short val)
{
	htomy(&val,sizeof(short));
	write(&val,sizeof(short));
	return m_state;
}
int SerialStream::operator << (ushort val)
{
	htomy(&val,sizeof(ushort));
	write(&val,sizeof(ushort));
	return m_state;
}
int SerialStream::operator << (sint32 val)
{
	htomy(&val,sizeof(sint32));
	write(&val,sizeof(sint32));
	return m_state;
}
int SerialStream::operator << (uint32 val)
{
	htomy(&val,sizeof(uint32));
	write(&val,sizeof(uint32));
	return m_state;
}
int SerialStream::operator << (sint64 val)
{
	htomy(&val,sizeof(sint64));
	write(&val,sizeof(sint64));
	return m_state;
}
int SerialStream::operator << (uint64 val)
{
	htomy(&val,sizeof(uint64));
	write(&val,sizeof(uint64));
	return m_state;
}


void SerialStream::htomy(void *arr,int len)
{
	if(SS_LITTLE_ENDIAN==m_endian)
	{
		if(!localLittleEndian())
			swap(arr,len);
	}
	else if(SS_BIG_ENDIAN==m_endian)
	{
		if(localLittleEndian())
			swap(arr,len);
	}
}
void SerialStream::mytoh(void *arr,int len)
{
	htomy(arr,len);
}
int SerialStream::check_resize(int more)
{
	if(-1!=m_state && m_wpos+more > m_size)
	{
		if(m_mynew)
		{
			int newsize = m_size * 2;
			if (newsize < m_wpos+more)
				newsize = m_wpos+more;
			char* newbuf = new char[newsize];
			if(!newbuf)
			{
				m_state = -1;
				return -1;
			}
			if (m_wpos>0)
			{
				memcpy(newbuf,m_buf,m_wpos);
			}
			if(m_buf)
				delete[] m_buf;
			m_buf = newbuf;
			m_size = newsize;
		}
		else
		{
			m_state = -1;
		}
	}
	return m_state;
}


int SerialStream::attach(char* buf,int buflen,int datalen/*=0*/)
{
	if(!buf || !buflen)
		return -1;
	reset();
	m_buf = buf;
	m_size = buflen;
	m_wpos = datalen;
	return 0;
}

void SerialStream::reset()
{
	if(m_mynew && m_buf)
	{
		delete[] m_buf;
	}
	m_buf = 0;
	m_mynew = false;
	m_size = 0;
	m_rpos = 0;
	m_wpos = 0;
	m_state = 0;
}
int operator << (SerialStream& ss, const puid_t& inf)
{
	ss.write(inf,PUIDLEN);
	return ss.ok();
}
int operator >> (SerialStream& ss, puid_t& inf)
{
	ss.read(inf,PUIDLEN);
	return ss.ok();
}
int operator << (SerialStream& ss, const fhash_t& inf)
{
	ss.write(inf,HASHLEN);
	return ss.ok();
}
int operator >> (SerialStream& ss, fhash_t& inf)
{
	ss.read(inf,HASHLEN);
	return ss.ok();
}



