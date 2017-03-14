// AllSNDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "svrmgr.h"
#include "AllSNDlg.h"

#include "Httpc.h"
#include "ICXml.h"
#include "CharCoder.h"


// CAllSNDlg 对话框

IMPLEMENT_DYNAMIC(CAllSNDlg, CDialog)

CAllSNDlg::CAllSNDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAllSNDlg::IDD, pParent)
	, m_bTimer(false)
	, m_iTimeOut(60)
{

}

CAllSNDlg::~CAllSNDlg()
{
}

void CAllSNDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ALLSN_PL, m_lspl);
	DDX_Control(pDX, IDC_STATIC_ALLSN_MSG, m_winMsg);
	DDX_Control(pDX, IDC_BUTTON_TIMER, m_wndBtnTimer);
	DDX_Text(pDX, IDC_EDIT_TIMEOUT, m_iTimeOut);
}


BEGIN_MESSAGE_MAP(CAllSNDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ALLSN_PL, &CAllSNDlg::OnBnClickedButtonAllsnPl)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_TIMER, &CAllSNDlg::OnBnClickedButtonTimer)
END_MESSAGE_MAP()



BOOL CAllSNDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_lspl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES /*| LVS_EX_CHECKBOXES*/);
	m_lspl.InsertColumn(0,_T("num"),LVCFMT_LEFT,30);
	m_lspl.InsertColumn(1,_T("name"),LVCFMT_LEFT,170);
	m_lspl.InsertColumn(2,_T("url"),LVCFMT_LEFT,0);
	m_lspl.InsertColumn(3,_T("token_i/max_i-fini_num"),LVCFMT_LEFT,120);
	m_lspl.InsertColumn(4,_T("update time"),LVCFMT_LEFT,150);
	m_lspl.InsertColumn(5,_T("downspeed KB"),LVCFMT_LEFT,60);
	m_lspl.InsertColumn(6,_T("sharespeed KB"),LVCFMT_LEFT,60);
	m_lspl.InsertColumn(7,_T("strhash"),LVCFMT_LEFT,100);
	m_lspl.InsertColumn(8,_T("usernum"),LVCFMT_LEFT,60);
	m_lspl.InsertColumn(9,_T("snnum"),LVCFMT_LEFT,60);
	
	
	this->SetTimer(2,200,NULL);

	this->MoveWindow(0,0,850,600);
	this->CenterWindow();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

// CAllSNDlg 消息处理程序

void CAllSNDlg::OnBnClickedButtonAllsnPl()
{
	// TODO: 在此添加控件通知处理程序代码
	GetAllList();
}
void CAllSNDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(2==nIDEvent)
		this->KillTimer(2);
	GetAllList();
	CDialog::OnTimer(nIDEvent);
}
void CAllSNDlg::GetAllList()
{
	string strret,header;
	char url[1024];
	CString str,ipport;
	int i=0,j=0,k=0;
	m_lspl.DeleteAllItems();
	map<CStringA,PlInfo_t>::iterator mpit;

	GetTrackerAllInfoList();
	for(list<CStringA>::iterator it=m_sns.begin();it!=m_sns.end();++it)
	{
		ipport = *it;
		m_lspl.InsertItem(i,"");
#ifdef __xy__
		str.Format("      =>%d.",++k);
#else
		str.Format("      =>%d. %s",++k,ipport);
#endif
		m_lspl.SetItemText(i,1,str);
		i++;
		sprintf(url,"http://%s/playlist/allinfo.do",(LPCSTR)ipport);
		if(0==Httpc::request(url,NULL,0,header,strret))
		{
			ICXml xml;
			if(0==xml.load_string(strret.c_str()))
			{
				//response/pl
				XMLNode *node = xml.find_first_node("response/playlist/pl");
				const char* ptr;
				string sname;
				while(node)
				{
					str.Format("%d",++j);
					m_lspl.InsertItem(i,str);

					ptr = node->attri.get_attri("name");
					if(ptr)
					{
						CharCoder::UTF_8ToGB2312(sname,ptr,(int)strlen(ptr));
						m_lspl.SetItemText(i,1,sname.c_str());
					}

					ptr = node->attri.get_attri("url");
					if(ptr)
						m_lspl.SetItemText(i,2,ptr);
					
					ptr = node->get_data();
					if(ptr)
						m_lspl.SetItemText(i,3,ptr);

					ptr =  node->attri.get_attri("UTIME");
					if(ptr)
						m_lspl.SetItemText(i,4,ptr);
					ptr =  node->attri.get_attri("RS");
					if(ptr)
						m_lspl.SetItemText(i,5,ptr);
					ptr =  node->attri.get_attri("SS");
					if(ptr)
						m_lspl.SetItemText(i,6,ptr);

					ptr = node->attri.get_attri("hash");
					if(ptr)
					{
						m_lspl.SetItemText(i,7,ptr);

						mpit = m_map_plinfo.find(ptr);
						if(mpit!=m_map_plinfo.end())
						{
							str.Format("%d",mpit->second.usernum);
							m_lspl.SetItemText(i,8,str);
							str.Format("%d",mpit->second.snnum);
							m_lspl.SetItemText(i,9,str);
						}
					}
					i++;
					node = node->next();
				}
			}
		}
	}
	time_t ti = time(NULL);
	m_winMsg.SetWindowTextA(Util::time_to_datetime_string(ti).c_str());
}

void CAllSNDlg::GetTrackerAllInfoList()
{
	m_map_plinfo.clear();
	char buf[1024];
	sprintf(buf,"http://%s/tracker/allfile_info.do",(LPCSTR)m_strTrackeriphttpport);
	string strret,header;
	if(0==Httpc::request(buf,NULL,0,header,strret))
	{
		ICXml xml;
		if(0==xml.load_string(strret.c_str()))
		{
			//response/pl
			XMLNode *node = xml.find_first_node("response/file");
			const char* ptr;
			string str,hash;
			PlInfo_t pi;
			while(node)
			{
				ptr = node->get_data();
				if(ptr)
				{
					str = ptr;
					hash = Util::get_string_index(str,0,"-");
					Util::string_trim(hash);
					pi.usernum = atoi(Util::get_string_index(str,1,"-").c_str());
					pi.snnum = atoi(Util::get_string_index(str,2,"-").c_str());
					m_map_plinfo[hash.c_str()] = pi;
				}
				node = node->next();
			}
		}
	}
}


void CAllSNDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	CRect rc;
	this->GetClientRect(&rc);
	if(::IsWindow(m_lspl.m_hWnd))
		this->m_lspl.MoveWindow(rc.left+5,rc.top+25,rc.right-10,rc.bottom-30);

	// TODO: 在此处添加消息处理程序代码
}

void CAllSNDlg::OnBnClickedButtonTimer()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(true);
	if(!m_bTimer)
	{
		if(m_iTimeOut>0)
		{
			m_bTimer = true;
			this->SetTimer(1,m_iTimeOut*1000,NULL);
			m_wndBtnTimer.SetWindowTextA("关定时");
		}
	}
	else
	{
		m_bTimer = false;
		this->KillTimer(1);
		m_wndBtnTimer.SetWindowTextA("开定时");
	}
}
