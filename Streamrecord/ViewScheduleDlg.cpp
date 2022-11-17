// ViewScheduleDlg.cpp : implementation file
//
//-------------------------------------------
// David Zientara
// 10-27-2022
//
// ViewScheduleDlg.cpp
//
// File for the ViewScheduleDlg class
//
//---------------------------------------------

#include "stdafx.h"
#include "ViewScheduleDlg.h"
#include "ScheduleDlg.h"
#include ".\viewscheduledlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DLG_TIMER_ID	2
#define TIMER_ID_6	5

const long NUM_LINES = 4;

/////////////////////////////////////////////////////////////////////////////
// ViewScheduleDlg dialog
//-------------------------------------------
// ConvertTimeToString
// Function takes a number from 0 to 60 and 
// converts it to string format...by adding 
// 48 to it
// PARAMS: n (DWORD), a number; ar (char array),
// a string
// RETURNS: Nothing; n is converted to string
//--------------------------------------------
void ConvertTimeToString(DWORD n, char ar[])
{
	if (n < 10)
	{
		ar[0] = '0';
		ar[1] = (char)n + 48;
	}
	else
	{
		ar[0] = (char)((n/10) + 48);
		ar[1] = (char)((n%10) + 48);
	}
	ar[2] = NULL;
}
//----------------------------------------------
// GetDays
// Function takes an unsigned char (1 to 128)
// and returns the day, in string format
// PARAMS: days (UCHAR), ar (char array)
// RETURNS: Nothing; days is converted to string
//------------------------------------------------
void GetDays(unsigned char days, char ar[])
{
	long i;
	const char days_of_week[7][12] = { "Sunday", "Monday", "Tuesday",
		"Wednesday", "Thursday", "Friday", "Saturday" };

	strcpy(ar,"");
	// days is a binary number
	// E.g. Sunday = 1; Monday = 2; etc.
	// This way, we can represent several days by 
	// OR-ing together days; e.g. 3 = Sunday + Monday

	for (i = 0; i < 7; i++)
	{	
		if ((days>>i)&0x1)
		{
			strcat(ar,days_of_week[i]);
			strcat(ar," ");
		}
	}
}
//------------------------------------------------------
// FirstDay
// Function takes an integer and returns the first day
// This is key for sorting programs
// PARAMS: n (long)
// RETURNS 1 if day is found; 0 otherwise
//------------------------------------------------------
long FirstDay(long n)
{
	long i, mask = 0x1;

	for (i = 0; i < 7; i++)
	{
		if ((n&mask) != 0)
			return (n&mask);
		else
			mask = mask<<1;
	}
	return 0;
}

//----------------------------------------------
// ViewScheduleDlg
// Constructor for ViewScheduleDlg class
// PARAMS: pref (preferences file); ignore (ignore
// list; add (schedule add list), CWnd (parent)
// RETURNS: Nothing; class is initialized
//-----------------------------------------------
ViewScheduleDlg::ViewScheduleDlg(STREAMRECORD_PREFERENCES *pref, 
								 IGNORE_LIST *ignore,
								 SCHEDULE_ADD_LIST *add,
								 CWnd* pParent /*=NULL*/)
  :CDialog(ViewScheduleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(ViewScheduleDlg)
	//}}AFX_DATA_INIT
	ppref = pref;
	pignore = ignore;
	padd = add;
	init = FALSE;
	schedule_box = NULL;

}
//----------------------------------------------
// ~ViewScheduleDlg
// Destructor for ViewScheduleDlg class
// PARAMS: None
// RETURNS: Nothing; list is deleted and 
// ScheduleDlg is destroyed
//----------------------------------------------
ViewScheduleDlg::~ViewScheduleDlg()
{
	DeleteList();

	if (schedule_box != NULL)
	{
		schedule_box->DestroyWindow();
		delete schedule_box;
	}
	
}
//----------------------------------------------
// DoDataExchange
// Function takes a pointer to CDataExchange
// and sends/receives to the control class
// PARAMS: pDX (CDataExchange pointer)
// RETURNS: Nothing
//---------------------------------------------

void ViewScheduleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ViewScheduleDlg)
	DDX_Control(pDX, IDC_LIST1, m_schedule_ls);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ViewScheduleDlg, CDialog)
	//{{AFX_MSG_MAP(ViewScheduleDlg)
	ON_WM_PAINT()
	ON_LBN_DBLCLK(IDC_LIST1, OnLbnDblclkList1)
	//}}AFX_MSG_MAP
	
	
	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_BUTTON1, OnScheduler)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ViewScheduleDlg message handlers

