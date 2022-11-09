// ViewScheduleDlg.cpp : implementation file
//

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

void GetDays(unsigned char days, char ar[])
{
	long i;
	const char days_of_week[7][12] = { "Sunday", "Monday", "Tuesday",
		"Wednesday", "Thursday", "Friday", "Saturday" };

	strcpy(ar,"");
	
	for (i = 0; i < 7; i++)
	{	
		if ((days>>i)&0x1)
		{
			strcat(ar,days_of_week[i]);
			strcat(ar," ");
		}
	}
}

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

ViewScheduleDlg::~ViewScheduleDlg()
{
	DeleteList();

	if (schedule_box != NULL)
	{
		schedule_box->DestroyWindow();
		delete schedule_box;
	}
	
}


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

BOOL ViewScheduleDlg::OnInitDialog()
{
	// TODO: Add extra initialization here
	SetTimer(DLG_TIMER_ID,1000,0);
	////SetTimer(TIMER_ID_6, 10000, 0);

	return CDialog::OnInitDialog();
}
void ViewScheduleDlg::FillList()
{
	long i = 0, j = 0;
	char str[1024];	
	char start_hr[3], start_min[3];
	char end_hr[3], end_min[3];
	char days[100];
	const char repeat[2][10] = { "NO", "YES" };
	CString tmp;

	list = new linked_list;
	temp = list;

	if (ppref->database)
	{
		LoadDatabase(*ppref);
		CopySchedule(*ppref);
	}

	for (i = 0; i < ppref->num_entries; i++)
	{
		if (ppref->schedule_entry[i].stream_idx != -2
			&& (ppref->schedule_entry[i].days != 0
			|| ppref->schedule_entry[i].monitor_mountpoint == 1
			|| ppref->schedule_entry[i].monitor_server == 1)
			&& ppref->schedule_entry[i].visible)
		{
			j++;
			if (list->next == NULL && list->prev == NULL && list->n == -1)
			{
				list->n = i;
			}
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

				if (list->prev == NULL)
				{
					list->prev = new linked_list;
					list->prev->next = list;
					list = list->prev;
					list->n = i;
				} 
				else 
				{
					temp = list->prev;
					list->prev = new linked_list;
					list->prev->next = list;
					list = list->prev;
					list->n = i;
					temp->next = list;
					list->prev = temp;
				}
			} 
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

				if (list->next == NULL)
				{
					list->next = new linked_list;
					list->next->prev = list;
					list = list->next;
					list->n = i;
				}
				else
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

	while (list->prev != NULL)
		list = list->prev;

	i = 1;
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




void ViewScheduleDlg::OnLbnDblclkList1()
{
	// TODO: Add your control notification handler code here
	long i, idx = m_schedule_ls.GetCurSel()/NUM_LINES;

	list = temp;

	while (list->prev != NULL)
		list = list->prev;
	for (i = 0; i < idx; i++)
		list = list->next;
	
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

void ViewScheduleDlg::OnOk()
{
	// TODO: Add your control notification handler code here
	KillTimer(DLG_TIMER_ID);
	KillTimer(TIMER_ID_6);

	CDialog::OnOK();
}

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
