#include "uac_UDPSpeedCtrl2.h"


namespace UAC
{



UDPSpeedCtrl2::UDPSpeedCtrl2(void)
:level(LEVEL_WIN)
,other_recv_win_num(10) ////初始时只发10个包
,send_win(other_recv_win_num)
,ttlus(100000)
,mtu(0)
,last_change_win_tick(0)
{
}

UDPSpeedCtrl2::~UDPSpeedCtrl2(void)
{
}

}

