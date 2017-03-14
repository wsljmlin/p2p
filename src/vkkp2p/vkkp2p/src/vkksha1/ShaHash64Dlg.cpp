// ShaHash64Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ShaHash64.h"
#include "ShaHash64Dlg.h"
#include "RDBFile64.h"
#include "sha1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CShaHash64Dlg 对话框




CShaHash64Dlg::CShaHash64Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShaHash64Dlg::IDD, pParent)
	, m_strPath(_T(""))
	, m_strHash(_T(""))
	, m_iFileSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CShaHash64Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PATH, m_strPath);
	DDX_Text(pDX, IDC_EDIT_HASH_VAL, m_strHash);
	DDX_Control(pDX, IDC_BTN_SHA, m_wndBtnSha);
	DDX_Text(pDX, IDC_EDIT_SIZE, m_iFileSize);
}

BEGIN_MESSAGE_MAP(CShaHash64Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_BLOWER, &CShaHash64Dlg::OnBnClickedBtnBlower)
	ON_BN_CLICKED(IDC_BTN_SHA, &CShaHash64Dlg::OnBnClickedBtnSha)
END_MESSAGE_MAP()


// CShaHash64Dlg 消息处理程序

BOOL CShaHash64Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CShaHash64Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CShaHash64Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
//
HCURSOR CShaHash64Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CShaHash64Dlg::OnBnClickedBtnBlower()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strFilter;
	strFilter = _T("视频文件(*.avi;*.mov;*.mp4;*.mpg;*.qt;*.rm;*.rmvb;*.wmv;*.mkv;*.flv;*.ic2;*.vkk)|*.avi;*.mov;*.mp4;*.mpg;*.qt;*.rm;*.rmvb;*.wmv;*.mkv;*.flv;*.ic2;*.vkk|");
	strFilter += _T("音频文件(*.mp3;*.wma)|*.mp3;*.wma|");
	strFilter += _T("所有文件(*.*)|*.*|");
	CFileDialog FileDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY|OFN_NOCHANGEDIR,strFilter);	
	if(FileDlg.DoModal()==IDCANCEL) {
		return;
	}

	UpdateData();

	m_strPath = FileDlg.GetPathName();
	if (m_strPath.StringLength(m_strPath)>255)
	{
		m_strPath="";
		return;
	}
	UpdateData(false);
}

void CShaHash64Dlg::OnBnClickedBtnSha()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	if(m_strPath.IsEmpty())
	{
		AfxMessageBox("请选择文件");
		return;
	}
	m_wndBtnSha.EnableWindow(false);

	m_strHash = "";
	m_iFileSize = ERDBFile64::get_filesize(m_strPath);
	UpdateData(false);

	char strHash[48];
	if(0==Sha1_BuildFile(m_strPath,strHash,NULL))
	{
		m_strHash = strHash;
		UpdateData(false);
	}
	m_wndBtnSha.EnableWindow();
}
