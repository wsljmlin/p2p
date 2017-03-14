#pragma once
#include "uac_Singleton.h"
#include "uac_UDPStunProtocol.h"

namespace UAC
{
class Setting
{
public:
	Setting(void);
	~Setting(void);
public:
	int init();
	int fini();
private:
	void load_setting();
protected:
	GETSET(PTL_STUN_RspStunsvrConfig_t,m_stunsvr_config,_stunsvr_config)
};
typedef Singleton<Setting> SettingSngl;

}
