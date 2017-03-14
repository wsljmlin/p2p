#include "MemBlock.h"
#include <string.h>

MemBlock* MemBlock::allot(int size,int token/*=1*/)
{ 
	return MemBlockPoolSngl::instance()->get_block(size,token);
}
void MemBlock::free(int token/*=1*/) 
{
	MemBlockPoolSngl::instance()->put_block(this,token);
}
MemBlock* MemBlock::dup(int token/*=1*/)
{
	MemBlock* b = allot(buflen,token);
	if(b)
	{
		memcpy(b->buf,buf,datalen);
		b->datalen = datalen;
		b->datapos = datapos;
	}
	return b;
}
