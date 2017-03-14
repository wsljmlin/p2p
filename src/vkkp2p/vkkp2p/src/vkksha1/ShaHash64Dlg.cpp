// ShaHash64Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ShaHash64.h"
#include "ShaHash64Dlg.h"
#include "RDBFile64.h"
#include "sha1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CShaHash64Dlg �Ի���




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


// CShaHash64Dlg ��Ϣ�������

BOOL CShaHash64Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CShaHash64Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ��������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ��
//
HCURSOR CShaHash64Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CShaHash64Dlg::OnBnClickedBtnBlower()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strFilter;
	strFilter = _T("��Ƶ�ļ�(*.avi;*.mov;*.mp4;*.mpg;*.qt;*.rm;*.rmvb;*.wmv;*.mkv;*.flv;*.ic2;*.vkk)|*.avi;*.mov;*.mp4;*.mpg;*.qt;*.rm;*.rmvb;*.wmv;*.mkv;*.flv;*.ic2;*.vkk|");
	strFilter += _T("��Ƶ�ļ�(*.mp3;*.wma)|*.mp3;*.wma|");
	strFilter += _T("�����ļ�(*.*)|*.*|");
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	UpdateData();
	if(m_strPath.IsEmpty())
	{
		AfxMessageBox("��ѡ���ļ�");
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
