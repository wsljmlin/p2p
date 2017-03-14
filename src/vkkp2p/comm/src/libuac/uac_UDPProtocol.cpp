#include "uac_UDPProtocol.h"


namespace UAC
{


int operator << (PTLStream& ps, const UDPSConnHeader_t& inf)
{
	ps << inf.cmd;
	ps << inf.src_sessionid;
	ps << inf.des_sessionid;
	return ps.ok();
}
int operator >> (PTLStream& ps, UDPSConnHeader_t& inf)
{
	ps >> inf.cmd;
	ps >> inf.src_sessionid;
	ps >> inf.des_sessionid;
	return ps.ok();
}
int operator << (PTLStream& ps, const UDPSData_t& inf)
{
	ps << inf.sequence_num;
	ps << inf.mask;
	ps << inf.speed_seq;
	ps << inf.buflen;
	//sockpool 已经将数据预拷到尾部
	return ps.ok();
}
int operator >> (PTLStream& ps, UDPSData_t& inf)
{
	ps >> inf.sequence_num;
	ps >> inf.mask;
	ps >> inf.speed_seq;
	ps >> inf.buflen;
	if(inf.buflen>0)
		ps.skipr(inf.buflen);
	return ps.ok();
}

int operator << (PTLStream& ps, const UDPSAck_t& inf)
{
	ps << inf.lowline_num;
	ps << inf.recv_win_num;
	ps << inf.ack_sequence;
	ps << inf.rerecv_num;
	ps << inf.csp_speed_seq;
	ps << inf.csp_num;
	ps << inf.csp_speedB;
	ps << inf.const_send_speedKB;
	ps << inf.const_send_lose_rate;
	//if(inf.size>ACK_ARR_LEN) inf.size = ACK_ARR_LEN;
	ps << inf.size;
	if(inf.size>0)
	{
		ps.write_array(inf.seq_nums,inf.size);
		ps.write_array(inf.wait_us,inf.size);
	}
	return ps.ok();
}
int operator >> (PTLStream& ps, UDPSAck_t& inf)
{
	ps >> inf.lowline_num;
	ps >> inf.recv_win_num;
	ps >> inf.ack_sequence;
	ps >> inf.rerecv_num;
	ps >> inf.csp_speed_seq;
	ps >> inf.csp_num;
	ps >> inf.csp_speedB;
	ps >> inf.const_send_speedKB;
	ps >> inf.const_send_lose_rate;
	ps >> inf.size;
	assert(inf.size<=ACK_ARR_LEN);
	if(inf.size>ACK_ARR_LEN) inf.size = ACK_ARR_LEN;
	if(inf.size>0)
	{
		ps.read_array(inf.seq_nums,inf.size);
		ps.read_array(inf.wait_us,inf.size);
	}
	return ps.ok();
}

}

