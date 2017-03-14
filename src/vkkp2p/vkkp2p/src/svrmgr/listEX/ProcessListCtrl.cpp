// ProcessListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "ProcessListCtrl.h"
#include ".\processlistctrl.h"


// CProcessListCtrl

IMPLEMENT_DYNAMIC(CProcessListCtrl, CListCtrl)
CProcessListCtrl::CProcessListCtrl():
m_iProcessCol(-1)
{
}

CProcessListCtrl::~CProcessListCtrl()
{
}


BEGIN_MESSAGE_MAP(CProcessListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
END_MESSAGE_MAP()



// CProcessListCtrl 消息处理程序


void CProcessListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		// This is the notification message for an item.  We'll request
		// notifications before each subitem's prepaint stage.
		  
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if (pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
	{
		// This is the prepaint stage for a subitem. Here's where we set the
		// item's text and background colors. Our return value will tell
		// Windows to draw the subitem itself, but it will use the new colors
		// we set here.
		  
		int nItem = static_cast<int> (pLVCD->nmcd.dwItemSpec);
		int nSubItem = pLVCD->iSubItem;
		  
		if(nSubItem != m_iProcessCol)//这里我只重绘一列
			return;

		COLORREF crText  = ::GetSysColor(COLOR_WINDOWFRAME);
		COLORREF crBkgnd = ::GetSysColor(COLOR_WINDOW);
		  
		CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
		CRect rect;
		GetSubItemRect(nItem, nSubItem, LVIR_BOUNDS, rect);
		if (GetItemState(nItem, LVIS_SELECTED))
			DrawText(nItem, nSubItem, pDC, ::GetSysColor(COLOR_HIGHLIGHT), 
				::GetSysColor(COLOR_HIGHLIGHT), rect);
		else
			DrawText(nItem, nSubItem, pDC, crText, crBkgnd, rect);

		*pResult = CDRF_SKIPDEFAULT; // We've painted everything.
	}


}

void CProcessListCtrl::DrawText(int nItem, 
        int nSubItem, 
        CDC *pDC, 
        COLORREF crText, 
        COLORREF crBkgnd, 
        CRect &rect)
{
	ASSERT(pDC);
	pDC->FillSolidRect(&rect, crBkgnd);
	 
	int nProcess = 0;//GetItemData(nItem);


	CString s = GetItemText(nItem,nSubItem);
	//add by he_cl 解决不同编码的兼用问题
#ifdef UNICODE
	{
		int n=0;
		int len=s.GetLength();
		char *p = NULL;
		n = WideCharToMultiByte(/*CP_ACP*/936,0,s,len,NULL,0,NULL,NULL);
		p = new char[n+1];
		n = WideCharToMultiByte(/*CP_ACP*/936,0,s,len,p,n,NULL,NULL);
		p[n] = '\0';
		nProcess = atoi(p);
		delete[] p;
	}
#else
	nProcess = atoi(s);
#endif

	CRect procRect = rect;
	procRect.left += 1;
	procRect.bottom -= 1;
	pDC->Rectangle(procRect);

	procRect.left += 1;
	procRect.bottom -= 1;
	procRect.top += 1;
	procRect.right = procRect.left + (rect.Width()-3) * nProcess / 100;
	CBrush brush(RGB(150,150,255));
	pDC->FillRect(&procRect, &brush);
	 
	CString str;
	str.Format(_T("%d%%"), nProcess);
	 
	if (!str.IsEmpty())
	{
		UINT nFormat = DT_VCENTER | DT_SINGLELINE | DT_CENTER;
		  
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(crText);
		pDC->SetBkColor(crBkgnd);
		pDC->DrawText(str, &rect, nFormat);
	}
}

