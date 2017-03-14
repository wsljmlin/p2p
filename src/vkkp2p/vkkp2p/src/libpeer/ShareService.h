#pragma once
#include "Peer.h"
#include "FileStorage.h"
#include "PeerManager.h"

class ShareService : public TimerHandler
	,public PeerShareHandler
{
	friend class Singleton<ShareService>;

	typedef struct tagBlockInfo{
		hash_t hash;
		unsigned int index;      //���ؿ��
		unsigned int pos;        //���ص���ʼλ��
		unsigned int blockSize;  //�����С
		unsigned int fileBlockSize; //�ļ��ֿ��С
		tagBlockInfo() : index(0),pos(0),blockSize(0),fileBlockSize(0){}
		void reset() { index=0,pos=0;blockSize=0;fileBlockSize=0;}
	}BlockInfo;

	typedef struct tagPeerData{
		hash_t              hash;
		list<BlockInfo>     blockList;
		unsigned int        iCurrIndex;            //��ǰ�����ṩ�Ŀ�
		FileBlock*          fb;
		//bool                isKickOut;    //�����������߳�������
		//int                 iCancelBlockCount;
		//int                 iCancelBlockIndex;
		//int                 iLastReqPos;		//�ϴ���������ʼλ��
		//�ٶ�ͳ��
		unsigned long       iPeerBeginTick;
		uint64              iPeerShareBytes;
		//unsigned long       iBlockBeginTick;
		//uint64              iBlockShareBytes;
		uchar	pause_send;
		tagPeerData() {
			iCurrIndex = 0;
			fb = NULL;
			//isKickOut = false;
			//iCancelBlockCount = 0;
			//iCancelBlockIndex = -1;
			//iLastReqPos = -1;

			iPeerBeginTick = 0;
			iPeerShareBytes = 0;
			//iBlockBeginTick = 0;
			//iBlockShareBytes = 0;
			pause_send = 0;
		}
	}PeerData;

	typedef map<Peer*,PeerData*> PeerMap;
	typedef PeerMap::iterator PeerMapIter;

	typedef list<Peer*> PeerList;
	typedef PeerList::iterator PeerListIter;
private:
	ShareService(void);
	~ShareService(void);
public:
	int init();
	int fini();

	virtual int peer_attach(Peer* peer);
	virtual void peer_detach(Peer* peer);
	virtual void on_writable(Peer* peer);
	virtual int on_ptl_packet(Peer* peer,uint16 cmd,PTLStream& ss);

	virtual void on_timer(int e);
	size_t get_peer_num() const { return m_peers.size();}
private:
	void PutAllPeer();
	int SendBlockData(Peer* peer);
	PeerData *GetPeerData(Peer* peer);

	int PTL_CancelBlock(Peer* peer,const hash_t& hash,int index);

	int ON_PTL_RequestFileBlockTable(Peer* peer,PTLStream& ss);
	int ON_PTL_RequestFileBlocks(Peer* peer,PTLStream& ss);
	int ON_PTL_CancelFileBlocks(Peer* peer,PTLStream& ss);
	int ON_PTL_RequestFileSubKeys(Peer* peer,PTLStream& ss);

	template<typename T>
	int SendPTLPacket(uint16 cmd,T& inf,Peer* peer,int iMaxSize);
private:
	PeerMap       m_peers;
	PeerList      m_sendingPeers;
	unsigned int  m_iCurrTick;
	PTL_Head		m_head;
	hash_t			_hash;
};

typedef Singleton<ShareService> ShareServiceSngl;
