#pragma once
#include "uac_basetypes.h"

namespace UAC
{
template<typename T,unsigned int LEN=60>
class Speedometer
{
public:
	Speedometer(void){ reset(); }
	~Speedometer(void) {}
public:
	void reset()
	{
		m_total = 0;
		m_last = 0;
		for(unsigned int i=0;i<LEN;++i)
			m_data[i] = 0;
		m_cursor = 0;
		m_sec_amount = 0;
		m_last_tick = my_get_tick_count();
		m_limit_speed = 0;
	}

	void add(T n)
	{
		m_total += n;
		m_last += n;
	}

	void on_second()
	{
		++m_sec_amount;
		++m_cursor;
		if(m_cursor>=LEN) m_cursor = 0;
		m_data[m_cursor] = m_last;
		m_last = 0;
		m_last_tick = my_get_tick_count();
	}
	
	unsigned int get_sec_amount() const { return m_sec_amount;}
	T get_total() const { return m_total;}
	T get_last() const { return m_last;}

	T get_speed(int sec=-1) const
	{
		T speed = 0;
		
		//如果实际还没有收集那么长时间，只取已经统计时间的平均速度
		if(sec>(int)m_sec_amount) 
			sec = (int)m_sec_amount;
		if(sec>0)
		{
			int n = 0,i = 0;
			int cursor = m_cursor;
			T total = 0;
			for(i=cursor;i>=0 && n<sec;--i,++n)
				total += m_data[i];
			for(i=LEN-1;i>cursor && n<sec;--i,++n)
				total += m_data[i];
			speed = total/n;
		}
		else
		{
			//如果<=0，计算总的平均速度
			if(m_sec_amount>0)
				speed = m_total/m_sec_amount;
			else
				speed = m_total;
		}
		return speed;
	}

	void limit(unsigned int speed) { m_limit_speed = speed;}
	int limit_recv()
	{
		if(0==m_limit_speed)
			return -1;
		int n = 0;
		//n = (my_get_tick_count()-m_last_tick)*m_limit_speed / 1000 - (int)m_last;
		n = m_limit_speed - (int)m_last;
		if(n<0) n = 0;
		return n;
	}
	
private:
	unsigned int my_get_tick_count()
	{
		return GetTickCount();
	}
private:
	T m_total;
	T m_last;
	T m_data[LEN];
	unsigned int m_cursor;
	unsigned int m_sec_amount;
	unsigned int m_last_tick;
	unsigned int m_limit_speed;
};

template<typename T,unsigned int LEN>
class ArrRuler
{
public:
	ArrRuler(void){reset();}
	~ArrRuler(void){}

	void reset()
	{
		memset(&m_data,0,LEN*sizeof(T));
		m_cursor = 0;
		m_amount = 0;
	}
	void step(T n)
	{
		++m_amount;
		++m_cursor;
		if(m_cursor>=LEN) m_cursor = 0;
		m_data[m_cursor] = n;
	}
	int get_amount() const {return m_amount;}
	int cursor() const {return m_cursor;}
	T get_rate(int num=-1) const
	{
		T rate = 0;
		
		//如果实际还没有收集那么长时间，只取已经统计时间的平均速度
		if(num<=0 || num>(int)m_amount)
			num = (int)(m_amount>LEN?LEN:m_amount);
		if(num>0)
		{
			int n = 0,i = 0;
			int cursor = m_cursor;
			T total = 0;
			for(i=cursor;i>=0 && n<num;--i,++n)
				total += m_data[i];
			for(i=LEN-1;i>cursor && n<num;--i,++n)
				total += m_data[i];
			rate = total/n;
		}
		return rate;
	}
private:
	T m_data[LEN];
	unsigned int m_cursor;
	unsigned int m_amount;
};

}

