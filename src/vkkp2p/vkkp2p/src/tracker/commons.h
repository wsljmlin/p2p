#pragma once

#define PEER_MAX_SIZE 200000
////test:
//#define PEER_MAX_SIZE 10
#include "comms.h"
#include "Protocol.h"
#include "BaseArray.h"
#include "ProReactor.h"
#include "SelectReactor.h"
#include "TCPAcceptor.h"
#include "Util.h"

#define LOG_disconnect(logbuf) Util::write_debug_log(logbuf,"./log/disconnect.log")



