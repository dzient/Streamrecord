// ScheduleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "streamrecord.h"
#include "ScheduleDlg.h"
#include <afxmt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "StreamInstance.h"
#include ".\scheduledlg.h"
#include "MountpointDlg.h"


long MAX_SCHEDULE_ENTRIES = 1024;


#define NUM_BR				7
#define DEMO_MAX_ENTRIES	2

bool saved = false;

static char bitrate_list[NUM_BR][10] = { "128", "96", "64", "48", "32", "16", "8" };

CMutex pref_mutex;
/////////////////////////////////////////////////////////////////////////////
// ScheduleDlg dialog


ScheduleDlg::ScheduleDlg(STREAMRECORD_PREFERENCES *pref, long idx, 
						 IGNORE_LIST *ignore,
						 SCHEDULE_ADD_LIST *add,
						 CWnd* pParent /*=NULL*/)
	: CDialog(ScheduleDlg::IDD, pParent)
	, m_monitor_mountpoint(FALSE)
	, m_monitor_server(FALSE)
	, m_level_list(_T(""))
	, m_delete_this(FALSE)
	, m_timeout(0)
	, m_start_pm(FALSE)
	, m_end_pm(FALSE)
{
	long i;
	//{{AFX_DATA_INIT(ScheduleDlg)
	m_sunday = FALSE;
	m_monday = FALSE;
	m_tuesday = FALSE;
	m_wednesday = FALSE;
	m_thursday = FALSE;
	m_friday = FALSE;
	m_saturday = FALSE;
	for (i = 0; i < 7; i++)
		dow[i] = FALSE;
	m_start_hour = 0;
	m_start_minute = 0;
	m_end_hour = 0;
	m_end_minute = 0;
	m_one_time = TRUE;
	m_repeating = FALSE;
	m_program = _T("sample");
	m_stream_URL = _T("http://127.0.0.1:8000/stream");
	m_shoutcast = FALSE;
	m_encode_list = _T("96");
	m_reencode = FALSE;
	m_delete_old = FALSE;
	m_monitor_mountpoint = FALSE;
	m_monitor_server = FALSE;
	m_level_list = _T("0");
	m_timeout = 30;
	m_genre = _T("Other");
	//}}AFX_DATA_INIT

	m_delete_this = FALSE;
	
	for (i = 0; i < 3; i++)
	{
		enable_ignore[i] = 0;
		ignore_day[i] = 0;
		ignore_start_hr[i] = 0;
		ignore_start_min[i]= 0; 
		ignore_end_hr[i] = 0;
		ignore_end_min[i] = 0;
	}
	for (i = 0; i < IGNORE_MP_MAX; i++)
	{
		enable_mp_ignore[i] = 0;
		ignore_mp[i][0] = NULL;
		enable_mp_ignore2[i] = 0;
		ignore_mp2[i][0] = NULL;
		enable_mp_ignore3[i] = 0;
		ignore_mp3[i][0] = NULL;
		enable_mp_ignore3[i+6] = 0;
		ignore_mp3[i+6][0] = NULL;
		enable_mp_ignore4[i] = 0;
		ignore_mp4[i][0] = NULL;
		enable_mp_ignore4[i+6] = 0;
		ignore_mp4[i+6][0] = NULL;
		enable_mp_ignore4[i+12] = 0;
		ignore_mp4[i+12][0] = NULL;
		enable_mp_ignore4[i+18] = 0;
		ignore_mp4[i+18][0] = NULL;
	}
	for (i = 0; i < IGNORE_EXT_MAX; i++)
	{
		ignore_ext[i][0] = NULL;
	}
	repeating = FALSE;
	ppref = pref;
	pignore = ignore;
	padd = add;
	if (ppref->num_entries > 0)
	{
		pref_counter = ppref->num_entries;
		cur_idx = pref_counter-1;
		while (cur_idx >= 0 && !ppref->schedule_entry[cur_idx].visible)
			cur_idx--;
		if (cur_idx >= 0)
		{
			if (idx >= 0 && idx <= cur_idx && ppref->schedule_entry[idx].visible)
			{
				CopyScheduleInfo(ppref,padd,idx);
				cur_idx = idx;
			}
			else if (ppref->schedule_entry[cur_idx].visible)
			{
				CopyScheduleInfo(ppref,padd,cur_idx);
			}
		}
		else
		{
			cur_idx = 0;
		}
	}
	else 
	{
		cur_idx = 0;
	}
	ConsolidateSchedule(ppref,padd,-2);
	entry_changed = false;
	SetDestroyed(false);	
	SetUpdated(false);
	saved = false;
}




void ScheduleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ScheduleDlg)
	DDX_Check(pDX, IDC_CHECK1, dow[0]);
	DDX_Check(pDX, IDC_CHECK2, dow[1]);
	DDX_Check(pDX, IDC_CHECK3, dow[2]);
	DDX_Check(pDX, IDC_CHECK4, dow[3]);
	DDX_Check(pDX, IDC_CHECK5, dow[4]);
	DDX_Check(pDX, IDC_CHECK6, dow[5]);
	DDX_Check(pDX, IDC_CHECK7, dow[6]);
	DDX_Check(pDX, IDC_CHECK8, m_shoutcast);
	DDX_Text(pDX, IDC_EDIT1, m_start_hour);
	DDV_MinMaxInt(pDX, m_start_hour, 0, 23);
	DDX_Text(pDX, IDC_EDIT2, m_start_minute);
	DDV_MinMaxInt(pDX, m_start_minute, 0, 59);
	DDX_Text(pDX, IDC_EDIT3, m_end_hour);
	DDV_MinMaxInt(pDX, m_end_hour, 0, 23);
	DDX_Text(pDX, IDC_EDIT4, m_end_minute);
	DDV_MinMaxInt(pDX, m_end_minute, 0, 59);
	DDX_Check(pDX, IDC_RADIO1, m_one_time);
	DDX_Check(pDX, IDC_RADIO2, m_repeating);
	DDX_Text(pDX, IDC_EDIT5, m_program);
	DDX_Text(pDX, IDC_EDIT6, m_stream_URL);
	DDX_CBString(pDX, IDC_COMBO1, m_encode_list);
	DDX_Check(pDX, IDC_CHECK9, m_reencode);
	DDX_Check(pDX, IDC_CHECK10, m_delete_old);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK11, m_monitor_mountpoint);
	DDX_Check(pDX, IDC_CHECK12, m_monitor_server);
	DDX_CBString(pDX, IDC_COMBO3, m_level_list);
	DDX_Check(pDX, IDC_CHECK15, m_delete_this);
	DDX_Text(pDX, IDC_EDIT7, m_timeout);
	DDX_CBString(pDX, IDC_COMBO4, m_genre);
	DDX_Check(pDX, IDC_CHECK16, m_start_pm);
	DDX_Check(pDX, IDC_CHECK17, m_end_pm);
}


BEGIN_MESSAGE_MAP(ScheduleDlg, CDialog)
	//{{AFX_MSG_MAP(ScheduleDlg)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON1, OnAdd)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_CHECK2, OnCheck2)
	ON_BN_CLICKED(IDC_CHECK3, OnCheck3)
	ON_BN_CLICKED(IDC_CHECK4, OnCheck4)
	ON_BN_CLICKED(IDC_CHECK5, OnCheck5)
	ON_BN_CLICKED(IDC_CHECK6, OnCheck6)
	ON_BN_CLICKED(IDC_CHECK7, OnCheck7)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_EN_KILLFOCUS(IDC_EDIT3, OnKillfocusEdit3)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusEdit4)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnKillfocusEdit5)
	ON_EN_KILLFOCUS(IDC_EDIT6, OnKillfocusEdit6)
	ON_BN_CLICKED(IDC_BUTTON2, OnViewSchedule)
	ON_BN_CLICKED(IDC_BUTTON3, OnBack)
	ON_BN_CLICKED(IDC_BUTTON4, OnForward)
	ON_BN_CLICKED(IDC_BUTTON5, OnEditCurrent)
	ON_BN_CLICKED(IDC_BUTTON6, OnDelete)
	ON_BN_CLICKED(IDC_CHECK8, OnCheck8)
	ON_BN_CLICKED(IDC_CHECK9, OnCheck9)
	ON_BN_CLICKED(IDC_CHECK10, OnCheck10)
	ON_CBN_KILLFOCUS(IDC_COMBO1, OnKillfocusCombo1)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK11, OnBnClickedCheck11)
	ON_BN_CLICKED(IDC_BUTTON7, OnAdvancedMountpointOptions)
	ON_BN_CLICKED(IDC_CHECK12, OnBnClickedCheck12)
	ON_CBN_KILLFOCUS(IDC_COMBO3, OnCbnKillfocusCombo3)
	ON_BN_CLICKED(IDC_CHECK15, OnBnClickedCheck15)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnEnKillfocusEdit7)
	ON_CBN_KILLFOCUS(IDC_COMBO4, OnCbnKillfocusCombo4)
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ScheduleDlg message handlers