//---------------------------------------
// OnInitDialog
// Function is called when dialog box is
// initialized
// PARAMS: None
// RETURNS: Nothing; SetTimer is called
//---------------------------------------
BOOL ViewScheduleDlg::OnInitDialog()
{
	// TODO: Add extra initialization here
	SetTimer(DLG_TIMER_ID,1000,0);
	////SetTimer(TIMER_ID_6, 10000, 0);

	return CDialog::OnInitDialog();
}
//---------------------------------------
// FillList
// Function "fills in" a sequential list
// of programs
// PARAMS: None
// RETURNS: Nothing; list is filled in
//---------------------------------------
void ViewScheduleDlg::FillList()
{
	long i = 0, j = 0;
	char str[1024];	
	char start_hr[3], start_min[3];
	char end_hr[3], end_min[3];
	char days[100];
	const char repeat[2][10] = { "NO", "YES" };
	CString tmp;
	// Create a linked list, because it's the easiest
	// way to create a sequential list
	list = new linked_list;
	temp = list;

	if (ppref->database)
	{
		LoadDatabase(*ppref);
		CopySchedule(*ppref);
	}
	// Iterate through the whole list:
	for (i = 0; i < ppref->num_entries; i++)
	{
		if (ppref->schedule_entry[i].stream_idx != -2
			&& (ppref->schedule_entry[i].days != 0
			|| ppref->schedule_entry[i].monitor_mountpoint == 1
			|| ppref->schedule_entry[i].monitor_server == 1)
			&& ppref->schedule_entry[i].visible)
		{
			j++;
			// If it's the first list item, just insert it:
			if (list->next == NULL && list->prev == NULL && list->n == -1)
			{
				list->n = i;
			} // else if this program is scheduled before the program in the list, do something:
			else if (((ppref->schedule_entry[list->n].monitor_mountpoint == 1
				|| ppref->schedule_entry[list->n].monitor_server == 1)
				&& ppref->schedule_entry[i].monitor_mountpoint == 0
				&& ppref->schedule_entry[i].monitor_server == 0) 
				|| ((FirstDay(ppref->schedule_entry[i].days) < FirstDay(ppref->schedule_entry[list->n].days)
				|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->n].days)
				&& ppref->schedule_entry[i].start_hr < ppref->schedule_entry[list->n].start_hr)
				|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->n].days)
				&& ppref->schedule_entry[i].start_hr == ppref->schedule_entry[list->n].start_hr
				&& ppref->schedule_entry[i].start_min < ppref->schedule_entry[list->n].start_min))
				&& ppref->schedule_entry[i].monitor_mountpoint == 0
				&& ppref->schedule_entry[i].monitor_server == 0))
			{
				// While current program is scheduled before program in the list OR this is the beginning of the list, 
				// go back one item:
				while (list->prev != NULL 
					&& (((ppref->schedule_entry[list->prev->n].monitor_mountpoint == 1 
					|| ppref->schedule_entry[list->prev->n].monitor_server == 1)
					&& ppref->schedule_entry[i].monitor_mountpoint == 0
					&& ppref->schedule_entry[i].monitor_server == 0)
					||  ((FirstDay(ppref->schedule_entry[i].days) < FirstDay(ppref->schedule_entry[list->prev->n].days)
					|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->prev->n].days)
					&& ppref->schedule_entry[i].start_hr < ppref->schedule_entry[list->prev->n].start_hr)
					|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->prev->n].days)
					&& ppref->schedule_entry[i].start_hr == ppref->schedule_entry[list->prev->n].start_hr
					&& ppref->schedule_entry[i].start_min < ppref->schedule_entry[list->prev->n].start_min)))))
				{
					list = list->prev;
				}
				// If this is the beginning of the list,
				// create a new link at the beginning
				// and assign current value to it
				if (list->prev == NULL)
				{
					list->prev = new linked_list;
					list->prev->next = list;
					list = list->prev;
					list->n = i;
				} 
				else //Else create a link in the middle of the list 
				{
					// Save a pointer to the previous prev:
					temp = list->prev;
					// Create a new link:
					list->prev = new linked_list;
					// Link to the current end of list:
					list->prev->next = list;
					//Go back one and assign current value:
					list = list->prev;
					list->n = i;
					// Assign list to next of previous prev:
					temp->next = list;
					// Finally, assign temp to prev:
					list->prev = temp;
				}
			} // Else if current program is scheduled after the program in the list, do something:
			else if (((ppref->schedule_entry[i].monitor_mountpoint
				|| ppref->schedule_entry[i].monitor_server)
				|| (FirstDay(ppref->schedule_entry[i].days) >= FirstDay(ppref->schedule_entry[list->n].days)
				|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->n].days)
				&& ppref->schedule_entry[i].start_hr >= ppref->schedule_entry[list->n].start_hr)
				|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->n].days)
				&& ppref->schedule_entry[i].start_hr == ppref->schedule_entry[list->n].start_hr
				&& ppref->schedule_entry[i].start_min >= ppref->schedule_entry[list->n].start_min
				&& !ppref->schedule_entry[list->n].monitor_mountpoint))))
			{
				// While current program is scheduled after the program in the list, 
				// go forward one item:
				while (list->next != NULL
					&& (((ppref->schedule_entry[list->next->n].monitor_mountpoint == 0
					&& ppref->schedule_entry[list->next->n].monitor_server == 0
					&& (ppref->schedule_entry[i].monitor_mountpoint == 1
					|| ppref->schedule_entry[i].monitor_server == 1))
					|| (FirstDay(ppref->schedule_entry[i].days) > FirstDay(ppref->schedule_entry[list->next->n].days)
					|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->next->n].days)
					&& ppref->schedule_entry[i].start_hr > ppref->schedule_entry[list->next->n].start_hr) 
					|| (FirstDay(ppref->schedule_entry[i].days) == FirstDay(ppref->schedule_entry[list->next->n].days)
					&& ppref->schedule_entry[i].start_hr == ppref->schedule_entry[list->next->n].start_hr
					&& ppref->schedule_entry[i].start_min > ppref->schedule_entry[list->next->n].start_min
					)) && ppref->schedule_entry[list->next->n].monitor_mountpoint == 0)))
				{
					list = list->next;
				}
				// If this is the end of the list,
				// create a new link at the end
				// and assign current value to it
				if (list->next == NULL)
				{
					list->next = new linked_list;
					list->next->prev = list;
					list = list->next;
					list->n = i;
				}
				else // Else create a link in the middle of the list:
				{
					temp = list->next;
					list->next = new linked_list;
					list->next->prev = list;
					list = list->next;
					list->n = i;
					temp->prev = list;
					list->next = temp;
				}
			}			

		}
	}
	// Now that we have create the list, go back to 
	// the beginning:
	while (list->prev != NULL)
		list = list->prev;

	i = 1;
	// Now all we need to do is iterate through the linked list
	// and print it out:
	if (j > 0)
	{
		while (list != NULL && list->n < ppref->num_entries)
			//&& ppref->schedule_entry[list->n].visible)
		{		
			sprintf(str,"[%d] Program: %s",i,ppref->schedule_entry[list->n].program);
			CopyString(tmp, str);
			m_schedule_ls.AddString(tmp); //str);
			if (ppref->schedule_entry[list->n].monitor_mountpoint)
			{
				sprintf(str,"   Continuously monitored mountpoint");
				CopyString(tmp, str);
				m_schedule_ls.AddString(tmp); //str);
				sprintf(str,
					"   Shoutcast: %s Convert: %s Bitrate: %d kbps Delete original: %s",
					repeat[ppref->schedule_entry[list->n].shoutcast&0x1],
					repeat[ppref->schedule_entry[list->n].reencode&0x1],
					ppref->schedule_entry[list->n].encodebr,
					repeat[ppref->schedule_entry[list->n].delete_old&0x1]);	
				CopyString(tmp, str);
				m_schedule_ls.AddString(tmp); //str);
				CopyString(tmp, "No additional information");
				m_schedule_ls.AddString(tmp); //LPCTSTR("No additional information"));
			}
			else
			{
				ConvertTimeToString(ppref->schedule_entry[list->n].start_hr,start_hr);
				ConvertTimeToString(ppref->schedule_entry[list->n].start_min,start_min);
				ConvertTimeToString(ppref->schedule_entry[list->n].end_hr,end_hr);
				ConvertTimeToString(ppref->schedule_entry[list->n].end_min,end_min);
				GetDays(ppref->schedule_entry[list->n].days,days);
				//sprintf(str,"   Start time: %s:%s   End time: %s:%s",start_hr,
				//	start_min,end_hr,end_min);
				sprintf(str, "   Start time: %s   End time: %s", ppref->schedule_entry[list->n].starttime, ppref->schedule_entry[list->n].endtime);
				CopyString(tmp, str);
				m_schedule_ls.AddString(tmp); //str);
				sprintf(str,"   Day(s): %s ",days);
				CopyString(tmp, str);
				m_schedule_ls.AddString(tmp); //str);
				sprintf(str,
					"   Repeating: %s Shoutcast: %s Convert: %s Bitrate: %d kbps Delete original: %s",
					repeat[ppref->schedule_entry[list->n].repeated&0x1],
					repeat[ppref->schedule_entry[list->n].shoutcast&0x1],
					repeat[ppref->schedule_entry[list->n].reencode&0x1],
					ppref->schedule_entry[list->n].encodebr,
					repeat[ppref->schedule_entry[list->n].delete_old&0x1]);	
				CopyString(tmp, str);
				m_schedule_ls.AddString(tmp); //str);
			}
			i++;
			temp = list;
			list = list->next;
		}
	}

}
//--------------------------------------
// DeleteList
// Function iterates through a list
// and deletes all nodes
// PARAMS: None
// RETURNS: Nothing; all nodes in the 
// list are deleted
//--------------------------------------
void ViewScheduleDlg::DeleteList()
{
	list = temp;

	while (list != NULL)
	{
		temp = list;
		list = list->prev;
		delete temp;
	}
}
//--------------------------------------
// OnPaint
// Function invokes FillList and 
// UpdateData as the screeen is redrawn
// PARAMS: None
// RETURNS: Nothing; calls FillList + 
// UpdateData
//---------------------------------------
void ViewScheduleDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting 
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
	if (!init)
	{
		FillList();
		UpdateData(TRUE);
		init = TRUE;
	}
	
}

