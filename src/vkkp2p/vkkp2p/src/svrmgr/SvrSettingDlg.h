#pragma once
#include "afxcmn.h"

#include "listEX/ComboListCtrl.h"
#include "afxwin.h"
#include "Tracker.h"
#include "basetypes.h"

// CSvrSettingDlg 对话框

class CSvrSettingDlg : public CDialog
{
	DECLARE_DYNAMIC(CSvrSettingDlg)

public:
	CSvrSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSvrSettingDlg();

// 对话框数据
	enum { IDD = IDD_DLG_SVRSETTING };

private:
	void init();
	int on_get_server_list(PTL_P2T_ServerInfo *svr,int size);
	int update_server();
	void enable_btn(bool b=true);
private:
	//CListCtrl m_list;
	CComboListCtrl m_list;
	CString m_strTrackIp;
	int m_iTrackPort;
	list<CStringA> m_list_sn_iphttpport;

	CButton m_wndBtnGet;
	CButton m_wndBtnSet;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnGet();
	afx_msg void OnBnClickedBtnSet();
	afx_msg LRESULT PopulateComboList(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT ListValidate(WPARAM wParam, LPARAM lParam);

public:
	CStatic m_wndStaMsg;
public:
	afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnBnClickedButtonState();
public:
	afx_msg void OnBnClickedBtnAllsn();
public:
	CString m_strHttpport;
};
