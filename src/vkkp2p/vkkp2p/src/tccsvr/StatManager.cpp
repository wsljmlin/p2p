#include "StatManager.h"
#include "Util.h"
#include "PeerManager.h"
//#include "UDPServer.h"

string format_curr_day()
{
	time_t tt = time(0);
	tm* t = localtime(&tt);
	char buf[32];
	sprintf(buf,"%04d.%02d.%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday);
	return buf;
}
StatManager::StatManager(void)
: m_curr_sec(0)
, m_last_log_sec(0)
{
	m_str_curr_day = format_curr_day();
	Util::my_create_directory("./stuns_log");
}

StatManager::~StatManager(void)
{
}
void StatManager::handle_timer()
{
	m_curr_sec++;
	if(m_last_log_sec+600<m_curr_sec)
	{
		m_last_log_sec = m_curr_sec;
		on_timer_log();
	}

}
void StatManager::on_timer_log()
{
	
	int test_num=0,test_succeed_num=0,udp_cmd_num=0,udp_wrong_cmd_num=0,channel_num=0;
	channel_num = PeerManagerSngl::instance()->get_peer_num();
	PeerManagerSngl::instance()->get_test_num(test_num,test_succeed_num);

	string day = format_curr_day();
	if(day!=m_str_curr_day)
	{
		m_str_curr_day = day;
		PeerManagerSngl::instance()->reset_test_num();
	}

	char path[64];
	sprintf(path,"./stuns_log/%s.log",day.c_str());
	char buf[1024];
	sprintf(buf,"channel_num=%d , test_succeed_num=%d , test_num=%d , udp_cmd_num=%d , udp_wrong_cmd_num=%d",
		channel_num,test_succeed_num,test_num,udp_cmd_num,udp_wrong_cmd_num);

	Util::write_log(buf,path);
}

