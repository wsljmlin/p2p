// SvrSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SvrMgr.h"
#include "SvrSettingDlg.h"
#include "SNDlg.h"
#include "Util.h"
#include "Httpc.h"
#include "AllSNDlg.h"


// CSvrSettingDlg 对话框


IMPLEMENT_DYNAMIC(CSvrSettingDlg, CDialog)

CSvrSettingDlg::CSvrSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSvrSettingDlg::IDD, pParent)
	, m_strTrackIp(_T(""))
	, m_iTrackPort(7120)
	, m_strHttpport(_T("10080"))
{

#ifdef __wh__
	m_strTrackIp=_T("222.135.178.247");
#elif defined(__xy__)
	m_strTrackIp=_T("122.226.79.235");
#endif
}

CSvrSettingDlg::~CSvrSettingDlg()
{
}

void CSvrSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Text(pDX, IDC_EDIT_TRACK_IP, m_strTrackIp);
	DDX_Text(pDX, IDC_EDIT_TRACK_PORT, m_iTrackPort);
	DDX_Control(pDX, IDC_BTN_GET, m_wndBtnGet);
	DDX_Control(pDX, IDC_BTN_SET, m_wndBtnSet);
	DDX_Control(pDX, IDC_STATIC_MSG, m_wndStaMsg);
	DDX_Text(pDX, IDC_EDIT_HTTPPORT, m_strHttpport);
}


BEGIN_MESSAGE_MAP(CSvrSettingDlg, CDialog)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_GET, &CSvrSettingDlg::OnBnClickedBtnGet)
	ON_BN_CLICKED(IDC_BTN_SET, &CSvrSettingDlg::OnBnClickedBtnSet)

	ON_MESSAGE(WM_SET_ITEMS, PopulateComboList)
	ON_MESSAGE(WM_VALIDATE, ListValidate)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CSvrSettingDlg::OnNMRclickList)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CSvrSettingDlg::OnNMDblclkList)
	ON_BN_CLICKED(IDC_BUTTON_STATE, &CSvrSettingDlg::OnBnClickedButtonState)
	ON_BN_CLICKED(IDC_BTN_ALLSN, &CSvrSettingDlg::OnBnClickedBtnAllsn)
END_MESSAGE_MAP()


// CSvrSettingDlg 消息处理程序

BOOL CSvrSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	init();

	////test:
	//InfServer svr[2];
	//svr[0].uin = 1234;
	//svr[0].ver = 10010808;
	//svr[0].sessionID = 772234;
	//svr[0].beginTime = time(NULL);
	//svr[0].sourceNum = 1000;
	//svr[0].bshare = 1;
	//svr[0].peerType = 3;
	//svr[0].natType = 2;
	//svr[0].tcpRealIP = 12341234;
	//svr[0].udpRealIP = 123412333;
	//svr[0].tcpRealPort = 7770;
	//svr[0].udpRealPort = 8880;

	//svr[1].uin = 123433;
	//svr[1].ver = 10010808;
	//svr[1].sessionID = 772244;
	//svr[1].beginTime = time(NULL)-3000;
	//svr[1].sourceNum = 1000;
	//svr[1].bshare = 0;
	//svr[1].peerType = 1;
	//svr[1].natType = 2;
	//svr[1].tcpRealIP = 1234123412;
	//svr[1].udpRealIP = 12412333;
	//svr[1].tcpRealPort = 7771;
	//svr[1].udpRealPort = 8881;

	//on_get_server_list(svr,2);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CSvrSettingDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(IsWindow(m_list.m_hWnd))
	{
		m_list.MoveWindow(10,60,cx-20,cy-70);
	}
}

