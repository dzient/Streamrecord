#pragma once
#include "afxwin.h"
#include "ircbotmfc.h"


// IRCChatDlg dialog

class IRCChatDlg : public CDialog
{
	DECLARE_DYNAMIC(IRCChatDlg)

public:
	IRCChatDlg(IrcBotMFC *irc, CWnd* pParent = NULL);   // standard constructor
	virtual ~IRCChatDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG8 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
private:
	void FillList();
public:
	afx_msg void OnBnClickedOk();
	CListBox m_irc_list;
	CString m_irc_input;
private:
	IrcBotMFC *ircptr;
	BOOL init;
public:
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnBnClickedButton1();
};
