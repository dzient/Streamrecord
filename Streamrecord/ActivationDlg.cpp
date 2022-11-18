// ActivationDlg.cpp : implementation file
//
///#include "pch.h"
#include "stdafx.h"
#include "streamrecord.h"
#include "ActivationDlg.h"
#include ".\activationdlg.h"
#include "activation.h"
#include "StreamInstance.h"


// ActivationDlg dialog

IMPLEMENT_DYNAMIC(ActivationDlg, CDialog)
ActivationDlg::ActivationDlg(STREAMRECORD_PREFERENCES *pref,
							 CWnd* pParent /*=NULL*/)
	: CDialog(ActivationDlg::IDD, pParent)
	, m_activation_code(_T(""))
{
	pref_ptr = pref;
	got_focus = FALSE;
}

ActivationDlg::~ActivationDlg()
{
}

void ActivationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_activation_code);
	DDX_Check(pDX, IDC_CHECK5, database);
}


BEGIN_MESSAGE_MAP(ActivationDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnEnKillfocusEdit1)
	ON_EN_SETFOCUS(IDC_EDIT1, OnEnSetfocusEdit1)
	ON_BN_CLICKED(IDC_CHECK5, &ActivationDlg::OnBnClickedDatabase)
	ON_BN_CLICKED(IDC_BUTTON15, &ActivationDlg::OnBnClickedDatabaseSettings)
	ON_BN_CLICKED(IDC_CHECK6, &ActivationDlg::OnBnClickedEnableDatabaseSettings)
END_MESSAGE_MAP()


// ActivationDlg message handlers

void ActivationDlg::OnOk()
{
	// TODO: Add your control notification handler code here

	CDialog::OnOK();
}

void ActivationDlg::OnEnKillfocusEdit1()
{
	// TODO: Add your control notification handler code here
	char str[32];
	bool check = true;

	if (got_focus)
	{	
		UpdateData(TRUE);
		///strncpy(str,LPCTSTR(m_activation_code),16);
		CopyString(m_activation_code, str);
		str[16] = NULL;
		if (!check) //check_activation_code(str))
		{
			MessageBoxA(NULL,LPCSTR("Invalid activation code."),PROGRAM_NAME,
				MB_OK|MB_ICONEXCLAMATION);
			pref_ptr->code_is_good = 0;
		}
		else
		{
			strcpy(pref_ptr->activation_code,str);
			pref_ptr->code_is_good = 1;
		}
	}
}

void ActivationDlg::OnEnSetfocusEdit1()
{
	// TODO: Add your control notification handler code here
	got_focus = TRUE;
}


void ActivationDlg::OnBnClickedDatabase()
{
	// TODO: Add your control notification handler code here
}


void ActivationDlg::OnBnClickedDatabaseSettings()
{
	// TODO: Add your control notification handler code here
}




void ActivationDlg::OnBnClickedEnableDatabaseSettings()
{
	// TODO: Add your control notification handler code here
}