void CSvrSettingDlg::OnBnClickedBtnGet()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_strTrackIp.IsEmpty() || m_iTrackPort==0)
	{
		AfxMessageBox(_T("tracker ip port不能为空!"));
		return;
	}

	enable_btn(false);
	Tracker tk;
	PTL_P2T_ResponseServerList inf;
	PTL_P2T_ServerInfo svr[2048];
	int n=0;
	int trackId=0,beginTime=0;
	if((n=tk.get_server(m_strTrackIp,m_iTrackPort,inf,svr,2048))>=0)
	{
		CString str;
		time_t bt = inf.beginTime;
		tm *t = ::localtime(&bt);
		str.Format("%d.%d.%d %d:%d:%d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		CString msg;
		msg.Format("[TrackID:%d; TrackVer:%d; UserNum: %d; TrackBeginTime:%s]",inf.trackID,inf.trackVer,inf.userNum,str);
		m_wndStaMsg.SetWindowText(msg);
		on_get_server_list(svr,n);
	}
	else
	{
		AfxMessageBox(_T("获取失败!"));
	}
	enable_btn();
}

void CSvrSettingDlg::OnBnClickedBtnSet()
{
	// TODO: 在此添加控件通知处理程序代码
	enable_btn(false);
	//update_server();
	enable_btn();
}
void CSvrSettingDlg::enable_btn(bool b/*=true*/)
{
	m_wndBtnGet.EnableWindow(b);
	m_wndBtnSet.EnableWindow(b);
}
LRESULT CSvrSettingDlg::PopulateComboList(WPARAM wParam,LPARAM lParam)
{
	// Get the Combobox window pointer
	CComboBox* pInPlaceCombo = static_cast<CComboBox*> (GetFocus());

	// Get the inplace combbox top left
	CRect obWindowRect;

	pInPlaceCombo->GetWindowRect(&obWindowRect);
	
	CPoint obInPlaceComboTopLeft(obWindowRect.TopLeft()); 
	
	// Get the active list
	// Get the control window rect
	// If the inplace combobox top left is in the rect then
	// The control is the active control
	m_list.GetWindowRect(&obWindowRect);
	
	int iColIndex = (int )wParam;
	
	CStringList* pComboList = reinterpret_cast<CStringList*>(lParam);
	pComboList->RemoveAll(); 
	
	if (obWindowRect.PtInRect(obInPlaceComboTopLeft)) 
	{
		if(3==iColIndex)
		{
			pComboList->AddTail(_T("共享"));
			pComboList->AddTail(_T("否"));
		}
	}
	return true;
}
LRESULT CSvrSettingDlg::ListValidate(WPARAM wParam, LPARAM lParam)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
	CComboListCtrl *plist = (CComboListCtrl*)this->GetDlgItem((int)wParam);
	if(plist==&m_list)
	{
		if(pDispInfo->item.lParam)
		{
			CString str = plist->GetItemText(pDispInfo->item.iItem,0);
			if(str.GetAt(0)!='*')
			{
				str = _T("*")+str;
				plist->SetItemText(pDispInfo->item.iItem,0,str);
			}
		}
	}
	return 0;
}
void CSvrSettingDlg::init()
{
	{
		// num(0)|ip(1)|ver(2)|bshare(3)|uid(4)|sessionID(5)|beginTime(6)|sourceNum(7)|peerType(8)|natType(9)|tcpRealIP:Port(10)|udpRealIP:Port(11)||||||||||
		m_list.SetBkColor(RGB(180,220,250));
		m_list.SetTextBkColor(RGB(180,220,250));
		//m_list.SetBkColor(RGB(240,240,240));
		//m_list.SetTextBkColor(RGB(240,240,240));
		m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES /*| LVS_EX_CHECKBOXES*/);
		m_list.SetReadOnlyColumns(0);
		m_list.SetReadOnlyColumns(1);
		m_list.SetReadOnlyColumns(2);
		//m_list.SetReadOnlyColumns(3);
		m_list.SetReadOnlyColumns(4);
		m_list.SetReadOnlyColumns(5);
		m_list.SetReadOnlyColumns(6);
		m_list.SetReadOnlyColumns(7);
		m_list.SetReadOnlyColumns(8);
		m_list.SetReadOnlyColumns(9);
		m_list.SetReadOnlyColumns(10);
		m_list.SetReadOnlyColumns(11);
		m_list.SetReadOnlyColumns(12);
		m_list.SetReadOnlyColumns(13);

		//CString str = _T("");
		//m_list.SetColumnValidEditCtrlCharacters(str,0);
		//m_list.SetProcessCol(3);

		//要能够下拉，list属性必须"enable edit"为true
		m_list.SetComboColumns(3);

		m_list.EnableVScroll();
		m_list.EnableHScroll(false);

		// num(0)|ip(1)|ver(2)|bshare(3)|uin(4)|sessionID(5)|beginTime(6)|sourceNum(7)|peerType(8)|natType(9)|tcpRealIP:Port(10)|udpRealIP:Port(11)||||||||||
		m_list.InsertColumn(0,_T("序"),LVCFMT_LEFT,30);
#ifdef __xy__
		m_list.InsertColumn(1,_T("IP"),LVCFMT_LEFT,0);
		m_list.InsertColumn(10,_T("tcpip端口"),LVCFMT_LEFT,0);
		m_list.InsertColumn(11,_T("udpip端口"),LVCFMT_LEFT,0);
#else
		m_list.InsertColumn(1,_T("IP"),LVCFMT_LEFT,100);
		m_list.InsertColumn(10,_T("tcpip端口"),LVCFMT_LEFT,140);
		m_list.InsertColumn(11,_T("udpip端口"),LVCFMT_LEFT,140);
#endif
		m_list.InsertColumn(2,_T("版本"),LVCFMT_LEFT,70);
		m_list.InsertColumn(3,_T("*共享*"),LVCFMT_LEFT,0);
		m_list.InsertColumn(4,_T("uid"),LVCFMT_LEFT,50);
		m_list.InsertColumn(5,_T("sessionID"),LVCFMT_LEFT,60);
		m_list.InsertColumn(6,_T("登陆时间"),LVCFMT_LEFT,120);
		m_list.InsertColumn(7,_T("文件数"),LVCFMT_LEFT,50);
		m_list.InsertColumn(8,_T("peer类型"),LVCFMT_LEFT,60);
		m_list.InsertColumn(9,_T("nat类型"),LVCFMT_LEFT,60);
		m_list.InsertColumn(12,_T("http端口"),LVCFMT_LEFT,60);
		m_list.InsertColumn(13,_T("menu"),LVCFMT_LEFT,60);
	}
}

