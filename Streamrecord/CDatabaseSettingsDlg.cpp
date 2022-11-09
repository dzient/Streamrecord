// CDatabaseSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "Streamrecord.h"
#include "afxdialogex.h"
#include "CDatabaseSettingsDlg.h"
#include "loadpref.h"


// CDatabaseSettingsDlg dialog

IMPLEMENT_DYNAMIC(CDatabaseSettingsDlg, CDialog)

CDatabaseSettingsDlg::CDatabaseSettingsDlg(STREAMRECORD_PREFERENCES* ppref,
	IGNORE_LIST* ignore,
	SCHEDULE_ADD_LIST* add,
	CWnd* pParent) : CDialog(IDD_DIALOG9, pParent)
	, m_interval(30)
{
	//char temp[256];
	pref = ppref;
	pignore = ignore;
	padd = add;
	m_ipaddress = ppref->ip_address;
	m_schema = ppref->schema;
	m_username = ppref->username;
	m_password = ppref->password;
	m_port = ppref->port;
	//sprintf(temp, "%d", ppref->DBinterval);
	m_interval = ppref->DBinterval;
}

CDatabaseSettingsDlg::~CDatabaseSettingsDlg()
{
}

void CDatabaseSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_ipaddress);
	DDV_MaxChars(pDX, m_ipaddress, 255);
	DDX_Text(pDX, IDC_EDIT2, m_schema);
	DDV_MaxChars(pDX, m_schema, 255);
	DDX_Text(pDX, IDC_EDIT4, m_password);
	DDV_MaxChars(pDX, m_password, 255);
	DDX_Text(pDX, IDC_EDIT3, m_username);
	DDV_MaxChars(pDX, m_username, 255);
	DDX_Text(pDX, IDC_EDIT5, m_port);
	DDV_MaxChars(pDX, m_port, 255);
	DDX_Text(pDX, IDC_EDIT6, m_interval);
	DDV_MinMaxInt(pDX, m_interval, 10, 120);
}


BEGIN_MESSAGE_MAP(CDatabaseSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDatabaseSettingsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDatabaseSettingsDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDatabaseSettingsDlg message handlers


void CDatabaseSettingsDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	///char temp[1024];
	UpdateData(TRUE);
	CopyString(pref->ip_address, m_ipaddress);
	CopyString(pref->schema, m_schema);
	CopyString(pref->username,m_username);
	CopyString(pref->password, m_password);
	CopyString(pref->port, m_port);
	///CopyString(temp, m_interval);
	pref->DBinterval = (char)m_interval; // atol(temp);
	SavePreferences(PREF_FILE, *pref, *pignore,*padd, false);

	if (pref->database)
	{
		ResetDatabase(*pref);
		ResetStatus(*pref);
		LoadDatabase(*pref);
	}
	//for (int i = 0; i < pref->num_entries; i++)
	//	pref->schedule_entry[i].status = 0;
	//SaveDatabase(*pref,true);

	CDialog::OnOK();
}


void CDatabaseSettingsDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}
