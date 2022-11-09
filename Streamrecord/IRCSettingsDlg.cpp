// IRCSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "streamrecord.h"
#include "IRCSettingsDlg.h"
#include ".\ircsettingsdlg.h"
#include "loadpref.h"


// IRCSettingsDlg dialog

IMPLEMENT_DYNAMIC(IRCSettingsDlg, CDialog)
IRCSettingsDlg::IRCSettingsDlg(STREAMRECORD_PREFERENCES *pref, CWnd* pParent /*=NULL*/)
	: CDialog(IRCSettingsDlg::IDD, pParent)
	, m_servername(_T(""))
	, m_nickname(_T(""))
	, m_channel(_T(""))
	, m_uid(_T(""))
	, m_enable(FALSE)
	, m_ringo_mode(FALSE)
	, m_color(0)
	, m_bkg_color(0)
	, m_update_topic(FALSE)
{
	m_servername = pref->irc.server;
	m_nickname = pref->irc.nickname;
	m_channel = pref->irc.channel;
	m_uid = pref->irc.UID;
	m_enable = pref->irc.enable_posting;
	m_ringo_mode = pref->irc.ringo_mode;
	m_color = pref->irc.color;
	m_bkg_color = pref->irc.background;
	m_update_topic = pref->irc.update_topic;
	ppref = pref;
}

IRCSettingsDlg::~IRCSettingsDlg()
{
}

void IRCSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_servername);
	DDX_Text(pDX, IDC_EDIT2, m_nickname);
	DDX_Text(pDX, IDC_EDIT3, m_channel);
	DDX_Text(pDX, IDC_EDIT4, m_uid);
	DDX_Check(pDX, IDC_CHECK1, m_enable);
	DDX_Check(pDX, IDC_CHECK2, m_ringo_mode);
	DDX_Text(pDX, IDC_EDIT5, m_color);
	DDX_Text(pDX, IDC_EDIT6, m_bkg_color);
	DDX_Check(pDX, IDC_CHECK3, m_update_topic);
}


BEGIN_MESSAGE_MAP(IRCSettingsDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// IRCSettingsDlg message handlers

void IRCSettingsDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	//strcpy(ppref->irc.server,LPCTSTR(m_servername));
	//strcpy(ppref->irc.nickname,LPCTSTR(m_nickname));
	//strcpy(ppref->irc.channel,LPCTSTR(m_channel));
	//strcpy(ppref->irc.UID,LPCTSTR(m_uid));

	CopyString(ppref->irc.server, LPCTSTR(m_servername));
	CopyString(ppref->irc.nickname, LPCTSTR(m_nickname));
	CopyString(ppref->irc.channel, LPCTSTR(m_channel));
	CopyString(ppref->irc.UID, LPCTSTR(m_uid));

	ppref->irc.enable_posting = m_enable;
	ppref->irc.ringo_mode = m_ringo_mode;
	ppref->irc.color = m_color;
	ppref->irc.background = m_bkg_color;
	ppref->irc.update_topic = m_update_topic;
	OnOK();
}

void IRCSettingsDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
