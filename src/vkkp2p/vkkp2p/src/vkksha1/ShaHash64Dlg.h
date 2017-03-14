// ShaHash64Dlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CShaHash64Dlg �Ի���
class CShaHash64Dlg : public CDialog
{
// ����
public:
	CShaHash64Dlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SHAHASH64_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_strPath;
public:
	CString m_strHash;
public:
	afx_msg void OnBnClickedBtnBlower();
public:
	afx_msg void OnBnClickedBtnSha();
public:
	CButton m_wndBtnSha;
public:
	LONGLONG m_iFileSize;
};
