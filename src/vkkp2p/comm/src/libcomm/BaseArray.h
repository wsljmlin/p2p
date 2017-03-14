#pragma once
#include <string.h>
#include <assert.h>

template<typename T>
class BaseArray
{
public:
	T *data;
	char *state;
	int size;
	int count;
	int cursor;
public:
	BaseArray(void) :data(0),state(0),size(0),count(0),cursor(0){ }
	~BaseArray(void) {resize(0);}
	int resize(int asize)
	{
		if(size)
		{
			delete[] data;
			data = 0;
			delete[] state;
			state = 0;
			size = 0;
			count = 0;
			cursor = 0;
		}

		if(asize)
		{
			data = new T[asize];
			if(!data)
				return -1;
			state = new char[asize];
			if(!state)
			{
				delete[] data;
				data = 0;
				return -1;
			}
			memset(state,0,asize);
			size = asize;
		}
		return 0;
	}

	int allot()
	{
		if(count>=size)
			return -1;
		int n = -1,i=0;
		for(i=cursor;i<size;++i)
		{
			if(0==state[i])
			{
				n = i;
				break;
			}
		}
		if(-1==n)
		{
			for(i=0;i<cursor;++i)
			{
				if(0==state[i])
				{
					n = i;
					break;
				}
			}
		}
		if(-1==n)
		{
			//assert(0);
			return -1;
		}

		count++;
		state[n] = 1;
		cursor = n+1;
		if(cursor>=size)
			cursor=0;
		return n;
	}
	int allot2()
	{
		if(count>=size)
			return -1;
		int i=0;
		for(i=0;i<size;++i)
		{
			if(0==state[i])
				break;
		}
		if(i==size)
			return -1;
		count++;
		state[i] = 1;
		if(cursor<i)
			cursor=i;
		return i;
	}
	void free(int i)
	{
		assert(i<size);
		state[i] = 0;
		count--;
	}
	void free2(int i)
	{
		assert(i<size);
		state[i] = 0;
		count--;
		while(cursor>0&&0==state[cursor])
			cursor--;
	}
	T& operator [](int i)
	{
		assert(i<size);
		return data[i];
	}

};


#define BASEARRAY2_SUBLEN 1024
template<typename T>
class BaseArray2
{
public:
	T **data;
	char *state;
	int size;
	int count;
	int cursor;
public:
	BaseArray2(void) :data(0),state(0),size(0),count(0),cursor(0){ }
	~BaseArray2(void) {resize(0);}
	int resize(int asize)
	{
		if(size)
		{
			int len = (size-1)/BASEARRAY2_SUBLEN + 1;
			for(int i=0;i<len;++i)
				delete[] data[i];
			delete[] data;
			data = 0;
			delete[] state;
			state = 0;
			size = 0;
			count = 0;
			cursor = 0;
		}

		if(asize)
		{
			size = asize;
			int len = (size-1)/BASEARRAY2_SUBLEN + 1;
			data = new T*[len];
			for(int i=0;i<len;++i)
				data[i] = new T[BASEARRAY2_SUBLEN];
			len = (size-1)/8 + 1;
			state = new char[len];
			memset(state,0,len);
		}
		return 0;
	}

	int allot()
	{
		if(count>=size)
			return -1;
		int n = -1,i=0;
		for(i=cursor;i<size;++i)
		{
			if(0==(state[i/8]&(1<<(i%8))))
			{
				n = i;
				break;
			}
		}
		if(-1==n)
		{
			for(i=0;i<cursor;++i)
			{
				if(0==(state[i/8]&(1<<(i%8))))
				{
					n = i;
					break;
				}
			}
		}
		if(-1==n)
		{
			//assert(0);
			return -1;
		}

		count++;
		state[n/8] |= 1<<(n%8);
		cursor = n+1;
		if(cursor>=size)
			cursor=0;
		return n;
	}
	void free(int i)
	{
		assert(i<size);
		state[i/8] &= ~(1<<(i%8));
		count--;
	}
	T& operator [](int i)
	{
		assert(i<size);
		return data[i/BASEARRAY2_SUBLEN][i%BASEARRAY2_SUBLEN];
	}

};

