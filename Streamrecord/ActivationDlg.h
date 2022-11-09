#pragma once


// ActivationDlg dialog

#include "loadpref.h"

class ActivationDlg : public CDialog
{
	DECLARE_DYNAMIC(ActivationDlg)

public:
	ActivationDlg(STREAMRECORD_PREFERENCES *pref, 
		CWnd* pParent = NULL);   // standard constructor
	virtual ~ActivationDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	STREAMRECORD_PREFERENCES *pref_ptr;
	BOOL got_focus;
	BOOL database;

	DECLARE_MESSAGE_MAP()
public:
	// // activation code for this program
	CString m_activation_code;
	afx_msg void OnOk();
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnEnSetfocusEdit1();
	afx_msg void OnBnClickedDatabase();
	afx_msg void OnBnClickedDatabaseSettings();
};
