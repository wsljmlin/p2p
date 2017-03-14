#pragma once


// CProcessListCtrl

/*
����:���������б�ؼ�
����ʱ��:2005-11-10
����:�δ���
*/
class CProcessListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CProcessListCtrl)

public:
	CProcessListCtrl();
	virtual ~CProcessListCtrl();
public:
	void DrawText(int nItem, 
        int nSubItem, 
        CDC *pDC, 
        COLORREF crText, 
        COLORREF crBkgnd, 
        CRect &rect);

	void SetProcessCol(int col){
		m_iProcessCol = col;
	};

private:
	int m_iProcessCol;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};


