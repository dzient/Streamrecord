// StopDlg.cpp : implementation file
//
//-------------------------------------------
// David Zientara
// 12-19-2022
//
// StopDlg.cpp
//
// File for the StopDlg class
//
//---------------------------------------------

#include "stdafx.h"
#include "afxmt.h"
#include "streamrecord.h"
#include "StopDlg.h"
#include ".\stopdlg.h"
#include "Parse.h"

#define TIMER_ID5	5

extern CMutex schedule_mutex;


// StopDlg dialog


IMPLEMENT_DYNAMIC(StopDlg, CDialog)
//----------------------------------------------------------
// StopDlg 
// Constructor for the StopDlg class
// PARAMS: pref (pointer to STREAMRECORD_PREFERENCES object)
// stream_araay (array od chars)
// CWnd (pointer to CWnd; parent window)
// RETURNS: Nothing; StopDlg object initialized
//-----------------------------------------------------------
StopDlg::StopDlg(const STREAMRECORD_PREFERENCES *pref, StreamPtr stream_array[], CWnd* pParent /*=NULL*/)
	: CDialog(StopDlg::IDD, pParent)
{
	ppref = (STREAMRECORD_PREFERENCES *)pref;
	init = FALSE;
	alist = new long[pref->num_entries];
	stream_list = new long[pref->num_entries];
	stream_ar = (StreamPtr *)stream_array;
	destroyed = false;
	slistsize = 0;
	update = FALSE;
}
//-------------------------------------------------------------
// ~StopDlg
// Destructor for the StopDlg class
// PARAMS: None
// RETURNS: Nothing; alist + stream_list are deleted
//-------------------------------------------------------------
StopDlg::~StopDlg()
{
	if (alist != NULL)
		delete [] alist;
	if (stream_list != NULL)
		delete [] stream_list;
	
}
//----------------------------------------------------------------
// DoDataExchange
// Function copies data from the dialog control to variable and
// vice-versa
// PARAMS: pDX (pointer to CDataExchange object)
// RETURNS: Nothing; data is copied
//-----------------------------------------------------------------
void StopDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_recording_list);
}


BEGIN_MESSAGE_MAP(StopDlg, CDialog)
	ON_LBN_DBLCLK(IDC_LIST1, OnLbnDblclkList1)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_TIMER()
	ON_WM_PAINT()
END_MESSAGE_MAP()
//----------------------------------------------------------------
// OnTimer
// Function invokes a timer at specific intervals
// PARAMS: nIDEvent (UINT_PTR): This is the unique identifier
// that identifies each timing event
// RETURNS: Nothing; switch-case statement is called
//------------------------------------------------------------------
void StopDlg::OnTimer(UINT_PTR nIDEvent)
{
	long i;

	switch(nIDEvent)
	{
		case TIMER_ID5:
			i = m_recording_list.GetCurSel();
			FillList(ppref);
			UpdateData(FALSE);
			m_recording_list.SetCurSel(i);
			break;
		default: break;
	}

	CDialog::OnTimer(nIDEvent);
}
//-----------------------------------------------------------------
// OnInitDialog
// Function is invoked when dialog box is initialized
// PARAMS: None
// RETURNS: the value of the parent class; SetTimer() is 
// initialized
//-----------------------------------------------------------------
BOOL StopDlg::OnInitDialog()
{
	SetTimer(TIMER_ID5,1500,0);
	
	return CDialog::OnInitDialog();
}
//-----------------------------------------------------------------
// FillList
// Function fills a dialog control list with the programs
// currently being recorded
// PARAMS: pref (pointer to STREAMRECORD_PREFERENCES object)
// RETURNS: Nothing; list is filled
//-----------------------------------------------------------------
void StopDlg::FillList(STREAMRECORD_PREFERENCES *pref)
{
	long i, j = 0;
	char str[1024], temp[1024];
	CString tmp;

	//return;
	while (m_recording_list.DeleteString(0) != LB_ERR);

	schedule_mutex.Lock();
	update = TRUE;
	for (i = 0; i < pref->num_entries; i++)
	{
		////CopyString(temp, stream_ar[pref->schedule_entry[i].stream_idx]->GetStatusMessage());
		if (pref->schedule_entry[i].stream_idx >= 0 && 
			pref->schedule_entry[i].stream_idx < 100 &&
			stream_ar[pref->schedule_entry[i].stream_idx] != NULL
			&& _strnicmp(temp,"IDLE",4) != 0)
		{
			alist[j] = i;
			stream_list[j] = pref->schedule_entry[i].stream_idx;
			sprintf(str,"[%d] %s",++j,pref->schedule_entry[i].program);
			CopyString(tmp, str);
			m_recording_list.AddString(tmp); //str);
		}
	}
	slistsize = j;
	update = FALSE;
	schedule_mutex.Unlock();
}
//-----------------------------------------------------------------
// OnPaint
// Function is invoked when the dialog box needs painting
// PARAMS: Nothing
// RETURNS: Nothing; FillList is invoked
//------------------------------------------------------------------

void StopDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting 

	if (!init)
	{
		init = TRUE;
		FillList(ppref);
	}
}

// StopDlg message handlers
//----------------------------------------------------------------------
// OnLbnDblclkList1
// Function is invoked when the user double clicks on a list entry
// PARAMS: Nothing
// RETURNS: Nothing; if the recording has not been terminated already,
// it will terminate the recording if the user selects Yes in a 
// dialog box
//----------------------------------------------------------------------
void StopDlg::OnLbnDblclkList1()
{
	// TODO: Add your control notification handler code here
	static BOOL busy = FALSE;
	long i, j, k, m;
	BOOL found = FALSE;
	char temp[256];

	if (!busy && !update)
	{
		busy = TRUE;
		i = m_recording_list.GetCurSel();
		if (ppref->schedule_entry[alist[i]].stream_idx != stream_list[i])
		{
			FillList(ppref);
			UpdateData(FALSE);
			for (j = 0; j < slistsize && !found; j++)
			{
				if (ppref->schedule_entry[alist[j]].stream_idx == stream_list[i])
				{
					i = j;
					found = TRUE;
				}
			}
		}
		else
		{
			found = TRUE;
		}
		if (!found || stream_ar[ppref->schedule_entry[alist[i]].stream_idx] == NULL)
		{
			MessageBoxA(NULL,LPCSTR("Recording already terminated"),PROGRAM_NAME,MB_ICONEXCLAMATION);
		}
		else if (MessageBoxA(NULL,(char *)LPCSTR("Stop recording?"),PROGRAM_NAME,MB_YESNO) == IDYES)
		{
			stream_ar[ppref->schedule_entry[alist[i]].stream_idx]->SetTerminate(TRUE);
			ppref->schedule_entry[alist[i]].stream_running = FALSE;
			ppref->schedule_entry[alist[i]].recorded = TRUE;
			ppref->schedule_entry[alist[i]].status = 3;
			found = FALSE;
			// If it's a mountpoint, we have to do things differently:
			if (ppref->schedule_entry[alist[i]].monitor_mountpoint)
			{
				for (j = 0, k = -1; j < ppref->num_entries && k == -1; j++)
					if (strncmp(ppref->schedule_entry[alist[i]].stream_URL, ppref->schedule_entry[j].stream_URL, strlen(ppref->schedule_entry[j].stream_URL)) == 0
						&& ppref->schedule_entry[j].monitor_server)
						k = j;
				if (k != -1)
				{
					for (j = 0; j < IGNORE_MP_MAX && !found; j++)
						if (strcmp(ppref->schedule_entry[k].ignore_mp[j], "") == 0)
						{
							strcpy(temp, ppref->schedule_entry[alist[i]].stream_URL);
							ParseFilename(temp);
							for (m = strlen(temp); m > 0 && !isspace(temp[m]); m--);
							temp[m] = '//';
							strcpy(temp, temp + m);
							strcpy(ppref->schedule_entry[k].ignore_mp[j], temp);
							found = TRUE;
							///SaveDatabase(*ppref, true, k);
						}
				}
				ppref->schedule_entry[alist[i]].parent = k;
			
			
				if (ppref->database)
				{
					SaveDatabase(*ppref, false, k); // alist[i]);
					strcpy(ppref->schedule_entry[alist[i]].password, "");
					Sleep(1000);
					ppref->schedule_entry[alist[i]].willpurge = 1;
				}
				
			}
			else
			{
				if (ppref->database && !ppref->schedule_entry[i].repeated)
					ppref->schedule_entry[alist[i]].willpurge = 1;
				else if (ppref->database)
					SetStatus(*ppref, alist[i]);
			}
		}
		UpdateData(FALSE);
		busy = FALSE;
	}
}
//----------------------------------------------------------------------
// OnBnClickedCancel
// Function is invoked when the user clicks the Cancel button
// PARAMS: None
// RETURNS: Nothing; KillTimer() is invoked
//-----------------------------------------------------------------------
void StopDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	SetDestroyed(true);
	KillTimer(TIMER_ID5);
	//OnCancel();
}
//----------------------------------------------------------------------
// OnBnClickedOk
// Function is invoked when Ok button is clicked
// PARAMS: None
// RETURNS: Nothing; KillTimer() is invoked
//----------------------------------------------------------------------
void StopDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	SetDestroyed(true);
	KillTimer(TIMER_ID5);
	//OnOK();
}





