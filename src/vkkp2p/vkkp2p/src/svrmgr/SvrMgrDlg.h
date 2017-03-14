// SvrMgrDlg.h : 头文件
//

#pragma once

#include "SvrSettingDlg.h"

// CSvrMgrDlg 对话框
class CSvrMgrDlg : public CDialog
{
// 构造
public:
	CSvrMgrDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SVRMGR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	void size();
private:
	CSvrSettingDlg m_SvrSettingDlg;
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
