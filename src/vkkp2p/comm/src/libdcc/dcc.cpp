#include "dcc.h"
#include "dcc_devicepool.h"
#include "dcc_httphandler.h"

int dcc_init(const char* conf_xmlpath)
{
	int ret=0;
	if(0!=(ret=dcc_devicepoolsngl::instance()->init(conf_xmlpath)))
		return ret;
	dcc_httpsvr_open(dcc_devicepoolsngl::instance()->get_httpport());
	return 0;
}
int dcc_fini()
{
	dcc_httpsvr_close();
	dcc_devicepoolsngl::instance()->fini();
	dcc_devicepoolsngl::destroy();
	return 0;
}

int dcc_call(int call_id,string& outmsg)
{
	return dcc_devicepoolsngl::instance()->call(call_id,outmsg);
}


