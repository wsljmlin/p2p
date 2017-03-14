// SNDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "svrmgr.h"
#include "SNDlg.h"
#include "Httpc.h"
#include "ICXml.h"
#include "Util.h"
#include "CharCoder.h"

// CSNDlg 对话框

IMPLEMENT_DYNAMIC(CSNDlg, CDialog)

CSNDlg::CSNDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSNDlg::IDD, pParent)
	, m_strSNIPPort(_T(""))
{

}

CSNDlg::~CSNDlg()
{
}

void CSNDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_IPPORT, m_strSNIPPort);
	DDX_Control(pDX, IDC_LIST_PL, m_lspl);
	DDX_Control(pDX, IDC_STATIC_MSG, m_winMsg);
}


BEGIN_MESSAGE_MAP(CSNDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_ALLPL, &CSNDlg::OnBnClickedButtonAllpl)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PL, &CSNDlg::OnNMDblclkListPl)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STATE, &CSNDlg::OnBnClickedButtonState)
END_MESSAGE_MAP()


// CSNDlg 消息处理程序

BOOL CSNDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	this->UpdateData(false);
	m_lspl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES /*| LVS_EX_CHECKBOXES*/);
	m_lspl.InsertColumn(0,_T("num"),LVCFMT_LEFT,30);
	m_lspl.InsertColumn(1,_T("name"),LVCFMT_LEFT,150);
	m_lspl.InsertColumn(2,_T("url"),LVCFMT_LEFT,330);
	m_lspl.InsertColumn(3,_T("token_i/max_i-fini_num"),LVCFMT_LEFT,100);
	m_lspl.InsertColumn(4,_T("update time"),LVCFMT_LEFT,150);
	m_lspl.InsertColumn(5,_T("downspeed KB"),LVCFMT_LEFT,60);
	m_lspl.InsertColumn(6,_T("sharespeed KB"),LVCFMT_LEFT,60);
	m_lspl.InsertColumn(7,_T("strhash"),LVCFMT_LEFT,200);
	GetAllList();
	this->SetTimer(1,30000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CSNDlg::GetAllList()
{
	this->UpdateData(true);
	string strret,header;
	char url[1024];
	CString str;
	int i=0;
	sprintf(url,"http://%s/playlist/allinfo.do",(LPCSTR)m_strSNIPPort);
	m_lspl.DeleteAllItems();
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
				str.Format("%d",i+1);
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
					m_lspl.SetItemText(i,7,ptr);
				i++;
				node = node->next();
			}
		}
		time_t ti = time(NULL);
		m_winMsg.SetWindowTextA(Util::time_to_datetime_string(ti).c_str());
	}
	else
	{
		//AfxMessageBox(" *** Get /playlist/allinfo.do faild! ");
		m_winMsg.SetWindowText(_T("Get /playlist/allinfo.do faild!"));
	}

}
void CSNDlg::OnBnClickedButtonAllpl()
{
	// TODO: 在此添加控件通知处理程序代码
	GetAllList();
}



void CSNDlg::OnNMDblclkListPl(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	POSITION pos = m_lspl.GetFirstSelectedItemPosition();
	int n = m_lspl.GetNextSelectedItem(pos);
	if(-1!=n)
	{
		CString url = m_lspl.GetItemText(n,2);
		char buf[1024];
		sprintf(buf,"http://%s/playlist/info.do?url=%s",(LPCSTR)m_strSNIPPort,(LPCSTR)url);
		string strret,header;
		if(0==Httpc::request(buf,NULL,0,header,strret))
		{
			MessageBoxA(strret.c_str(),url,MB_OK);
		}
	}
	*pResult = 0;
}

void CSNDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	GetAllList();
	CDialog::OnTimer(nIDEvent);
}

void CSNDlg::OnBnClickedButtonState()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(true);
	char buf[1024];
	sprintf(buf,"http://%s/vttcmd/state.do",(LPCSTR)m_strSNIPPort);
	string strret,header;
	if(0==Httpc::request(buf,NULL,0,header,strret))
	{
		MessageBoxA(strret.c_str(),"allinfo",MB_OK);
	}
}
