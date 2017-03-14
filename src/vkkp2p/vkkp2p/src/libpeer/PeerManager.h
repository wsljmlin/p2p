#pragma once
#include "PeerFactory.h"

class PeerDownloadHandler
{
public:
	virtual ~PeerDownloadHandler(void){}
public:
	virtual int on_peer_ready(Peer* peer,void* dref){return -1;}
	virtual void on_peer_close(Peer* peer,void* dref){}
	virtual int on_peer_turn(Peer* peer,uint32 sid){return -1;}
	virtual int on_ptl_packet(Peer* peer,uint16 cmd,PTLStream& ss){ return 0; }
};
class PeerShareHandler
{
public:
	virtual ~PeerShareHandler(void){}
public:
	virtual int peer_attach(Peer* peer){return -1;}
	virtual void peer_detach(Peer* peer){}
	virtual void on_writable(Peer* peer){}
	virtual int on_ptl_packet(Peer* peer,uint16 cmd,PTLStream& ss){ return 0; }
};
class PeerManager : public PeerFactory
{
public:
	PeerManager(void);
	virtual ~PeerManager(void);

public:
	int init(PeerDownloadHandler* pdh,PeerShareHandler* psh);
	int add_dref(void* dref,Peer* peer);
	int del_dref(void* dref,Peer* peer);
	int add_share(Peer* peer);
	int del_share(Peer* peer);
	int check_put_peer(Peer* peer);
public:
	virtual void on(PeerListener::Connecting,Peer* peer);
	virtual void on(PeerListener::Connected,Peer* peer);
	virtual void on(PeerListener::Data,Peer* peer,char* buf,int len);
	virtual void on(PeerListener::Writable,Peer* peer);
	virtual void on_disconnected(Peer* peer);
private:
	int PTL_welcome(Peer* peer);
	int PTL_release(Peer* peer);

	int ON_PTL_welcome(Peer* peer,PTLStream& ss);
	int ON_PTL_release(Peer* peer,PTLStream& ss);
private:
	PeerDownloadHandler *m_pdh;
	PeerShareHandler *m_psh;
	PTLStream _ss;
	PTL_Head _head;
};
typedef Singleton<PeerManager> PeerManagerSngl;

