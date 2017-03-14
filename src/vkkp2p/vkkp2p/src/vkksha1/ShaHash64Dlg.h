// ShaHash64Dlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CShaHash64Dlg 对话框
class CShaHash64Dlg : public CDialog
{
// 构造
public:
	CShaHash64Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SHAHASH64_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