CStringA ip_hltoa(unsigned int ip)
{
	unsigned char ip_n[4];
	ip_n[0] = ip >> 24;
	ip_n[1] = ip >> 16;
	ip_n[2] = ip >> 8;
	ip_n[3] = ip;
	char buf[32];
	sprintf(buf,"%d.%d.%d.%d",ip_n[0],ip_n[1],ip_n[2],ip_n[3]);
	return buf;
}
int CSvrSettingDlg::on_get_server_list(PTL_P2T_ServerInfo *svr,int size)
{
	m_list.DeleteAllItems();
	m_list_sn_iphttpport.clear();
	//char buf[1024];
	CStringA str,str2,iphttpport;
	int n = 0;

	// num(0)|ip(1)|ver(2)|bshare(3)|uid(4)|sessionID(5)|beginTime(6)|sourceNum(7)|peerType(8)|natType(9)|tcpRealIP:Port(10)|udpRealIP:Port(11)||||||||||
	for(int i=0;i<size;++i)
	{
		str.Format("%d",i+1);
		n = m_list.InsertItem(i,str);
		str2 = ip_hltoa(svr[i].tcpRealIP);
		iphttpport = str2;

#ifndef __xy__
		str.Format("%s:%d",str2,svr[i].tcpRealPort);
		m_list.SetItemText(n,1,str2);
		m_list.SetItemText(n,10,str);
		str2 = ip_hltoa(svr[i].udpRealIP);
		str.Format("%s:%d",str2,svr[i].udpRealPort);
		m_list.SetItemText(n,11,str);
#endif

		str.Format("%d",svr[i].ver);
		m_list.SetItemText(n,2,str);

		if(svr[i].menu & 0x01)
			str = "共享";
		else
			str = "否";
		m_list.SetItemText(n,3,str);

		//http port
		uint32 port = svr[i].menu >> 16;
		str.Format("%d",port);
		iphttpport += ":"+str;
		m_list.SetItemText(n,12,str);
		//menu
		str.Format("%d",(svr[i].menu & 0x00ff));
		m_list.SetItemText(n,13,str);

		str.Format("%s",svr[i].uid);
		m_list.SetItemText(n,4,str);

		str.Format("%d",svr[i].sessionID);
		m_list.SetItemText(n,5,str);

		time_t bt = svr[i].beginTime;
		tm *t = ::localtime(&bt);
		str.Format("%d.%d.%d %d:%d:%d",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);
		m_list.SetItemText(n,6,str);

		str.Format("%d",svr[i].sourceNum);
		m_list.SetItemText(n,7,str);

		str.Format("%d",svr[i].utype);
		m_list.SetItemText(n,8,str);

		str.Format("%d",svr[i].ntype);
		m_list.SetItemText(n,9,str);

		m_list_sn_iphttpport.push_back(iphttpport);
	}
	return 0;
}
int CSvrSettingDlg::update_server()
{
	//// num(0)|ip(1)|ver(2)|bshare(3)|uin(4)|sessionID(5)|beginTime(6)|sourceNum(7)|peerType(8)|natType(9)|tcpRealIP:Port(10)|udpRealIP:Port(11)||||||||||
	//int count = m_list.GetItemCount();
	//int n=0;
	//CString str;
	//P2TRspUpdateServer svr[2048];
	//for(int i=0;i<count;++i)
	//{
	//	str = m_list.GetItemText(i,0);
	//	if(str.GetAt(0)!='*')
	//		continue;

	//	svr[n].uin = atoi(m_list.GetItemText(i,4));
	//	svr[n].sessionID = atoi(m_list.GetItemText(i,5));
	//	svr[n].flag = 1;
	//	svr[n].limit_share_connect = 0;
	//	svr[n].limit_share_speed_KB = 0;
	//	svr[n].brestart = 0;
	//	str = m_list.GetItemText(i,3);
	//	if(str == "否")
	//		svr[n].bshare = 0;
	//	else
	//		svr[n].bshare = 1;
	//	n++;
	//}
	//if(n>0)
	//{
	//	Tracker tk;
	//	if(0==tk.update_server(m_strTrackIp,m_iTrackPort,svr,n))
	//	{
	//		AfxMessageBox("更新成功,请重新获取查看结果！");
	//	}
	//}
	return 0;
}


