#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CSNDlg �Ի���

class CSNDlg : public CDialog
{
	DECLARE_DYNAMIC(CSNDlg)

public:
	CSNDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CSNDlg();

// �Ի�������
	enum { IDD = IDD_DLG_SN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CString m_strSNIPPort;
	CListCtrl m_lspl;
	//CListBox m_ls;
public:
	virtual BOOL OnInitDialog();

public:
	void GetAllList();
public:
	afx_msg void OnBnClickedButtonAllpl();

	afx_msg void OnNMDblclkListPl(NMHDR *pNMHDR, LRESULT *pResult);
public:
	CStatic m_winMsg;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg void OnBnClickedButtonState();
};