void ScheduleDlg::OnAdd() 
{
	// TODO: Add your control notification handler code here
	long i = 0, j, k, m;
	SCHEDULE *temp;
	char str[128], tmp[1024];
	////CString tmp;
	/////////UpdateData(TRUE);
	ppref->no_load = true;
	pref_mutex.Lock();
	ppref->schedule_entry[ppref->num_entries].stream_idx = -2;

	if (dow[0] == 0 && dow[1] == 0 && dow[2] == 0 && dow[3] == 0 
		&& dow[4] == 0 && dow[5] == 0 && dow[6] == 0
		&& !m_monitor_mountpoint
		&& !m_monitor_server)
	{
		sprintf(str,"You must select at least one day for recording or select monitor mountpoint/monitor server.");
		MessageBoxA(NULL,LPCSTR(str),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
	}
	else if (!ppref->code_is_good && ppref->num_entries >= DEMO_MAX_ENTRIES)
	{
		sprintf(str,"Demo version limited to %d schedule entries.",
			DEMO_MAX_ENTRIES);
		MessageBoxA(NULL,LPCSTR(str),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
	}
	else if (ppref->num_entries >= MAX_SCHEDULE_ENTRIES)
	{
		temp = new SCHEDULE[MAX_SCHEDULE_ENTRIES*2];
		if (temp != NULL)
		{
			memcpy(temp,ppref->schedule_entry,
				sizeof(SCHEDULE)*MAX_SCHEDULE_ENTRIES);
			delete [] ppref->schedule_entry;
			ppref->schedule_entry = temp;
			MAX_SCHEDULE_ENTRIES *= 2;
			for (i = MAX_SCHEDULE_ENTRIES/2; 
				i < MAX_SCHEDULE_ENTRIES; i++)
			{
				ppref->schedule_entry[i].thread_ptr = NULL;
				ppref->schedule_entry[i].stream_running = FALSE;
				ppref->schedule_entry[i].stream_idx = -2;
				memset(ppref->schedule_entry[i].oldbuf,0,BUFFERSIZE);
			}
		}
		else
		{
			MessageBoxA(NULL,LPCSTR("Memory allocation failed.\nCannot add more schedule entries."),
				PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
		}
	}
	
	if (ppref->num_entries < MAX_SCHEDULE_ENTRIES
		&& (ppref->code_is_good || ppref->num_entries < DEMO_MAX_ENTRIES)
		&& ((dow[0] + dow[1] + dow[2] + dow[3] + dow[4] + dow[5] + dow[6]) > 0
		|| m_monitor_mountpoint || m_monitor_server))
	{
		i = 0;
		do
		{
			if (ppref->schedule_entry[i].stream_idx == -2)
			{
				ppref->schedule_entry[i].schedule_id = 0;
				ppref->schedule_entry[i].days = dow[0] + dow[1]*2
					+ dow[2]*4 + dow[3]*8 + dow[4]*16 + dow[5]*32 
					+ dow[6]*64;
				
				if (m_start_hour < 12)
					ppref->schedule_entry[i].start_hr = m_start_hour + m_start_pm * 12;
				else if (m_start_hour == 12 && !m_start_pm)
					m_start_hour = 0;
				else
					ppref->schedule_entry[i].start_hr = m_start_hour;
				ppref->schedule_entry[i].start_min = m_start_minute;
				if (m_end_hour < 12)
					ppref->schedule_entry[i].end_hr = m_end_hour + m_end_pm * 12;
				else if (m_end_hour == 12 && !m_end_pm)
					m_end_hour = 0;
				else
					ppref->schedule_entry[i].end_hr = m_end_hour;
				
				ppref->schedule_entry[i].end_min = m_end_minute;
				UpdateTime(*ppref, i);
				CopyString(ppref->schedule_entry[i].program, m_program, 255);
				//strncpy(ppref->schedule_entry[i].program,LPCTSTR(m_program),255);
				ppref->schedule_entry[i].repeated = m_repeating;
				CopyString(ppref->schedule_entry[i].stream_URL, m_stream_URL, 255);
				//strncpy(ppref->schedule_entry[i].stream_URL,LPCTSTR(m_stream_URL),255);
				ppref->schedule_entry[i].stream_idx = -1;
				ppref->schedule_entry[i].shoutcast = m_shoutcast;
				CopyString(tmp, m_encode_list);
				ppref->schedule_entry[i].encodebr = (short)atol(tmp); //LPCTSTR(m_encode_list));
				ppref->schedule_entry[i].reencode = m_reencode;
				ppref->schedule_entry[i].delete_old = m_delete_old;
				ppref->schedule_entry[i].visible = 1;
				ppref->schedule_entry[i].monitor_mountpoint = m_monitor_mountpoint;
				ppref->schedule_entry[i].monitor_server = m_monitor_server;
				CopyString(tmp, m_level_list);
				ppref->schedule_entry[i].monitor_level = (char)atol(tmp); //LPCTSTR(m_level_list));
				ppref->schedule_entry[i].delete_this = m_delete_this;
				ppref->schedule_entry[i].limited_retry = FALSE;
				for (k = 0, m = 0; k < (signed)strlen(ppref->schedule_entry[i].program); k++)
					m += ppref->schedule_entry[i].program[k];
				ppref->schedule_entry[i].id = m%100001;
				for (j = 0; j < 3; j++)
				{
					ppref->schedule_entry[i].enable_ignore[j] = enable_ignore[j];
					ppref->schedule_entry[i].ignore_day[j] = ignore_day[j];
					ppref->schedule_entry[i].ignore_start_hr[j] = ignore_start_hr[j];
					ppref->schedule_entry[i].ignore_start_min[j] = ignore_start_min[j];
					ppref->schedule_entry[i].ignore_end_hr[j] = ignore_end_hr[j];
					ppref->schedule_entry[i].ignore_end_min[j] = ignore_end_min[j];
				}
				for (j = 0; j < IGNORE_MP_MAX; j++)		
				{
					ppref->schedule_entry[i].enable_mp_ignore[j] = enable_mp_ignore[j];
					if (j < 6)
						strncpy(ppref->schedule_entry[i].ignore_mp[j],ignore_mp[j],16);
					else
						strncpy(ppref->schedule_entry[i].ignore_mp[j], ignore_mp2[j], 16);

					padd->schedule_entry[i].enable_mp_ignore[j] = enable_mp_ignore2[j];
					strncpy(padd->schedule_entry[i].ignore_mp[j],ignore_mp2[j],16);
				}
				for (j = 0; j < IGNORE_MP_MAX*2; j++)
				{
					padd->schedule_entry[i].enable_mp_ignore2[j] = enable_mp_ignore3[j];
					strncpy(padd->schedule_entry[i].ignore_mp2[j],ignore_mp3[j],16);
				}
				for (j = 0; j < IGNORE_MP_MAX*4; j++)
				{
					padd->schedule_entry[i].enable_mp_ignore3[j] = enable_mp_ignore4[j];
					strncpy(padd->schedule_entry[i].ignore_mp3[j],ignore_mp4[j],32);
				}
				for (j = 0; j < IGNORE_EXT_MAX; j++)
				{
					strncpy(padd->schedule_entry[i].ignore_ext[j],ignore_ext[j],5);
				}
				
				ppref->schedule_entry[i].record_now = FALSE;
				ppref->schedule_entry[i].recorded = FALSE;
				ppref->schedule_entry[i].timeout = m_timeout;
				ppref->schedule_entry[i].visible = TRUE;
				ppref->schedule_entry[i].fail_count = 0;
				CopyString(tmp, m_genre);
				if (strcmp(tmp,"Podcast") == 0)
					ppref->schedule_entry[i].genre = 1;
				else if (strcmp(tmp, "Talk") == 0)
					ppref->schedule_entry[i].genre = 2;
				else if (strcmp(tmp, "Music") == 0)
					ppref->schedule_entry[i].genre = 3;
				else if (strcmp(tmp, "Sports") == 0)
					ppref->schedule_entry[i].genre = 4;
				else if (strcmp(tmp, "Other") == 0)
					ppref->schedule_entry[i].genre = 5;
				memset(ppref->schedule_entry[i].reserved,0,50);
				if (i >= ppref->num_entries)
					ppref->num_entries++;
				cur_idx = i;
				i = -1;
			}
			else
			{
				i++;	
			}		
		} while (i >= 0);
		if (ppref->database) // && !saved)
		{
			SaveDatabase(*ppref,false,cur_idx); // , true);
			saved = true;
		}
		pref_mutex.Unlock();
		
		ppref->no_load = false;
		MessageBoxA(NULL,LPCSTR("New schedule entry added."),PROGRAM_NAME,
			MB_OK|MB_ICONINFORMATION);

		SetUpdated(true);
	}
	else
	{
		ppref->no_load = false;
		pref_mutex.Unlock();
	}
}

void ScheduleDlg::OnCheck1() 
{
	// TODO: Add your control notification handler code here
	//m_sunday = !m_sunday;
	dow[0] = !dow[0];
	UpdateData(FALSE);
	entry_changed = true;
	
}

void ScheduleDlg::OnCheck2() 
{
	// TODO: Add your control notification handler code here
	//m_monday = !m_monday;
	dow[1] = !dow[1];
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck3() 
{
	// TODO: Add your control notification handler code here
	//m_tuesday = !m_tuesday;
	dow[2] = !dow[2];
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck4() 
{
	// TODO: Add your control notification handler code here
	//m_wednesday = !m_wednesday;
	dow[3] = !dow[3];
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck5() 
{
	// TODO: Add your control notification handler code here
	//m_thursday = !m_thursday;
	dow[4] = !dow[4];
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck6() 
{
	// TODO: Add your control notification handler code here
	//m_friday = !m_friday;
	dow[5] = !dow[5];
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck7() 
{
	// TODO: Add your control notification handler code here
	//m_saturday = !m_saturday;
	dow[6] = !dow[6];
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck8()
{
	m_shoutcast = !m_shoutcast;
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck9()
{
	m_reencode = !m_reencode;
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnCheck10()
{
	m_delete_old = !m_delete_old;
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	entry_changed = true;
}

void ScheduleDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	entry_changed = true;
}

void ScheduleDlg::OnKillfocusEdit3() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	entry_changed = true;
}

void ScheduleDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	entry_changed = true;
}

void ScheduleDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	m_one_time = TRUE;
	m_repeating = FALSE;
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	m_repeating = TRUE;
	m_one_time = FALSE;
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnKillfocusEdit5() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	entry_changed = true;
}

void ScheduleDlg::OnOK() 
{
	// TODO: Add extra validation here
	//CDialog::OnOK();
	if (ppref->num_entries > 0)
		CommitChanges(FALSE);
	///if (!ppref->database)
	///	SavePreferences(PREF_FILE,*ppref,*pignore,*padd);
	SetDestroyed(true);
}

void ScheduleDlg::OnKillfocusEdit6() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);	
	entry_changed = true;
}

void ScheduleDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	//ppref->num_entries = pref_counter;
	SetDestroyed(true);
	//CDialog::OnCancel();
}

void ScheduleDlg::OnViewSchedule() 
{
	// TODO: Add your control notification handler code here
	ViewScheduleDlg box(ppref,pignore,padd);

	box.DoModal();
}

void ScheduleDlg::OnBack() 
{
	// TODO: Add your control notification handler code here
	if (cur_idx > 0)
	{
		CommitChanges(FALSE);	
		do
		{
			cur_idx--;
		} while (cur_idx > 0 && !ppref->schedule_entry[cur_idx].visible);
		if (ppref->schedule_entry[cur_idx].visible)
			CopyScheduleInfo(ppref,padd,cur_idx);
		DisableControls();
		UpdateData(FALSE);
		entry_changed = false;
	}
}

void ScheduleDlg::OnForward() 
{
	// TODO: Add your control notification handler code here
	if (cur_idx < ppref->num_entries-1)
	{
		//CommitChanges(FALSE);
		CommitChanges(FALSE);	
		do
		{
			cur_idx++;
		} while (cur_idx < ppref->num_entries-1
			&& !ppref->schedule_entry[cur_idx].visible);
		if (ppref->schedule_entry[cur_idx].visible)
			CopyScheduleInfo(ppref,padd,cur_idx);
		DisableControls();
		UpdateData(FALSE);
		entry_changed = false;
	}
	
}

void ScheduleDlg::OnEditCurrent() 
{
	// TODO: Add your control notification handler code here	
	
	saved = false;
	UpdateData(TRUE);
	CommitChanges(TRUE);
	
	//SaveDatabase(*ppref);
	///LoadDatabase(*ppref);
}

void ScheduleDlg::CopyScheduleInfo(const STREAMRECORD_PREFERENCES *pref, 
								   const SCHEDULE_ADD_LIST *add, long i)
{
	long j;
	char tmp[16];

	m_start_hour = pref->schedule_entry[i].start_hr;
	m_start_minute = pref->schedule_entry[i].start_min;
	m_end_hour = pref->schedule_entry[i].end_hr;
	m_end_minute = pref->schedule_entry[i].end_min;
	m_repeating = pref->schedule_entry[i].repeated;
	m_one_time = !m_repeating;
	m_program = pref->schedule_entry[i].program;
	m_stream_URL = pref->schedule_entry[i].stream_URL;
	m_shoutcast = pref->schedule_entry[i].shoutcast;
	switch (pref->schedule_entry[i].genre)
	{
	case 1: m_genre = _T("Podcast");
		break;
	case 2: m_genre = _T("Talk");
		break;
	case 3: m_genre = _T("Music");
		break;
	case 4: m_genre = _T("Sports");
		break;
	case 5:
	default:
		m_genre = _T("Other");
		break;
	}
	m_start_pm = m_end_pm = false;
	if (m_start_hour >= 12)
		m_start_pm = true;
	if (m_end_hour >= 12)
		m_end_pm = true;

	if (m_start_hour > 12)
		m_start_hour -= 12;
	if (m_start_hour == 0 && !m_start_pm)
		m_start_hour = 12;
	if (m_end_hour > 12)
		m_end_hour -= 12;
	if (m_end_hour == 0 && !m_end_pm)
		m_end_hour = 12;

	

	for (j = 0; j < 7; j++)
	{
		dow[j] = (pref->schedule_entry[i].days>>j)&0x1;
	}

	_itoa(pref->schedule_entry[i].encodebr,tmp,10);
	m_encode_list = tmp;
	m_reencode = pref->schedule_entry[i].reencode;
	m_delete_old = pref->schedule_entry[i].delete_old;
	m_monitor_mountpoint = pref->schedule_entry[i].monitor_mountpoint;
	m_delete_this = pref->schedule_entry[i].delete_this;

	for (j = 0; j < 3; j++)
	{
		enable_ignore[j] = pref->schedule_entry[i].enable_ignore[j];
		ignore_day[j] = pref->schedule_entry[i].ignore_day[j];
		ignore_start_hr[j] = pref->schedule_entry[i].ignore_start_hr[j];
		ignore_start_min[j]= pref->schedule_entry[i].ignore_start_min[j];  
		ignore_end_hr[j] = pref->schedule_entry[i].ignore_end_hr[j];
		ignore_end_min[j] = pref->schedule_entry[i].ignore_end_min[j];
	}

	for (j = 0; j < IGNORE_MP_MAX; j++)
	{
		enable_mp_ignore[j] = pref->schedule_entry[i].enable_mp_ignore[j];
		strncpy(ignore_mp[j],pref->schedule_entry[i].ignore_mp[j],16);
	}

	for (j = 0; j < IGNORE_MP_MAX; j++)
	{
		enable_mp_ignore2[j] = add->schedule_entry[i].enable_mp_ignore[j];
		strncpy(ignore_mp2[j],add->schedule_entry[i].ignore_mp[j],16);
	}

	for (j = 0; j < IGNORE_MP_MAX*2; j++)
	{
		enable_mp_ignore3[j] = add->schedule_entry[i].enable_mp_ignore2[j];
		strncpy(ignore_mp3[j],add->schedule_entry[i].ignore_mp2[j],16);
	}
	for (j = 0; j < IGNORE_MP_MAX*4; j++)
	{
		enable_mp_ignore4[j] = padd->schedule_entry[i].enable_mp_ignore3[j];
		strncpy(ignore_mp4[j],padd->schedule_entry[i].ignore_mp3[j],32);
	}
	for (j = 0; j < IGNORE_EXT_MAX; j++)
	{
		strncpy(ignore_ext[j],padd->schedule_entry[i].ignore_ext[j],5);
	}

	m_monitor_server = pref->schedule_entry[i].monitor_server;
	_itoa(pref->schedule_entry[i].monitor_level,tmp,10);
	m_timeout = pref->schedule_entry[i].timeout;
	m_level_list = tmp;
}

void ScheduleDlg::ConsolidateSchedule(STREAMRECORD_PREFERENCES *pref, 
									  SCHEDULE_ADD_LIST *add, long idx)
{
	long i = 0, j, k, m;

	pref_mutex.Lock();
	if (pref->num_entries > 0)
	{
		while (i < pref->num_entries)
		{
			if (pref->schedule_entry[i].stream_idx == -2)
			{
				if (pref->schedule_entry[i].stream_running)
				{
					pref->schedule_entry[i].stream_idx = idx;
					pref->schedule_entry[i].repeated = FALSE;
					pref->schedule_entry[i].visible = 0;
				}
				else
				{
					m = pignore->num_entries;
					for (j = 0; j < m; j++)
						if (pignore->ignore_entry[j].entry_num == pref->schedule_entry[i].id)
						{
							for (k = j+1; k < pignore->num_entries; k++)
								pignore->ignore_entry[k-1] = pignore->ignore_entry[k];
							pignore->num_entries--;
						}
					for (j = i+1; j < pref->num_entries; j++)
						pref->schedule_entry[j-1] = pref->schedule_entry[j];
					pref->num_entries--;
					for (j = i+1; j < pref->num_entries; j++)
						add->schedule_entry[j-1] = add->schedule_entry[j];
					add->num_entries--;					
				}
			}
			i++;
		}
		if (cur_idx > pref->num_entries-1 
			&& pref->num_entries > 0)
		{
			cur_idx = pref->num_entries-1;
			CopyScheduleInfo(pref,padd,cur_idx);
		}
	}
	pref_mutex.Unlock();
}

void ScheduleDlg::OnDelete()
{
	long stream_idx;


	if (ppref->schedule_entry[cur_idx].thread_ptr != NULL)
	{
		MessageBoxA(NULL, LPCSTR("Cannot delete a program currently being recorded."), PROGRAM_NAME,
			MB_OK | MB_ICONEXCLAMATION);
	}
	else
	{

		pref_mutex.Lock();
		stream_idx = ppref->schedule_entry[cur_idx].stream_idx;
		ppref->schedule_entry[cur_idx].stream_idx = -2;
		if (ppref->database)
		{
			DeleteDatabase(ppref->schedule_entry[cur_idx].schedule_id, *ppref);
			//LoadDatabase(*ppref);
		}
		ConsolidateSchedule(ppref, padd, stream_idx);
		UpdateData(FALSE);
		pref_mutex.Unlock();

		MessageBoxA(NULL, LPCSTR("Current entry deleted."), PROGRAM_NAME,
			MB_OK | MB_ICONINFORMATION);
		CopyScheduleInfo(ppref, padd, cur_idx);

		UpdateData(FALSE);
		SetUpdated(true);
	}
}

void ScheduleDlg::UpdateTime(STREAMRECORD_PREFERENCES& pref, int idx)
{
	if (pref.schedule_entry[idx].start_min < 10 && pref.schedule_entry[idx].start_hr < 12)
		sprintf(pref.schedule_entry[idx].starttime, "%d:0%d AM", pref.schedule_entry[idx].start_hr, pref.schedule_entry[idx].start_min);
	else if (pref.schedule_entry[idx].start_min < 10 && pref.schedule_entry[idx].start_hr == 12)
		sprintf(pref.schedule_entry[idx].starttime, "%d:0%d PM", pref.schedule_entry[idx].start_hr, pref.schedule_entry[idx].start_min);
	else if (pref.schedule_entry[idx].start_min < 10 && pref.schedule_entry[idx].start_hr > 12)
		sprintf(pref.schedule_entry[idx].starttime, "%d:0%d PM", pref.schedule_entry[idx].start_hr-12, pref.schedule_entry[idx].start_min);
	else if (pref.schedule_entry[idx].start_min >= 10 && pref.schedule_entry[idx].start_hr < 12)
		sprintf(pref.schedule_entry[idx].starttime, "%d:%d AM", pref.schedule_entry[idx].start_hr, pref.schedule_entry[idx].start_min);
	else if (pref.schedule_entry[idx].start_min >= 10 && pref.schedule_entry[idx].start_hr == 12)
		sprintf(pref.schedule_entry[idx].starttime, "%d:%d PM", pref.schedule_entry[idx].start_hr, pref.schedule_entry[idx].start_min);
	else if (pref.schedule_entry[idx].start_min >= 10 && pref.schedule_entry[idx].start_hr >= 12)
		sprintf(pref.schedule_entry[idx].starttime, "%d:%d PM", pref.schedule_entry[idx].start_hr-12, pref.schedule_entry[idx].start_min);

	if (pref.schedule_entry[idx].end_min < 10 && pref.schedule_entry[idx].end_hr < 12)
		sprintf(pref.schedule_entry[idx].endtime, "%d:0%d AM", pref.schedule_entry[idx].end_hr, pref.schedule_entry[idx].end_min);
	else if (pref.schedule_entry[idx].end_min < 10 && pref.schedule_entry[idx].end_hr == 12)
		sprintf(pref.schedule_entry[idx].endtime, "%d:0%d PM", pref.schedule_entry[idx].end_hr, pref.schedule_entry[idx].end_min);
	else if (pref.schedule_entry[idx].end_min < 10 && pref.schedule_entry[idx].end_hr > 12)
		sprintf(pref.schedule_entry[idx].endtime, "%d:0%d PM", pref.schedule_entry[idx].end_hr-12, pref.schedule_entry[idx].end_min);
	else if (pref.schedule_entry[idx].end_min >= 10 && pref.schedule_entry[idx].end_hr < 12)
		sprintf(pref.schedule_entry[idx].endtime, "%d:%d AM", pref.schedule_entry[idx].end_hr, pref.schedule_entry[idx].end_min);
	else if (pref.schedule_entry[idx].end_min >= 10 && pref.schedule_entry[idx].end_hr == 12)
		sprintf(pref.schedule_entry[idx].endtime, "%d:%d PM", pref.schedule_entry[idx].end_hr, pref.schedule_entry[idx].end_min);
	else if (pref.schedule_entry[idx].end_min >= 10 && pref.schedule_entry[idx].end_hr >= 12)
		sprintf(pref.schedule_entry[idx].endtime, "%d:%d PM", pref.schedule_entry[idx].end_hr-12, pref.schedule_entry[idx].end_min);


}

void ScheduleDlg::CommitChanges(BOOL dialog_box)
{
	bool change = true;
	long i, j, k;
	char temp[1024];

	if (ppref->schedule_entry[cur_idx].stream_running && entry_changed)
	{
		if (MessageBoxA(NULL,LPCSTR("You are making changes to an entry\nfor a program currently being recorded. Continue?"),
			PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION|MB_YESNO) == IDNO)
		{
			change = false;
		}
	}
	
	if (change)
	{
		pref_mutex.Lock();
		ppref->schedule_entry[cur_idx].days = dow[0] + dow[1]*2
			+ dow[2]*4 + dow[3]*8 + dow[4]*16 + dow[5]*32 
			+ dow[6]*64;
		
		if (m_start_hour == 12 && !m_start_pm)
			m_start_hour = 0;
		if (m_start_hour < 12)
			ppref->schedule_entry[cur_idx].start_hr = m_start_hour + m_start_pm*12;
		else
			ppref->schedule_entry[cur_idx].start_hr = m_start_hour;
		ppref->schedule_entry[cur_idx].start_min = m_start_minute;
		if (m_end_hour == 12 && !m_end_pm)
			m_end_hour = 0;
		if (m_end_hour < 12)
			ppref->schedule_entry[cur_idx].end_hr = m_end_hour + m_end_pm*12;
		else
			ppref->schedule_entry[cur_idx].end_hr = m_end_hour;
		
		ppref->schedule_entry[cur_idx].end_min = m_end_minute;
		UpdateTime(*ppref, cur_idx);
		//strncpy(ppref->schedule_entry[cur_idx].program,LPCTSTR(m_program),255);
		CopyString(ppref->schedule_entry[cur_idx].program, m_program, 255);
		ppref->schedule_entry[cur_idx].repeated = m_repeating;
		//strncpy(ppref->schedule_entry[cur_idx].stream_URL,LPCTSTR(m_stream_URL),255);
		CopyString(ppref->schedule_entry[cur_idx].stream_URL, m_stream_URL);
		ppref->schedule_entry[cur_idx].shoutcast = m_shoutcast;
		CopyString(temp, m_genre);
		ppref->schedule_entry[cur_idx].genre = (char)atol(temp);
		CopyString(temp, m_encode_list);
		ppref->schedule_entry[cur_idx].encodebr = (short)atol(temp); //LPCTSTR(m_encode_list));
		ppref->schedule_entry[cur_idx].reencode = m_reencode;
		ppref->schedule_entry[cur_idx].delete_old = m_delete_old;
		ppref->schedule_entry[cur_idx].monitor_mountpoint = m_monitor_mountpoint;
		ppref->schedule_entry[cur_idx].monitor_server = m_monitor_server;
		CopyString(temp, m_level_list);
		ppref->schedule_entry[cur_idx].monitor_level = (char)atol(temp); //LPCTSTR(m_level_list));
		ppref->schedule_entry[cur_idx].limited_retry = FALSE;
		ppref->schedule_entry[cur_idx].delete_this = m_delete_this;
		for (j = 0, k = 0; j < (signed)strlen(ppref->schedule_entry[cur_idx].program); j++)
			k += ppref->schedule_entry[cur_idx].program[j];
		ppref->schedule_entry[cur_idx].id = k%100001;

		char temp[1024];

		CopyString(temp,m_genre);

		if (_stricmp(temp, "Podcast") == 0)
			ppref->schedule_entry[cur_idx].genre = 1;
		else if (_stricmp(temp, "Talk") == 0)
			ppref->schedule_entry[cur_idx].genre = 2;
		else if (_stricmp(temp, "Music") == 0)
			ppref->schedule_entry[cur_idx].genre = 3;
		else if (_stricmp(temp, "Sports") == 0)
			ppref->schedule_entry[cur_idx].genre = 4;
		else if (_stricmp(temp, "Other") == 0)
			ppref->schedule_entry[cur_idx].genre = 5;

		for (i = 0; i < 3; i++)
		{
			ppref->schedule_entry[cur_idx].enable_ignore[i] = enable_ignore[i];
			ppref->schedule_entry[cur_idx].ignore_day[i] = ignore_day[i];
			ppref->schedule_entry[cur_idx].ignore_start_hr[i] = ignore_start_hr[i];
			ppref->schedule_entry[cur_idx].ignore_start_min[i] = ignore_start_min[i];
			ppref->schedule_entry[cur_idx].ignore_end_hr[i] = ignore_end_hr[i];
			ppref->schedule_entry[cur_idx].ignore_end_min[i] = ignore_end_min[i];
		}

		for (j = 0; j < IGNORE_MP_MAX; j++)
		{
			ppref->schedule_entry[cur_idx].enable_mp_ignore[j] = enable_mp_ignore[j];
			strncpy(ppref->schedule_entry[cur_idx].ignore_mp[j],ignore_mp[j],16);

			padd->schedule_entry[cur_idx].enable_mp_ignore[j] = enable_mp_ignore2[j];
			strncpy(padd->schedule_entry[cur_idx].ignore_mp[j],ignore_mp2[j],16);
		}

		for (j = 0; j < IGNORE_MP_MAX*2; j++)
		{
			padd->schedule_entry[cur_idx].enable_mp_ignore2[j] = enable_mp_ignore3[j];
			strncpy(padd->schedule_entry[cur_idx].ignore_mp2[j],ignore_mp3[j],16);
		}

		for (j = 0; j < IGNORE_MP_MAX*4; j++)
		{
			padd->schedule_entry[cur_idx].enable_mp_ignore3[j] = enable_mp_ignore4[j];
			strncpy(padd->schedule_entry[cur_idx].ignore_mp3[j],ignore_mp4[j],32);
		}
		for (j = 0; j < IGNORE_EXT_MAX; j++)
		{
			strncpy(padd->schedule_entry[cur_idx].ignore_ext[j],ignore_ext[j],5);
		}
		
		ppref->schedule_entry[cur_idx].record_now = FALSE;
		ppref->schedule_entry[cur_idx].recorded = FALSE;
		ppref->schedule_entry[cur_idx].timeout = m_timeout;
		ppref->schedule_entry[cur_idx].visible = TRUE;
		ppref->schedule_entry[cur_idx].fail_count = 0;
		ppref->schedule_entry[cur_idx].status = 0;
		///memset(ppref->schedule_entry[cur_idx].reserved,0,50);
		if (cur_idx == 0 && ppref->num_entries == 0)
			ppref->num_entries = 1;
		if (ppref->database && !saved)
		{
			SaveDatabase(*ppref,false,cur_idx);
			LoadDatabase(*ppref);
			saved = true;
		}
		pref_mutex.Unlock();
		if (dialog_box)
		{
			MessageBoxA(NULL,LPCSTR("Current entry edited."),PROGRAM_NAME,
				MB_OK|MB_ICONINFORMATION);
		}
		SetUpdated(true);
	}
}

void ScheduleDlg::OnPaint() 
{
	GetDlgItem(IDC_CHECK9)->EnableWindow(ppref->code_is_good);
	GetDlgItem(IDC_CHECK10)->EnableWindow(ppref->code_is_good);
	GetDlgItem(IDC_COMBO1)->EnableWindow(ppref->code_is_good);
	DisableControls();
	CPaintDC dc(this); // device context for painting 
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
	
}


void ScheduleDlg::OnKillfocusCombo1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);	
	entry_changed = true;
}

void ScheduleDlg::MoveToEntry(long idx)
{
	if (ppref->num_entries > 0)
	{
		CommitChanges(FALSE);
		cur_idx = ppref->num_entries;
		do
		{
			cur_idx--;
		} while (cur_idx > 0 && !ppref->schedule_entry[cur_idx].visible);
		if (idx >= 0 && idx <= cur_idx && ppref->schedule_entry[idx].visible)
		{
			CopyScheduleInfo(ppref,padd,idx);
			cur_idx = idx;
		}
		else if (ppref->schedule_entry[cur_idx].visible)
		{
			CopyScheduleInfo(ppref,padd,cur_idx);
		}
	}
	else 
	{
		cur_idx = 0;
	}
	UpdateData(FALSE);
}
void ScheduleDlg::OnBnClickedCheck11()
{
	// TODO: Add your control notification handler code here
	m_monitor_mountpoint = !m_monitor_mountpoint;
	UpdateData(FALSE);
	entry_changed = true;
	DisableControls();
}

void ScheduleDlg::DisableControls()
{
	GetDlgItem(IDC_EDIT1)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_EDIT2)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_EDIT3)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_EDIT4)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_EDIT5)->EnableWindow(!m_monitor_server);
	GetDlgItem(IDC_CHECK1)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_CHECK2)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_CHECK3)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_CHECK4)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_CHECK5)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_CHECK6)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_CHECK7)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_RADIO1)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
	GetDlgItem(IDC_RADIO2)->EnableWindow(!m_monitor_mountpoint&&!m_monitor_server);
}

