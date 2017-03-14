#pragma once

template<class TYPE>
class Singleton
{
public:
	Singleton(void){}
	virtual ~Singleton(void){}
private:
	static TYPE* _instance;

public:
	static TYPE* instance()
	{
		if(0==_instance)
		{
			_instance = new TYPE();
		}
		return _instance;
	}
	static void instance(TYPE *ins)
	{
		destroy();
		_instance = ins;
	}
	static void destroy()
	{
		if(_instance)
		{
			delete _instance;
			_instance = 0;
		}
	}

private:
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);

};
template<class TYPE> TYPE* Singleton<TYPE>::_instance = 0;