//-----------------------------------------
// OnLbnDblclkList1
// Function is invoked when the list is
// double-clicked; it spawns a ScheduleDlg-
// derived box representing the item clicked 
// on
// PARAMS: None
// RETURNS: Nothing; spawns a dialog box for
// the program
//------------------------------------------

void ViewScheduleDlg::OnLbnDblclkList1()
{
	// TODO: Add your control notification handler code here
	// Get an index #:
	long i, idx = m_schedule_ls.GetCurSel()/NUM_LINES;

	list = temp;
	// Find the node in the linked list:
	while (list->prev != NULL)
		list = list->prev;
	for (i = 0; i < idx; i++)
		list = list->next;
	// Now that we have found the node, spawn a dialog box
	// if schedule_box equals NULL; if not, invoke MoveToEntry:
	if (schedule_box == NULL)
	{
		schedule_box = new ScheduleDlg(ppref,list->n,pignore,padd);
		schedule_box->Create(IDD_DIALOG1);
		schedule_box->ShowWindow(SW_SHOW);
	}
	else
	{
		schedule_box->MoveToEntry(list->n);
	}

	//MessageBox(str,"SSR",MB_OK);
}
//------------------------------------------------------
// OnTimer
// Function is invoked at a specific time interval
// If schedule_box needs to be destroyed, it will be
// destroyed and deleted
// PARAMS: nIDEvent (UINT_PTR) pointer to the event
// RETURNS: Nothing
//------------------------------------------------------
void ViewScheduleDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	long i, j, k;

	switch (nIDEvent)
	{
	case TIMER_ID_6:
		if (ppref->database)
		{
			//LoadDatabase(*ppref);
			//schedule_box->SetUpdated(false);
			//FillList();
			//UpdateData(TRUE);
			//init = TRUE;
		}
			
	}

	if (schedule_box != NULL && schedule_box->IsDestroyed())
	{
		schedule_box->DestroyWindow();
		delete schedule_box;
		schedule_box = NULL;
		DeleteList();
		j = m_schedule_ls.GetCount();
		k = m_schedule_ls.GetCurSel();
		for (i = 0; i < j; i++)
			m_schedule_ls.DeleteString(0);
		FillList();
		j = m_schedule_ls.GetCount();
		if (k < j)
			m_schedule_ls.SetCurSel(k);
		else
			m_schedule_ls.SetCurSel(0);
	}
	else if (schedule_box != NULL && schedule_box->IsUpdated())
	{
		DeleteList();
		j = m_schedule_ls.GetCount();
		k = m_schedule_ls.GetCurSel();
		for (i = 0; i < j; i++)
			m_schedule_ls.DeleteString(0);
		FillList();
		j = m_schedule_ls.GetCount();
		if (k < j)
			m_schedule_ls.SetCurSel(k);
		else
			m_schedule_ls.SetCurSel(0);
		schedule_box->SetUpdated(false);
	}

	CDialog::OnTimer(nIDEvent);
}
//--------------------------------------------------
// OnOK
// Function is invoked when the OK button is pressed
// PARAMS: None
// RETURNS: Nothing; the timers are killed 
//---------------------------------------------------
void ViewScheduleDlg::OnOk()
{
	// TODO: Add your control notification handler code here
	KillTimer(DLG_TIMER_ID);
	KillTimer(TIMER_ID_6);

	CDialog::OnOK();
}
//---------------------------------------------------------
// OnScheduler
// Function is invoked when the Scheduler button is pressed
// Basically, the a ScheduleDlg-derived dialog box is 
// invoked and defaults are loaded into the box
// PARAMS: None
// RETURNS: Nothing; dialog box is spawned
//---------------------------------------------------------
void ViewScheduleDlg::OnScheduler()
{
	// TODO: Add your control notification handler code here

	if (schedule_box != NULL)
	{
		schedule_box->DestroyWindow();
		delete schedule_box;
	}
	if (schedule_box == NULL)
	{
		schedule_box = new ScheduleDlg(ppref,-1,pignore,padd);
		schedule_box->Create(IDD_DIALOG1);
		schedule_box->ShowWindow(SW_SHOW);
	}
}
