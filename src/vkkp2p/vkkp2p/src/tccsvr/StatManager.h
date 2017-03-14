#pragma once
#include "commons.h"
class StatManager
{
public:
	StatManager(void);
	~StatManager(void);

public:
	void handle_timer();

private:
	void on_timer_log();

private:
	int m_curr_sec;
	int m_last_log_sec;
	string m_str_curr_day;
};
typedef Singleton<StatManager> StatManagerSngl;

