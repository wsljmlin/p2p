// SvrMgrDlg.h : ͷ�ļ�
//

#pragma once

#include "SvrSettingDlg.h"

// CSvrMgrDlg �Ի���
class CSvrMgrDlg : public CDialog
{
// ����
public:
	CSvrMgrDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SVRMGR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

private:
	void size();
private:
	CSvrSettingDlg m_SvrSettingDlg;
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