void ScheduleDlg::OnAdvancedMountpointOptions()
{
	int i;
	// TODO: Add your control notification handler code here
	for (i = 0; i < IGNORE_EXT_MAX; i++)
		enable_mp_ignore[i] = 1;
	MountpointDlg box(ppref,cur_idx,enable_ignore,ignore_day,ignore_start_hr,
		ignore_start_min,ignore_end_hr,ignore_end_min,enable_mp_ignore,ignore_mp,
		enable_mp_ignore2,ignore_mp2,enable_mp_ignore3,ignore_mp3,
		enable_mp_ignore4,ignore_mp4,ignore_ext);

	box.DoModal();

	for (i = 0; i < IGNORE_EXT_MAX; i++)
	{
		strcpy(ignore_ext[i],box.ignore_ext_ptr[i]);
	}
}

void ScheduleDlg::OnBnClickedCheck12()
{
	// TODO: Add your control notification handler code here
	m_monitor_server = !m_monitor_server;
	UpdateData(FALSE);
	entry_changed = true;
	DisableControls();
}

void ScheduleDlg::OnCbnKillfocusCombo3()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void ScheduleDlg::OnBnClickedCheck15()
{
	// TODO: Add your control notification handler code here
	m_delete_this = !m_delete_this;
	UpdateData(FALSE);
	entry_changed = true;
}

void ScheduleDlg::OnEnKillfocusEdit7()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	entry_changed = true;
}

void ScheduleDlg::OnCbnKillfocusCombo4()
{
	UpdateData(TRUE);
}

