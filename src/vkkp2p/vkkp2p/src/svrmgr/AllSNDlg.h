#pragma once
#include "afxcmn.h"
#include "Util.h"
#include "afxwin.h"

// CAllSNDlg �Ի���

class CAllSNDlg : public CDialog
{
	DECLARE_DYNAMIC(CAllSNDlg)

public:
	CAllSNDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CAllSNDlg();

// �Ի�������
	enum { IDD = IDD_DLG_ALLSN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_lspl;
public:
	afx_msg void OnBnClickedButtonAllsnPl();
public:
	virtual BOOL OnInitDialog();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	typedef struct tagPlInfo
	{
		int usernum;
		int snnum;
	}PlInfo_t;

	list<CStringA> m_sns;
	map<CStringA,PlInfo_t> m_map_plinfo;

	CStringA m_strTrackeriphttpport;
	void GetAllList();
	void GetTrackerAllInfoList();
public:
	CStatic m_winMsg;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	CButton m_wndBtnTimer;
	bool m_bTimer;
public:
	afx_msg void OnBnClickedButtonTimer();
public:
	int m_iTimeOut;
};