void CSvrSettingDlg::OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CSvrSettingDlg::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	POSITION pos = m_list.GetFirstSelectedItemPosition();
	int n = m_list.GetNextSelectedItem(pos);
	if(-1!=n)
	{
		CString ip = m_list.GetItemText(n,1);
		CString port = m_list.GetItemText(n,12);
		if(port!="0")
		{
			CSNDlg sndlg;
			sndlg.m_strSNIPPort = ip+":"+port;
			sndlg.DoModal();
		}
	}
	*pResult = 0;
}

void CSvrSettingDlg::OnBnClickedButtonState()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(true);
	char buf[1024];
	sprintf(buf,"http://%s:%s/tracker/allfile_info.do",(LPCSTR)m_strTrackIp,(LPCSTR)m_strHttpport);
	string strret,header;
	if(0==Httpc::request(buf,NULL,0,header,strret))
	{
		MessageBoxA(strret.c_str(),"tracker_info",MB_OK);
	}
}

void CSvrSettingDlg::OnBnClickedBtnAllsn()
{
	// TODO: 在此添加控件通知处理程序代码
	this->UpdateData(true);
	CAllSNDlg sndlg;
	if(!m_list_sn_iphttpport.empty())
	{
		sndlg.m_strTrackeriphttpport.Format("%s:%s",(LPCSTR)m_strTrackIp,(LPCSTR)m_strHttpport);
		sndlg.m_sns = m_list_sn_iphttpport;
		sndlg.DoModal();
	}
}
