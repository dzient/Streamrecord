// IRCChatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "streamrecord.h"
#include "IRCChatDlg.h"
#include ".\ircchatdlg.h"
#include "loadpref.h"


// IRCChatDlg dialog

IMPLEMENT_DYNAMIC(IRCChatDlg, CDialog)
IRCChatDlg::IRCChatDlg(IrcBotMFC *irc, CWnd* pParent /*=NULL*/)
	: CDialog(IRCChatDlg::IDD, pParent)
	, m_irc_input(_T(""))
{
	ircptr = irc;
	init = FALSE;
}

IRCChatDlg::~IRCChatDlg()
{
}

void IRCChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_irc_list);
	DDX_Text(pDX, IDC_EDIT1, m_irc_input);
}


BEGIN_MESSAGE_MAP(IRCChatDlg, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnEnKillfocusEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()


// IRCChatDlg message handlers

void IRCChatDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting 

	if (!init)
	{
		init = TRUE;
		//FillList();
	}
}

void IRCChatDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void IRCChatDlg::FillList()
{
	char buf[128];
	char *bbuf = new char[4096];
	long i, j;
	CString bbuf_str;

	for (i = 0; i < 127; i++)
		buf[i] = 'a';
	buf[127] = NULL;
	bbuf[0] = NULL;

	if (ircptr != NULL)
		while (ircptr->GetOutput(buf) >= 99)
		{
			for (i = 0, j = -1; i < (long)strlen(buf); i++)
				if (buf[i] == '\r' || buf [i] == '\n' || i > 4094)
				{
					buf[i] = NULL;
					j = i;
				}
			if (bbuf[0] == NULL)
				strcpy(bbuf,buf);
			else
				strcat(bbuf,buf);
			if (j != -1)				
			{
				CopyString(bbuf_str, bbuf);
				m_irc_list.AddString(bbuf_str);
				bbuf[0] = NULL;
				do
				{
					j++;
				} while (buf[j] == '\r' || buf[j] == '\n');
				strcpy(bbuf,buf+j);
			}
		}
	CopyString(bbuf_str, bbuf);
	m_irc_list.AddString(bbuf_str);
	UpdateData(FALSE);
	delete [] bbuf;
}

void IRCChatDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	
}

void IRCChatDlg::OnEnKillfocusEdit1()
{
	// TODO: Add your control notification handler code here
}

void IRCChatDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	char buf[1024];

	strncpy(buf,(char *)LPCTSTR(m_irc_input),1021);

	buf[strlen(buf)+2] = NULL;
	buf[strlen(buf)+1] = '\n';
	buf[strlen(buf)] = '\r';

	if (ircptr != NULL)
		ircptr->Send(buf);
	///FillList();
}
