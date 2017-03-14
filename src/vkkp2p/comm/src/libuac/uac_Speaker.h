#pragma once

#include <assert.h>
#include <string.h>

namespace UAC
{
template<typename Listener>
class Speaker 
{
public:
	Speaker(int size)
	{
		_size = size<1?1:size;
		_listeners = new Listener*[_size];
		_tmp_listeners = new Listener*[_size];
		_cursor = 0;
		_tmp_cursor = 0;
	}
	virtual ~Speaker() 
	{ 
		assert(0==_cursor);
		delete[]  _listeners;
		delete[]  _tmp_listeners;
	}

public:
	int listener_num() const { return _cursor;}
	bool add_listener(Listener* listener)
	{
		if(-1!=find(listener))
			return true;
		if(_cursor>=_size)
			return false;
		_listeners[_cursor++] = listener;
		return true;
	}
	void remove_listener(Listener* listener)
	{
		int i = find(listener);
		if(-1==i)
			return;
		--_cursor;
		_listeners[i] = _listeners[_cursor];
		_listeners[_cursor] = 0;
	}

protected:
	int find(Listener* listener)
	{
		for(int i=0;i<_cursor;++i)
		{
			if(_listeners[i]==listener)
				return i;
		}
		return -1;
	}

	//*************************************************
		//for(i=0;i<_cursor;++i)			
		//	_tmp_listeners[i] = _listeners[i]; 
#define _BEGIN_FIRE_ROOT_  int i = 0;	\
		_tmp_cursor = _cursor;			\
		memcpy(_tmp_listeners,_listeners,sizeof(Listener*)*_cursor);\
		for(i=0;i<_tmp_cursor;++i){	

#define _END_FIRE_ROOT_  }

	template<typename T0>
	void fire(T0 type) throw()
	{
		_BEGIN_FIRE_ROOT_
		_tmp_listeners[i]->on(type);
		_END_FIRE_ROOT_
	}

	template<typename T0, class T1>
	void fire(T0 type,const T1& p1) throw() 
	{
		_BEGIN_FIRE_ROOT_
		_tmp_listeners[i]->on(type,p1);
		_END_FIRE_ROOT_
	}

	template<typename T0, class T1, class T2>
	void fire(T0 type,const T1& p1,const T2& p2) throw() 
	{
		_BEGIN_FIRE_ROOT_
		_tmp_listeners[i]->on(type,p1,p2);
		_END_FIRE_ROOT_
	}

	template<typename T0, class T1, class T2, class T3>
	void fire(T0 type,const T1& p1,const T2& p2,const T3& p3) throw() 
	{
		_BEGIN_FIRE_ROOT_
		_tmp_listeners[i]->on(type,p1,p2,p3);
		_END_FIRE_ROOT_
	}

	template<typename T0, class T1, class T2, class T3, class T4>
	void fire(T0 type,const T1& p1,const T2& p2,const T3& p3,const T4& p4) throw()
	{
		_BEGIN_FIRE_ROOT_
		_tmp_listeners[i]->on(type,p1,p2,p3,p4);
		_END_FIRE_ROOT_
	}

	
protected:
	int _size;
	Listener **_listeners;
	Listener **_tmp_listeners;
	int _cursor;
	int _tmp_cursor;
};

}
