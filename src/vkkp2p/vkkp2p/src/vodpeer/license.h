#pragma once

#include "Thread.h"
#include "Singleton.h"

class License : public Thread
{
public:
	License(void): m_brun(false){}
	virtual ~License(void) {}

public:
	virtual int work(int e);
	int run();
	void end();
private:
	bool m_brun;
};
typedef Singleton<License> LicenseSngl;

bool check_license_ok();
