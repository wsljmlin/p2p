#pragma once
#include "ntypes.h"
#include <assert.h>

enum {SS_LITTLE_ENDIAN=0,SS_BIG_ENDIAN=1,SS_HOST_ENDIAN=2};
class SerialStream
{
public:
	SerialStream(char endian=SS_LITTLE_ENDIAN);
	SerialStream(int buflen,char endian=SS_LITTLE_ENDIAN);
	SerialStream(char* buf,int buflen,int datalen=0,char endian=SS_LITTLE_ENDIAN);
	virtual ~SerialStream(void);
private:
	static char __localEndianType;
public:
	static int localLittleEndian();
	static void swap(void *arr,int len);

	//h:host endian; b:big endian; l:little endian ;
	static void htol(void *arr,int len);
	static void ltoh(void *arr,int len);
	static void htob(void *arr,int len);
	static void btoh(void *arr,int len);

	static short htol16(short val);  
	static short ltoh16(short val);  
	static short htob16(short val); 
	static short btoh16(short val); 

	static sint32 htol32(sint32 val); 
	static sint32 ltoh32(sint32 val); 
	static sint32 htob32(sint32 val); 
	static sint32 btoh32(sint32 val);  

	static sint64 htol64(sint64 val); 
	static sint64 ltoh64(sint64 val); 
	static sint64 htob64(sint64 val); 
	static sint64 btoh64(sint64 val);  

public:
	int ok() const { return m_state;}
	char* buffer() const { return m_buf;}
	char* read_ptr() const { return m_buf+m_rpos; }
	char* write_ptr() const { return m_buf+m_wpos; }
	int buffer_size() const { return m_size;}
	int length() const { return m_wpos-m_rpos; }
	int tellr() const {return m_rpos;}
	int tellw() const {return m_wpos;}
	void zero_rw() {m_rpos=m_wpos=0;}
	int seekr(int pos);
	int seekw(int pos);
	int skipr(int len);
	int skipw(int len);
	int read(void *arr,int len);
	int write(const void *arr,int len);
	int set_memery(int pos,void *arr,int len);
	int get_memery(int pos,void *arr,int len);

	template<typename T>
	int read_array(T *arr,int len)
	{
		for(int i=0;i<len;++i)
			(*this) >> arr[i];
		return m_state;
	}

	template<typename T>
	int write_array(T *arr,int len)
	{
		for(int i=0;i<len;++i)
			(*this) << arr[i];
		return m_state;
	}

	int read_string(char* str,int maxsize);
	int write_string(const char* str);

	int operator >> (char& val);
	int operator >> (uchar& val);
	int operator >> (short& val);
	int operator >> (ushort& val);
	int operator >> (sint32& val);
	int operator >> (uint32& val);
	int operator >> (sint64& val);
	int operator >> (uint64& val);

	int operator << (char val);
	int operator << (uchar val);
	int operator << (short val);
	int operator << (ushort val);
	int operator << (sint32 val);
	int operator << (uint32 val);
	int operator << (sint64 val);
	int operator << (uint64 val);

protected:
	void htomy(void *arr,int len);
	void mytoh(void *arr,int len);
	int check_resize(int more);

public:
	int attach(char* buf,int buflen,int datalen=0);
	void reset();
protected:
	char m_endian;
	char* m_buf;
	int m_size;
	int m_state;
	int m_rpos;
	int m_wpos;
	bool m_mynew;
};

int operator << (SerialStream& ss, const puid_t& inf);
int operator >> (SerialStream& ss, puid_t& inf);
int operator << (SerialStream& ss, const fhash_t& inf);
int operator >> (SerialStream& ss, fhash_t& inf);

#define  PTL_ENDIAN_TYPE      SS_LITTLE_ENDIAN
class PTLStream : public SerialStream
{
public:
	PTLStream() : SerialStream((char)PTL_ENDIAN_TYPE) {}
	PTLStream(int buflen) : SerialStream(buflen,(char)PTL_ENDIAN_TYPE) {}
	PTLStream(char* buf,int buflen,int datalen=0) : SerialStream(buf,buflen,datalen,(char)PTL_ENDIAN_TYPE) {}
	int fitsize32(int pos)
	{
		//pos位置打上32位包大小
		assert(m_wpos >= (pos+4));
		if(m_wpos >= (pos+4))
		{
			int tmp = m_wpos;
			htomy(&tmp,4);
			set_memery(pos,&tmp,4);
			return 0;
		}
		m_state=-1;
		return m_state;
	}
};

