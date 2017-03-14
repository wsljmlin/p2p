#include "AutoFileManager.h"
#include "SourceService.h"
#include "UserManager.h"
#include "Setting.h"

AutoFileManager::AutoFileManager(void)
: m_binit(false)
, m_vipsuper_minnum(SettingSngl::instance()->get_vip_super_minnum())
{
}

AutoFileManager::~AutoFileManager(void)
{
	assert(m_supers.empty());
	assert(m_vipsources.empty());
	assert(m_vipfiles.empty());
}



int AutoFileManager::init()
{
	m_binit = true;
	return 0;
}

void AutoFileManager::fini()
{
	if(!m_binit)
		return;
}

int AutoFileManager::add_super(UserInfo* user)
{
	m_supers.push_back(user);
	return 0;
}

int AutoFileManager::del_super(UserInfo* user)
{
	m_supers.remove(user);
	return 0;
}

int AutoFileManager::add_vipsource(SourceInfo* node)
{
	m_vipsources.push_back(node);
	return 0;
}

int AutoFileManager::del_vipsource(SourceInfo* node)
{
	m_vipsources.remove(node);
	return 0;
}

//��ΪVIP��Ŀ��������һ��VIP�û���ʼ���ش��ļ���ʼ��ΪVIP��Ŀ
int AutoFileManager::on_vipfile_start(PTL_P2T_ReportStartDownloadList& inf)
{
	//��ʼ��ȹ������ȣ�������ʱ��sourcelist��δһ���д˽�Ŀ
	hash_t hash;
	string url;
	hash.set_buffer((uchar*)inf.fhash);
	//Ŀǰֻ�Զ�����playlist
	if(HT_URLDL!=hash.hash_type())
		return 0;
	url = inf.url;
	assert(!url.empty());
	if(url.empty())
		return 0;
	VipfileIter it=m_vipfiles.find(hash);
	if(it==m_vipfiles.end())
	{	
		m_vipfiles[hash] = url;
	}
	handle_super_source(hash,url);
	return 0;
}
//ֹͣ����:�����й����vipԴ����SoureService��delete�Ǵ���
int AutoFileManager::on_vipfile_stop(const hash_t& hash)
{
	if(HT_URLDL!=hash.hash_type())
		return 0;
	VipfileIter it=m_vipfiles.find(hash);
	if(it!=m_vipfiles.end())
		m_vipfiles.erase(it);
	
	if(m_vipsuper_minnum>0)
	{
		//��source service�л�ȡ����super
		list<UserInfo*> ls;
		SourceServiceSngl::instance()->get_super_by_hash(hash,ls);
		for(list<UserInfo*>::iterator it=ls.begin();it!=ls.end();++it)
		{
			UserManagerSngl::instance()->ptl_request_stopdownloadlist(*it,hash);
		}
	}
	return 0;
}
int AutoFileManager::handle_super_source(hash_t& hash,const string& url)
{
	//Ϊ0ʱ������
	if(m_vipsuper_minnum<=0)
		return 0;
	int n = SourceServiceSngl::instance()->get_super_source_num(hash);
	if(n>=m_vipsuper_minnum)
		return 0;
	n = m_vipsuper_minnum-n; //���ֻ��n��super���ٱ���Ŀ
	if(n>(int)m_supers.size())
		n = m_supers.size();

	//UserInfo *user = NULL;
	UserInfo *tmpuser = NULL,*maxuser;
	map<hash_t,uint32>::iterator hit;
	int tmpn = 0,maxn = 0;
	while(n>0)
	{
		//user = NULL;
		maxuser = NULL;
		maxn = 0;
		//��һ��û�м��ٴ˽�Ŀ��super����������е�super
		for(UserIter it=m_supers.begin();it!=m_supers.end();++it)
		{
			tmpuser = *it;
			if(tmpuser->sourceMap.find(hash)==tmpuser->sourceMap.end())
			{
				tmpn = tmpuser->downloadlist_maxnum - tmpuser->downloadlist_num;
				if(tmpn>maxn)
				{
					maxn = tmpn;
					maxuser = tmpuser;
				}
			}
		}
		if(!maxuser)
			break;
		//���ͼ���ָ������������ӵ������б�
		UserManagerSngl::instance()->ptl_request_startdownloadlist(maxuser,hash,url.c_str());
		n--;
	}
	return 0;
}

