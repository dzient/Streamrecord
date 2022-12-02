#pragma once
#include "afxdialogex.h"
#include "loadpref.h"


// CPushoverDlg dialog

class CPushoverDlg : public CDialog
{
	DECLARE_DYNAMIC(CPushoverDlg)

public:
	CPushoverDlg(STREAMRECORD_PREFERENCES* ppref,CWnd* pParent = nullptr);   // standard constructor
	virtual ~CPushoverDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG10 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CString m_api_key;
	CString m_api_user;
	STREAMRECORD_PREFERENCES* pref;
public:
	afx_msg void OnBnClickedOk();
};
