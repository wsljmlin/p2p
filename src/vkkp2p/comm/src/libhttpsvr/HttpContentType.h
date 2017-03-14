#pragma once
#include "basetypes.h"

class HttpContentType
{
public:
	HttpContentType(void);
	~HttpContentType(void);

	string get_ct_name(const string& key);
	string operator [](const string& key);

private:
	map<string,string> m_ct_map;
};

static HttpContentType g_content_type;
