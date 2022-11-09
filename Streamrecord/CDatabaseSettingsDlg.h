#pragma once
#include "afxdialogex.h"
#include "loadpref.h"


// CDatabaseSettingsDlg dialog

class CDatabaseSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(CDatabaseSettingsDlg)

public:
	CDatabaseSettingsDlg(STREAMRECORD_PREFERENCES* ppref,
		IGNORE_LIST* ignore,
		SCHEDULE_ADD_LIST* add,
		CWnd* pParent = nullptr);   // standard constructor
	virtual ~CDatabaseSettingsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG9 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// IP address/hostname for the DB server
	CString m_ipaddress;
	//The schema containing tables:
	CString m_schema;
	// MySQL username
	CString m_username;
	// MySQL password
	CString m_password;
	// MySQL port	
	CString m_port;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	STREAMRECORD_PREFERENCES* pref;
	IGNORE_LIST* pignore;
	SCHEDULE_ADD_LIST* padd;
	// Database query interval (in seconds):
	DWORD m_interval;
};
