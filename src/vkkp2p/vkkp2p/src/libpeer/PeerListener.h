#pragma once

class Peer;
class HttpPeer;
class PeerListener
{
public:
	virtual ~PeerListener(void){}

	template<int I>
	struct S{enum{T=I};};

	typedef S<1> Connecting;
	typedef S<2> Connected;
	typedef S<3> Disconnected;

	typedef S<4> HttpHeader;
	typedef S<5> Data;
	typedef S<6> Writable;

	virtual void on(Connecting,Peer* peer){}
	virtual void on(Connected,Peer* peer){}
	virtual void on(Disconnected,Peer* peer){}

	virtual void on(HttpHeader,Peer* peer,char* buf,int len){}
	virtual void on(Data,Peer* peer,char* buf,int len){}
	virtual void on(Writable,Peer* peer){}
};

class PeerReadLimitSinker
{
public:
	virtual ~PeerReadLimitSinker(void){}
public:
	//Peer目录只对TCP起限速作用
	virtual int on_peer_readlimit(Peer* peer) {return -1;}  //-1不限
};

