#pragma once

#include <stdio.h>

namespace UAC
{
#define F32_READ        0x01
#define F32_WRITE       0x02
#define F32_RDWR        0x03
#define F32_TRUNC       0x04
#define F32_BINARY      0x08

#define FILE32_WRITE_RETURN(file,data,len,iret) if((int)len!=file.write(data,len)) return iret
#define FILE32_READ_RETURN(file,data,len,iret) if((int)len!=file.read(data,len)) return iret

class File32
{
public:
	File32(void);
	~File32(void);
public:
	int open(const char *_Filename,int _Mode);
	bool is_open() const;
	void close();

	int seek(int _Offset,int _Origin);

	int tell();
	void flush();
	bool eof();

	int write(const char *buf,int len);
	int read(char *buf,int len);
	char* getline(char *buf,int maxlen);
	bool write_n(const char *buf,int len);
	bool read_n(char *buf,int len);

public:
	static int remove_file(const char* path);
	static int rename_file(const char* from,const char* to);
private:
	FILE* m_fp;
};
}
