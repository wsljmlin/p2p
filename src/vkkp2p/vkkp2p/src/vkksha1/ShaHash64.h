// ShaHash64.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CShaHash64App:
// �йش����ʵ�֣������ ShaHash64.cpp
//

class CShaHash64App : public CWinApp
{
public:
	CShaHash64App();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CShaHash64App theApp;