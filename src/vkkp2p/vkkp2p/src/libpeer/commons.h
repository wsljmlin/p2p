
#include "comms.h"
#include "SynchroObj.h"
#include "Protocol.h"
#include "SelectReactor.h"
#include "TCPAcceptor.h"
#include "Timer.h"
#include "BaseArray.h"
#include "RDBFile64.h"
#include "IniFile.h"
#include "Util.h"

#include "consts.h"

typedef Singleton<TCPAcceptor> TCPAcceptorSngl;

#define LOG_disconnect(strlog) Util::write_debug_log(strlog,"./log/disconnect.log")

#define LOG_playliststate(strlog) Util::write_debug_log(strlog,"./log/playliststate.log")

