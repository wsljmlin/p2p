#ifndef __CMNLICENSE_H__
#define __CMNLICENSE_H__

class CmnLicense
{
public:
    static int GenerateLicense(const char* ip, const char* enddate, char** pbuf, int* buflen);
    static bool CheckLicense(const char* ip, const char* enddate, char* buf, int len);

	static int GenerateLicenseFile(const char* ip, const char* enddate, const char* filename);
	static bool CheckLicenseFile(const char* ip, const char* enddate, const char* filename);
};

#endif
