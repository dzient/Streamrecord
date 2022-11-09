#pragma once
#include "afxwin.h"
#include "loadpref.h"


// IRCSettingsDlg dialog

class IRCSettingsDlg : public CDialog
{
	DECLARE_DYNAMIC(IRCSettingsDlg)

public:
	IRCSettingsDlg(STREAMRECORD_PREFERENCES *pref, CWnd* pParent = NULL);   // standard constructor
	virtual ~IRCSettingsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG7 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_servername;
	CString m_nickname;
	CString m_channel;
	CString m_uid;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
private:
	STREAMRECORD_PREFERENCES *ppref;
public:
	BOOL m_enable;
	BOOL m_ringo_mode;
	BYTE m_color;
	BYTE m_bkg_color;
	BOOL m_update_topic;
};
