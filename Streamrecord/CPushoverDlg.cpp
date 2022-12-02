// CPushoverDlg.cpp : implementation file
//

#include "pch.h"
#include "Streamrecord.h"
#include "afxdialogex.h"
#include "CPushoverDlg.h"
#include "loadpref.h"


// CPushoverDlg dialog

IMPLEMENT_DYNAMIC(CPushoverDlg, CDialog)

CPushoverDlg::CPushoverDlg(STREAMRECORD_PREFERENCES *ppref, CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG10, pParent)
	, m_api_key(_T(""))
	, m_api_user(_T(""))
{
	pref = ppref;
	CopyString(m_api_key, pref->api_key); // , 35);
	CopyString(m_api_user, pref->api_user); // , 35);
}

CPushoverDlg::~CPushoverDlg()
{
}

void CPushoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_api_key);
	DDX_Text(pDX, IDC_EDIT2, m_api_user);
}


BEGIN_MESSAGE_MAP(CPushoverDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CPushoverDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CPushoverDlg message handlers


void CPushoverDlg::OnBnClickedOk()
{
	UpdateData(TRUE);
	CopyString(pref->api_key,m_api_key,35);
	CopyString(pref->api_user,m_api_user,35);
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
