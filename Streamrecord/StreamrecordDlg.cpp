//-------------------------------------------
// David Zientara
// 10-27-2022
//
// Streamrecord.Dlg.cpp
//
// File for the StreamrecordDlg classs
//
//---------------------------------------------


// streamrecordDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afxmt.h"
#include "streamrecord.h"
#include "streamrecordDlg.h"
#include "StreamInstance.h"
#include "ScheduleDlg.h"
#include "Parse.h"
#include "loadpref.h"
#include "mpg123.h"
#include ".\\include\\libzplay.h"
#include "ViewScheduleDlg.h"
#include "ActivationDlg.h"
#include "parseserver.h"
#include "RecordMountpointDlg.h"
#include "StopDlg.h"

#include <math.h>
#include <direct.h>
#include <process.h>
#include <time.h>
#include ".\streamrecorddlg.h"
#include "systray.h"
#include "ircbotmfc.h"
#include "IRCSettingsDlg.h"
#include "IRCChatDlg.h"
#include "Database.h"
#include "CDatabaseSettingsDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_STREAMS	65536
#define TIMER_ID	1
#define TIMER_ID_2	2
#define TIMER_ID_3	3
#define TIMER_ID_4	4	
#define TIMER_ID_5	5
#define TIMER_ID_6	6
#define TIMER_ID_7	7
#define TIMER_ID_8	8
#define TIMER_ID_9	9
#define	FC_MAX		50

extern Database* dbase;
extern bool preferences_lock;

StreamInstance *stream_ptr = NULL;
//typedef StreamInstance* StreamPtr;
StreamPtr stream_array[MAX_STREAMS];
HINSTANCE dllhandle;
CWinThread *threadPtr = NULL;

////using namespace libZPlay;

const long MSEC = 1000;
static long stream_count = 0;
static bool busy = false;
static bool dialog_init = false;
static bool path_changed = false;
static bool status_update = false;
static bool paused = false;
static bool playing = false;
static bool display_dlg = true;

////static ZPlay *player;
////static TTimeFormat time_format;
///static TStreamTime stream_time;
////static TStreamInfo stream_info;
static long msec;
static long samples;
CMutex server_mutex, thread_mutex;
static char mp_list[25][128];
static CString tempstr = "IDLE";
wchar_t minimize_icon[32]; // = "icon.ico";

CMutex schedule_mutex;

extern char working_dir[1024];


const char month_ar[12][12] = { "January", "February", "March", "April", "May",
"June", "July", "August", "September", "October", "November", "December" };

struct thread_struct
{
	char stream_URL[1024];
	char output_filename[1024];
	StreamInstance *stream_ptr;
} thread_params;

struct parse_struct
{
	STREAMRECORD_PREFERENCES *pref;
	STREAMRECORD_PREFERENCES* temp;
	IGNORE_LIST *ignore;
	SCHEDULE_ADD_LIST* add;
	char *url;
	short retval;
} parse_params;

struct dialog_struct
{
	CStreamrecordDlg* dlg_ptr;
} dialog_params;

UINT LoadPref(LPVOID pParam)
{
	parse_struct* pp = (parse_struct *)&pParam;
	LoadDatabase(*pp->pref);
	///LoadPreferences(PREF_FILE, *pp->pref, *pp->ignore, *pp->add);
	return 1;
}

UINT RecordStream(LPVOID pParam)
{
	thread_struct *param_ptr = (thread_struct *)pParam;

	param_ptr->stream_ptr->RecordStream(); //param_ptr->stream_URL,param_ptr->output_filename);
	return 1;
}

UINT CheckSchedule(LPVOID pParam)
{
	dialog_params.dlg_ptr->CheckForScheduledEvents();
	return 1;
}
UINT CheckServerStatus(LPVOID pParam)
{
	dialog_params.dlg_ptr->CheckServer();
	return 1;
}

UINT ParseServerData(LPVOID pParam)
{
	parse_struct *param_ptr = (parse_struct *)pParam;

	thread_mutex.Lock();
	param_ptr->retval = -1;
	//param_ptr->retval = ParseServerStatus(param_ptr->pref,param_ptr->ignore,
	//	param_ptr->url,mp_list);
	thread_mutex.Unlock();
	return 1;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()
char curdir[1024];


/////////////////////////////////////////////////////////////////////////////
// CStreamrecordDlg dialog

//-------------------------------------------------------------
// CStreamrecordDlg
// Constructor for the CStreamrecordDlg class
// PARAMS: CWnd (parent window - should be NULL)
// RETURNS: Nothing; class variables are initialized
//
//-------------------------------------------------------------
CStreamrecordDlg::CStreamrecordDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStreamrecordDlg::IDD, pParent)
	, m_sound_file(_T(""))
	, m_play_sound_file(FALSE)
	, m_psound_file(_T(""))
	, m_nosubdirs(FALSE)
	, m_enable_sounds(FALSE)
	, m_database(FALSE)
	, m_password(_T(""))
	, m_enable_dbox(FALSE)
{
	
	_getcwd(curdir, 1023);
	ConvertString(minimize_icon, "icon.ico");

	memset(stream_array,0,sizeof(StreamPtr)*MAX_STREAMS);
	LoadPreferences(PREF_FILE, pref, ignore, add);
	CDatabaseSettingsDlg box(&pref, &ignore, &add);
	if (pref.database && pref.enable_dbox)
		box.DoModal();
	//{{AFX_DATA_INIT(CStreamrecordDlg)
	m_enable_dbox = pref.enable_dbox;
	m_stream_URL = pref.stream_URL; //_T("");
	m_output_file = pref.output_filename;  //_T("");
	m_status = _T("IDLE\r\n");
	m_path = pref.path;
	m_shoutcast = pref.shoutcast;
	m_sound_file = pref.play_file;
	m_time_elapsed = _T("00:00");
	m_play_sound_file = pref.play_sound_file;
	m_psound_file = pref.sound_file;
	m_nosubdirs = pref.no_subdirs;
	m_enable_sounds = pref.enable_sounds;
	m_database = pref.database;
	m_password = pref.DBpassword;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
	shutdown = FALSE;
	last_recording = " ";	
	stop_box = NULL;
	/*
	minimize_icon[0] = 'i';
	minimize_icon[1] = 'c';
	minimize_icon[2] = 'o';
	minimize_icon[3] = 'n';
	minimize_icon[4] = '.';
	minimize_icon[5] = 'i';
	minimize_icon[6] = 'c';
	minimize_icon[7] = 'o';
	minimize_icon[8] = NULL;
	minimize_icon[9] = NULL;
	*/
	////player = CreateZPlay();

	irc = new IrcBotMFC(pref.irc.server,pref.irc.channel,pref.irc.nickname,pref.irc.UID,
		pref.irc.color,pref.irc.background);
	if (pref.irc.enable_posting && irc != NULL)
		irc->Start();
	if (pref.database)
		ResetStatus(pref);
	srand(time(NULL));

	
	
}

//-------------------------------------------------------------
// Cleanup
// Method that performs "cleanup" operations for CStreamrecord
// variables
// PARAMS: None
// RETURNS: Nothing; several threads are closed out 
//
//-------------------------------------------------------------


void CStreamrecordDlg::Cleanup()
{
	long i, j, k;

	k = pref.num_entries;

	for (i = 0; i < k; i++)
	{
		if (pref.schedule_entry[i].stream_idx >= 0
			&& pref.schedule_entry[i].stream_idx < pref.num_entries
			&& stream_array[pref.schedule_entry[i].stream_idx] != NULL)
		{
			stream_array[pref.schedule_entry[i].stream_idx]->SetTerminate(TRUE);
			if (pref.schedule_entry[i].thread_ptr != NULL)
				WaitForSingleObject(pref.schedule_entry[i].thread_ptr,INFINITE);
			pref.schedule_entry[i].thread_ptr = NULL;
		}
		if (pref.schedule_entry[i].visible == 0)
		{
			for (j = i+1; j < pref.num_entries; j++)
				pref.schedule_entry[j-1] = pref.schedule_entry[j];
			pref.num_entries--;
		}
	}

	////player->Release();
	
}
//------------------------------------------------------
// DoDataExchange
// Data exchange takes place between variables and 
// dialog controls
//
// PARAMS: CDataExchange pointer (pDX)
// RETURNS: Nothing; if UpdateData is passed an argument of
// TRUE, data is passed from the dialog controls to the 
// variables; if FALSE, data is passed from the variables to
// the dialog controls
//
//-------------------------------------------------------

void CStreamrecordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStreamrecordDlg)
	if (!status_update)
	{
		DDX_Text(pDX, IDC_EDIT1, m_stream_URL);
		DDX_Text(pDX, IDC_EDIT2, m_output_file);
		DDX_Text(pDX, IDC_EDIT4, m_path);
	}
	DDX_Text(pDX, IDC_EDIT3, m_status);
	DDX_Check(pDX, IDC_CHECK1, m_shoutcast);
	DDX_Control(pDX, IDC_SLIDER1, mplay_ctrl);
	DDX_Text(pDX, IDC_EDIT6, m_time_elapsed);
	//}}AFX_DATA_MAP
	dialog_init = true;
	DDX_Text(pDX, IDC_EDIT5, m_sound_file);
	DDV_MaxChars(pDX, m_sound_file, 1024);


	DDX_Control(pDX, IDC_BUTTON5, m_play_button);
	DDX_Control(pDX, IDC_BUTTON6, m_stop_button);
	DDX_Control(pDX, IDC_BUTTON7, m_pause_button);

	DDX_Check(pDX, IDC_CHECK2, m_play_sound_file);
	DDX_Text(pDX, IDC_EDIT7, m_psound_file);
	DDX_Check(pDX, IDC_CHECK3, m_nosubdirs);
	DDX_Check(pDX, IDC_CHECK4, m_enable_sounds);
	DDX_Check(pDX, IDC_CHECK5, m_database);
	DDX_Text(pDX, IDC_EDIT8, m_password);
	DDX_Check(pDX, IDC_CHECK6, m_enable_dbox);
}

BEGIN_MESSAGE_MAP(CStreamrecordDlg, CDialog)
	//{{AFX_MSG_MAP(CStreamrecordDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnConnect)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_EN_KILLFOCUS(IDC_EDIT2, OnKillfocusEdit2)
	ON_BN_CLICKED(IDC_BUTTON2, OnStop)
	ON_BN_CLICKED(IDC_BUTTON3, OnSchedule)
	ON_EN_KILLFOCUS(IDC_EDIT4, OnKillfocusEdit4)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_BUTTON4, OnAboutProgram)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON5, OnPlayFile)
	ON_BN_CLICKED(IDC_BUTTON6, OnStopFile)
	ON_EN_KILLFOCUS(IDC_EDIT5, OnEnKillfocusEdit5)
	ON_BN_CLICKED(IDC_BUTTON7, OnPauseFile)
	ON_BN_CLICKED(IDC_BUTTON8, OnBrowseSoundFile)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnNMReleasedcaptureSlider1)
	ON_BN_CLICKED(IDC_BUTTON9, OnEnterActivationCode)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
	ON_EN_KILLFOCUS(IDC_EDIT7, OnEnKillfocusEdit7)
	ON_BN_CLICKED(IDC_BUTTON10, OnBnClickedButton10)
	ON_BN_CLICKED(IDC_CHECK3, OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_BUTTON11, OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, OnBnClickedDatabaseSettings)
	ON_BN_CLICKED(IDC_CHECK5, OnBnClickedDatabase)
	ON_BN_CLICKED(IDC_BUTTON16, &CStreamrecordDlg::OnBnClickedSync)
	ON_BN_CLICKED(IDC_BUTTON17, &CStreamrecordDlg::OnBnClickedResetDatabase)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStreamrecordDlg message handlers
//------------------------------------------------------
// OnInitDialog
// Function executes a series of commands to be done when
// dialog box is initialized
//
// PARAMS: CDataExchange pointer (pDX)
// RETURNS: Nothing; if UpdateData is passed an argument of
// TRUE, data is passed from the dialog controls to the 
// variables; if FALSE, data is passed from the variables to
// the dialog controls
//
//-------------------------------------------------------

BOOL CStreamrecordDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	SetTimer(TIMER_ID,5000,0);
	SetTimer(TIMER_ID_2,10000,0);
	SetTimer(TIMER_ID_3,1000,0);
	SetTimer(TIMER_ID_4,30000,0);
	SetTimer(TIMER_ID_5,pref.DBinterval*1000, 0);
	SetTimer(TIMER_ID_6, 3600000, 0);
	SetTimer(TIMER_ID_7, 600000, 0);
	SetTimer(TIMER_ID_8, 60000, 0);
	SetTimer(TIMER_ID_9,180000, 0);
	

	m_play_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP5)));
	m_stop_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP6)));
	m_pause_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP7)));

	UpdateData(FALSE);

	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}
//------------------------------------------------------
// OnBnClickedDatabase
//
// Function is called when the user clicks on the
// Sync with Database checkmark
// PARMS: None
// RETURNS: Nothing; if enabled, preferences are saved
// and the database is loaded into memory
//-------------------------------------------------------
void CStreamrecordDlg::OnBnClickedDatabase()
{
	m_database = !m_database;
	pref.database = m_database;
	if (m_database)
	{
		SavePreferences(PREF_FILE, pref, ignore,add,false);
		LoadDatabase(pref);
		CopySchedule(pref);
	}
	UpdateData(FALSE);
}

void CStreamrecordDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
//------------------------------------------------------
// OnPaint
// Function is called when it's time to paint the 
// screen
// PARAMS: None
// RETURNS: Nothing; screen is painted
//------------------------------------------------------
void CStreamrecordDlg::OnPaint() 
{
	// Enable activation code button if activation code not
	// entered or if activation code is invalid
	GetDlgItem(IDC_BUTTON9)->EnableWindow(!pref.code_is_good);
	// Enable media player buttons and slider if activation code is good
	GetDlgItem(IDC_BUTTON5)->EnableWindow(pref.code_is_good);
	GetDlgItem(IDC_BUTTON6)->EnableWindow(pref.code_is_good);
	GetDlgItem(IDC_BUTTON7)->EnableWindow(pref.code_is_good);
	GetDlgItem(IDC_BUTTON8)->EnableWindow(pref.code_is_good);
	GetDlgItem(IDC_SLIDER1)->EnableWindow(pref.code_is_good);
	if (!display_dlg)
	{
		_chdir(working_dir);
		InitNotifyIconData(GetSafeHwnd(),minimize_icon,(wchar_t *)"IDLE");
		Minimize(GetSafeHwnd());
	}
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}

	
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStreamrecordDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
//------------------------------------------------------
// OnConnect
// Function is called when the Connect button is pressed
// Spawns a stream that records the stream URL to the
// output file
// PARAMS: None
// Returns: Nothing; stream is recorded
//-------------------------------------------------------
void CStreamrecordDlg::OnConnect() 
{
	// TODO: Add your control notification handler code here
    bool sc = false;
	char *file_ext, ext[32];
	char *temp;
	char output_file[1024];
	char stream_URL[1024];

	if (stream_ptr == NULL)
	{
		_chdir(working_dir);
		stream_ptr = new StreamInstance;
		if (stream_ptr != NULL)
		{
			//CopyString(temp, m_output_file);
			CopyString(output_file, m_output_file);
			strcpy(thread_params.output_filename, output_file); //LPCTSTR(m_output_file));
			CopyString(stream_URL, m_stream_URL);
			strcpy(thread_params.stream_URL, stream_URL); //LPCTSTR(m_stream_URL));
			thread_params.stream_ptr = stream_ptr;
			if (m_shoutcast)
				sc = true;
			strcpy(ext,"mp3");
			///CopyString(output_file, m_output_file);
			file_ext = strrchr(output_file, '.'); //LPCTSTR(m_output_file), '.');
			if (file_ext != NULL && !m_shoutcast
				&& strchr(file_ext,'/') == NULL
				&& strchr(file_ext,':') == NULL)
			{
				temp = file_ext;
				do
				{
					temp--;
					if (*temp == '.')
						file_ext = NULL;
				} while (temp != output_file  //LPCTSTR(m_output_file) 
					&& *temp != '/');
				if (file_ext != NULL)
					file_ext++;
				if (file_ext != NULL)
					strncpy(ext,file_ext,31);
			}
			stream_ptr->CopyParams(stream_URL, //LPCTSTR(m_stream_URL),
				//LPCTSTR(m_output_file),
				output_file, sc,false,false,0,false,ext,false,-1,false,
				NULL,NULL,pref.enable_sounds);		
			threadPtr = AfxBeginThread(RecordStream,(LPVOID)&thread_params,THREAD_PRIORITY_NORMAL);		
		}
	}
}
//------------------------------------------------------
// OnKillfocusEdit1
// Function is called when Stream URL edit box loses
// focus
// PARAMS: None
// RETURNS: Nothing
//------------------------------------------------------
void CStreamrecordDlg::OnKillfocusEdit1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
//------------------------------------------------------
// OnKillfocusEdit2
// Function is called when output file edit box loses
// focus
// PARAMS: None
// RETURNS: Nothing
//------------------------------------------------------
void CStreamrecordDlg::OnKillfocusEdit2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}
//------------------------------------------------------
// CheckMessagePump
// Function is called when Stream URL edit box loses
// focus
// PARAMS: None
// RETURNS: Nothing
//------------------------------------------------------
void CStreamrecordDlg::CheckMessagePump()
{
	MSG msg;

	while (::PeekMessage(&msg,NULL,0,0,PM_REMOVE) != NULL)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
//------------------------------------------------------
// OnStop
// Function is called when stop button is pressed
// All it does it set the terminate flag
// 
// PARAMS: None
// RETURN: Nothing; terminate flag set to TRUE
//-------------------------------------------------------
void CStreamrecordDlg::OnStop() 
{
	// TODO: Add your control notification handler code here
	if (stream_ptr != NULL)
	{
		stream_ptr->SetTerminate(TRUE);
	}
}

//--------------------------------------------------------------
// WindowProc
// Callback function is used to process messages
// PARAMS: message (UINT), wParam (WPARAM), lParam (LPARAM)
// RETURNS: LRESULT (function calls the parent class's function)
// This function is used to minimize and restore the program
// 
//---------------------------------------------------------------
LRESULT CStreamrecordDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class	

	if (busy)
		return CDialog::WindowProc(message, wParam, lParam);

	switch (message)
	{
		case WM_TRAYICON:
			switch (wParam)
			{
				case ID_TRAY_APP_ICON:
					if (lParam == WM_LBUTTONUP)
					{
						Restore(GetSafeHwnd());
						display_dlg = true;
					}
					break;
			}
			break;
		case WM_SIZE:
			switch (wParam)
			{
				case SIZE_MINIMIZED:
					char tempdir[1024];
					_getcwd(tempdir, 1023);
					_chdir(curdir);
					InitNotifyIconData(GetSafeHwnd(),minimize_icon,tempstr);
					Minimize(GetSafeHwnd());
					display_dlg = false;
					break;
			}
			break;
		default: break;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
//--------------------------------------------------------------
// OnOK
// Function is called if the OK button is clicked
// PARAMS: None
// RETURNS: Nothing; variables are copies into preferences
// structure and saved
//
//---------------------------------------------------------
void CStreamrecordDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//strncpy(pref.stream_URL,LPCTSTR(m_stream_URL),255);
	//strncpy(pref.output_filename,LPCTSTR(m_output_file),255);
	//strncpy(pref.path,LPCTSTR(m_path),1023);
	//strncpy(pref.play_file,LPCTSTR(m_sound_file),255);

	CopyString(pref.stream_URL, m_stream_URL, 255);
	CopyString(pref.output_filename, m_output_file, 255);
	CopyString(pref.path, m_path);
	CopyString(pref.play_file, m_sound_file, 255);

	CopyString(pref.DBpassword, m_password);

	pref.shoutcast = m_shoutcast;
	pref.play_sound_file = m_play_sound_file;
	CopyString(pref.sound_file, m_psound_file, 255);
	//strncpy(pref.sound_file,LPCTSTR(m_psound_file),255);
	pref.no_subdirs = m_nosubdirs;
	pref.enable_sounds = m_enable_sounds;
	pref.database = m_database;
	pref.enable_dbox = m_enable_dbox;
	KillTimer(TIMER_ID);
	KillTimer(TIMER_ID_2);
	KillTimer(TIMER_ID_3);
	KillTimer(TIMER_ID_4);
	Cleanup();
	SavePreferences(PREF_FILE,pref,ignore,add);


	delete [] pref.schedule_entry;
	delete [] ignore.ignore_entry;

	if (stop_box != NULL)
	{
		stop_box->DestroyWindow();
		delete stop_box;
	}
	if (irc != NULL)
	{
		if (pref.irc.enable_posting)
			irc->Stop();
		delete irc;
	}
	if (dbase != NULL)
		delete dbase;
	CDialog::OnOK();
}
//----------------------------------------------------------
//OnSchedule
//Function is called when the Schedule button is pressed
//PARAMS: None
//Returns: Nothing; Schedule dialog box is spawned
//
//----------------------------------------------------------
void CStreamrecordDlg::OnSchedule() 
{
	// TODO: Add your control notification handler code here
	/*
	ScheduleDlg box(&pref);

	if (box.DoModal() == IDOK)
	{
		SavePreferences(PREF_FILE,pref);
	}
	*/
	//LoadPreferences(PREF_FILE, pref, ignore, add);
	pref.no_load = true;
	ViewScheduleDlg box(&pref,&ignore,&add);
	
	box.DoModal();
	pref.no_load = false;
}
//-----------------------------------------------------------
//CheckServer
//Function is called every 10 seconds to monitor an Icecast
//server
//PARAMS: None
//RETURNS Nothing; ParseServerStatus function is spawned 
//as a thread
//-----------------------------------------------------------
void CStreamrecordDlg::CheckServer()
{
	long i, j, mp_num;
	time_t osBinaryTime;
	CTime *cur_time;
	unsigned int hour, min, day, sec, month, date, year;
	char temp[1024], logfile[1024], psound_file[1024];
	RecordMountpointDlg box;
	static bool locked = false;
	FILE *fp;
	//HANDLE thread_handle;
	StreamInstance *streamptr = NULL;
	unsigned int day_mask;

	time(&osBinaryTime);

	server_mutex.Lock();
	schedule_mutex.Lock();
	//while (preferences_lock)
	//	Sleep(100);

	if (locked)
		return;
	locked = true;

	strcpy(logfile,working_dir);
	if (logfile[strlen(logfile)-1] != '\\')
		strcat(logfile,"\\");
	strcat(logfile,LOG_FILE);

	cur_time = new CTime(osBinaryTime);

	hour = cur_time->GetHour();
	min = cur_time->GetMinute();
	sec = cur_time->GetSecond();
	day = cur_time->GetDayOfWeek();
	month = cur_time->GetMonth();
	date = cur_time->GetDay();
	year = cur_time->GetYear();

	//if (sec >= 30 && sec <= 35)
	//if (sec < 60)
	if ((sec >= 0 && sec < 5) || (sec >= 15 && sec < 20) 
		|| (sec >= 30 && sec < 35) || (sec >= 45 && sec < 50))
	{
		schedule_mutex.Lock();
		if (pref.irc.enable_posting && irc != NULL)
		{
			irc->Send("PING\r\n");
			if (pref.irc.ringo_mode && (rand()%10) == 0)
				irc->Ringo();
			
		}
		day_mask = (unsigned int)pow((double)2.0,(double)(day-1));
		streamptr = new StreamInstance(&pref);
		for (i = 0; i < pref.num_entries; i++)
		{
			if (pref.schedule_entry[i].monitor_server
				&& (pref.schedule_entry[i].enable_ignore[0] == 0
				|| (hour < (unsigned)pref.schedule_entry[i].ignore_start_hr[0]
				|| (hour == pref.schedule_entry[i].ignore_start_hr[0]
				&&	min < (unsigned)pref.schedule_entry[i].ignore_start_min[0]))
				|| (hour > (unsigned)pref.schedule_entry[i].ignore_end_hr[0]
				|| (hour == pref.schedule_entry[i].ignore_end_hr[0]
				&& min >= (unsigned)pref.schedule_entry[i].ignore_end_min[0]))
				|| (day_mask&pref.schedule_entry[i].ignore_day[0]) == 0)
				&& (pref.schedule_entry[i].enable_ignore[1] == 0
				|| (hour < (unsigned)pref.schedule_entry[i].ignore_start_hr[1]
				|| (hour == pref.schedule_entry[i].ignore_start_hr[1]
				&&	min < (unsigned)pref.schedule_entry[i].ignore_start_min[1]))
				|| (hour > (unsigned)pref.schedule_entry[i].ignore_end_hr[1]
				|| (hour == pref.schedule_entry[i].ignore_end_hr[1]
				&& min >= (unsigned)pref.schedule_entry[i].ignore_end_min[1]))
				|| (day_mask&pref.schedule_entry[i].ignore_day[1]) == 0)
				&& (pref.schedule_entry[i].enable_ignore[2] == 0
				|| (hour < (unsigned)pref.schedule_entry[i].ignore_start_hr[2]
				|| (hour == pref.schedule_entry[i].ignore_start_hr[2]
				&&	min < (unsigned)pref.schedule_entry[i].ignore_start_min[2]))
				|| (hour > (unsigned)pref.schedule_entry[i].ignore_end_hr[2]
				|| (hour == pref.schedule_entry[i].ignore_end_hr[2]
				&& min >= (unsigned)pref.schedule_entry[i].ignore_end_min[2]))
				|| (day_mask&pref.schedule_entry[i].ignore_day[2]) == 0))
			{
				CopyString(psound_file, m_psound_file);
				streamptr->CopyParams(pref.schedule_entry[i].stream_URL,
					"output.mp3",false,false,false,96,false,"mp3",false,i,
					m_play_sound_file,psound_file,NULL,pref.enable_sounds,irc,
					pref.irc.update_topic);

				mp_num = streamptr->ParseServerStatus(&pref,&ignore,
					&add,pref.schedule_entry[i].stream_URL,mp_list,i);
				Sleep(100);

				/*
				parse_params.pref = &pref;
				parse_params.ignore = &ignore;
				parse_params.url = pref.schedule_entry[i].stream_URL;
				parse_params.retval = -2;

				thread_handle = AfxBeginThread(ParseServerData,
					(LPVOID)&parse_params,THREAD_PRIORITY_HIGHEST);

				j = 0;
				while (parse_params.retval == -2 && j++ < 3)
					Sleep(50);

				if (j < 3)
					WaitForSingleObject(thread_mutex,INFINITE);
					
				mp_num = parse_params.retval;
				*/

				/*
				mp_num = ParseServerStatus(&pref,&ignore,
					pref.schedule_entry[i].stream_URL,
 					mp_list);
				*/

				if (mp_num > 0)
				{
					fp = fopen(logfile,"a");
					if (fp != NULL)
					{
						fprintf(fp,"%d new mountpoint[s] found at %d:%d on %d-%d-%d\n",
							mp_num,hour,min,month,date,year);
						fclose(fp);
					}
					if (pref.schedule_entry[i].monitor_level == 2)
						Beep(523,500);
					for (j = 0; j < mp_num; j++)
					{
						sprintf(temp,
							"A new mointpoint has been found at:\n%s\nRecord?",
							mp_list[j]);
						fp = fopen(logfile,"a");
						if (fp != NULL)
						{
							fprintf(fp,"Found at %s\n",mp_list[j]);
							fclose(fp);
						}
						if (pref.schedule_entry[i].monitor_level == 5)
						{
							AddToSchedule(&pref,&add,mp_list[j],TRUE,
								FALSE,0,0,0,0,pref.schedule_entry[i].timeout);
							SaveDatabase(pref, false, pref.num_entries - 1);
							//ignore.ignore_entry[ignore.num_entries].entry_num = pref.schedule_entry[i].id;
							//strcpy(ignore.ignore_entry[ignore.num_entries++].mountpoint_URL,mp_list[j]);
						}
						else if (pref.schedule_entry[i].monitor_level > 0
							&& MessageBoxA(NULL,temp,PROGRAM_NAME,MB_YESNO|MB_ICONINFORMATION)
							== IDYES)
						{
							// Do something
							box.DoModal();
							AddToSchedule(&pref,&add,mp_list[j],box.ContinuousMonitor()|box.MonitorUntilDisc(),
								box.MonitorUntilDisc(),box.GetStartHour(),
								box.GetStartMin(),box.GetEndHour(),box.GetEndMin(),
								pref.schedule_entry[i].timeout);
							SaveDatabase(pref,false,pref.num_entries-1);
							//ignore.ignore_entry[ignore.num_entries].entry_num = pref.schedule_entry[i].id;
							//strcpy(ignore.ignore_entry[ignore.num_entries++].mountpoint_URL,mp_list[j]);
						}
						else
						{
							// Do something else
							ignore.ignore_entry[ignore.num_entries].entry_num = pref.schedule_entry[i].id;
							strcpy(ignore.ignore_entry[ignore.num_entries++].mountpoint_URL,mp_list[j]);
						}
					}
				}
			}
		}
		delete streamptr;
		//preferences_lock = false;
		schedule_mutex.Unlock();
	}

	delete cur_time;
	locked = false;

	
	//thread_mutex.Unlock();
	//server_mutex.Unlock();
}
//---------------------------------------------------
//CheckforScheduledEvents
//Function is called every 5 seconds to see
//if an item in the schedule needs to be recorded
//PARAMS: None
//RETURNS: Nothing; all items in the schedule array
//are checked; if an item needs to be recorded, a
//RecordStream function is spawned
//
//----------------------------------------------------
void CStreamrecordDlg::CheckForScheduledEvents()
{
	long i, j;
	time_t osBinaryTime;
	CTime *cur_time;
	unsigned int hour, min, day, sec, day_mask, daytwo_mask;
	unsigned int month, date, year;
	char output_filename[1024];	
	char tmp[1024], tmp_dir[1024], ext[32];
	char tempstr[1024], psound_file[1024], output_file[1024], stream_URL[1024]; 
	char lr[1024], ptemp[1024], ctemp[1024], dtemp[1024];
	CString cstr;
	wchar_t tempwchar[1024];
	bool scast = false;
	bool enc = false;
	bool del_old = false;
	bool status_info = false;
	long num_entries = pref.num_entries;
	long *stream_index = new long[pref.num_entries];
	char *file_ext = NULL;
	char *temp;
	bool mp = false;
	bool infinite_retry = true;
	bool changed = false;
	bool recording = false;
	FILE *gp;
	static BOOL dir_dlg_shown = FALSE;
	static BOOL is_idle = FALSE;
	bool straddle = false;
//	IrcBot *bot = new IrcBot("NICK testBOT\r\n","USER guest tolmoon tolsun :GI Joe poopies\r\n");
    
//	bot->start();

//	delete bot;

	schedule_mutex.Lock();
	///while (preferences_lock)
	//	Sleep(100);
	/////preferences_lock = true;
	//if (pref.database)
	//	CopySchedule(pref);

	time(&osBinaryTime);
	cur_time = new CTime(osBinaryTime);

	memset(stream_index,-1,sizeof(long)*pref.num_entries);
	// Get the current time;
	hour = cur_time->GetHour();
	min = cur_time->GetMinute();
	sec = cur_time->GetSecond();
	day = cur_time->GetDayOfWeek();
	month = cur_time->GetMonth();
	date = cur_time->GetDay();
	year = cur_time->GetYear();

	sprintf_s(pref.datetime, "%d-%d-%d %02d:%02d:%02d", month, date, year, hour, min, sec);

	UpdateData(TRUE);
	///strncpy(pref.path,LPCTSTR(m_path),1023);
	CopyString(pref.path, m_path);

	m_status = "";

	CopyString(tempstr, last_recording);
	if (strcmp(tempstr, " ") != 0)
		m_status += last_recording;

	day_mask = (unsigned int)pow((double)2.0,(double)(day-1));

	if ((day - 2) >= 0)
		daytwo_mask = (unsigned int)pow((double)2.0, (double)(day - 2));
	else
		daytwo_mask = 64;

	
	//Iterate through all schedule items;
	//check to see if anything needs to be recorded


	for (i = 0; i < pref.num_entries; i++)
	{
		//straddle variable is for a program that "straddles" between one day
		//and another day; e.g. 11 PM to 1AM, or 10 PM to 2AM, etc.
		straddle = false;
		if (hour == 0 && min == 0 && sec < 10)
			pref.schedule_entry[i].recorded = FALSE;

		//straddle is true if end hour is less than the start hour:
		if (pref.schedule_entry[i].end_hr < pref.schedule_entry[i].start_hr)
			straddle = true;

		

		if (!pref.schedule_entry[i].recorded
			&& !pref.schedule_entry[i].monitor_server
			&& ((pref.schedule_entry[i].stream_idx < 0
			&& (((hour == pref.schedule_entry[i].start_hr
			&& (min >= (pref.schedule_entry[i].start_min-1)
			|| min >= pref.schedule_entry[i].start_min)
			&& (sec >= 50 || min >= pref.schedule_entry[i].start_min))
			|| (hour >= (pref.schedule_entry[i].start_hr-1)
			&& (60-min+pref.schedule_entry[i].start_min) <= 1
			&& sec >= 50) || (hour > pref.schedule_entry[i].start_hr)
				|| ((daytwo_mask & pref.schedule_entry[i].days) != 0 && straddle)
			) && (((hour < pref.schedule_entry[i].end_hr)
			|| (hour == pref.schedule_entry[i].end_hr
			&& min < pref.schedule_entry[i].end_min)
			|| (pref.schedule_entry[i].end_hr < pref.schedule_entry[i].start_hr))
			|| (hour == pref.schedule_entry[i].start_hr
			&& min == pref.schedule_entry[i].start_min)))
			|| ( (((sec <= 5 || (sec >= 27 && sec < 32)) && pref.schedule_entry[i].monitor_mountpoint == 1
			&& pref.schedule_entry[i].monitor_server == 0)
			|| (pref.schedule_entry[i].monitor_mountpoint == 1 
			&& pref.schedule_entry[i].monitor_server == 0
			&& pref.schedule_entry[i].record_now))
			&& (pref.schedule_entry[i].enable_ignore[0] == 0
			|| (hour < (unsigned)pref.schedule_entry[i].ignore_start_hr[0]
			|| (hour == pref.schedule_entry[i].ignore_start_hr[0]
			&&	min < (unsigned)pref.schedule_entry[i].ignore_start_min[0]))
			|| (hour > (unsigned)pref.schedule_entry[i].ignore_end_hr[0]
			|| (hour == pref.schedule_entry[i].ignore_end_hr[0]
			&& min > (unsigned)pref.schedule_entry[i].ignore_end_min[0]))
			|| (day_mask&pref.schedule_entry[i].ignore_day[0]) == 0)
			&& (pref.schedule_entry[i].enable_ignore[1] == 0
			|| (hour < (unsigned)pref.schedule_entry[i].ignore_start_hr[1]
			|| (hour == pref.schedule_entry[i].ignore_start_hr[1]
			&&	min < (unsigned)pref.schedule_entry[i].ignore_start_min[1]))
			|| (hour > (unsigned)pref.schedule_entry[i].ignore_end_hr[1]
			|| (hour == pref.schedule_entry[i].ignore_end_hr[1]
			&& min > (unsigned)pref.schedule_entry[i].ignore_end_min[1]))
			|| (day_mask&pref.schedule_entry[i].ignore_day[1]) == 0)
			&& (pref.schedule_entry[i].enable_ignore[2] == 0
			|| (hour < (unsigned)pref.schedule_entry[i].ignore_start_hr[2]
			|| (hour == pref.schedule_entry[i].ignore_start_hr[2]
			&&	min < (unsigned)pref.schedule_entry[i].ignore_start_min[2]))
			|| (hour > (unsigned)pref.schedule_entry[i].ignore_end_hr[2]
			|| (hour == pref.schedule_entry[i].ignore_end_hr[2]
			&& min > (unsigned)pref.schedule_entry[i].ignore_end_min[2]))
			|| (day_mask&pref.schedule_entry[i].ignore_day[2]) == 0))			)			
			&& !pref.schedule_entry[i].stream_running))
		{
			
			//If the passwords don't match, skip this entry:
			if (strcmp(pref.schedule_entry[i].password, pref.DBpassword) != 0)
				continue;
			//Record this entry IF the passwords match AND [IF the days match
			//OR (IF it's the second day and the days match AND straddle 
			//AND (hour is less than the end hour OR hour equals the end hour and 
			//min is less than the end minute))
			if (strcmp(pref.schedule_entry[i].password, pref.DBpassword) == 0
				&& (day_mask&pref.schedule_entry[i].days) != 0 
				|| ((daytwo_mask & pref.schedule_entry[i].days) != 0 && straddle
					&& ((hour < pref.schedule_entry[i].end_hr) 
						|| (hour == pref.schedule_entry[i].end_hr && min < pref.schedule_entry[i].end_min) ))
				|| pref.schedule_entry[i].monitor_mountpoint == 1)
			{
				
 				if (pref.path[strlen(pref.path)-1] != '\\')
				{
					strcat(pref.path,"\\");
				}
				if (_chdir(pref.path) != 0)
				{
					if (_mkdir(pref.path) != 0 && path_changed)
					{
						path_changed = false;
						MessageBoxA(NULL,LPCSTR("Invalid path"),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
						return;
					}
				}
				pref.schedule_entry[i].rec_date = day_mask;
				// It's time!
				if (stream_count < MAX_STREAMS)
				{
					//chdir to the correct directory; if it does
					//not exist, create it:
					j = 0;
					while (stream_array[j] != NULL)
						j++;
					if (j >= stream_count)
						stream_count++;
					stream_array[j] = new StreamInstance(&pref);
					pref.schedule_entry[i].stream_idx = j;
					strcpy(tmp,pref.path);
					strcpy(tmp_dir,pref.schedule_entry[i].program);
					if (!pref.no_subdirs)
					{
						ParseFilename(tmp_dir);
						strcat(tmp,tmp_dir);
					}
					if (!ParseDirectory(tmp))
					{
						gp = fopen(LOG_FILE,"a");
						if (gp != NULL)
						{
							fprintf(gp,"Directory does not exist and/or cannot be created.");
							fclose(gp);
						}
						if (!dir_dlg_shown)
						{
							dir_dlg_shown = TRUE;
							MessageBoxA(NULL,LPCSTR("Cannot save output file. Invalid path."),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);													
						}						
						delete stream_array[j];
						stream_array[j] = NULL;
						continue;
					}
					if (_chdir(tmp) != 0)
					{
						if (_mkdir(tmp) != 0)
						{
							gp = fopen(LOG_FILE,"a");
							if (gp != NULL)
							{
								fprintf(gp,"Directory does not exist and/or cannot be created.");
								fclose(gp);							
							}
							delete stream_array[j];
							stream_array[j] = NULL;
							continue;
						}
					}
					if (!pref.schedule_entry[i].shoutcast)
						file_ext = strrchr(pref.schedule_entry[i].stream_URL,'.');
					strcpy(ext,"mp3");
					if (file_ext != NULL 
						&& !pref.schedule_entry[i].shoutcast
						&& strchr(file_ext,'/') == NULL
						&& strchr(file_ext,':') == NULL)
					{
						temp = file_ext;
						do
						{
							temp--;
							if (*temp == '.')
								file_ext = NULL;
						} while (temp != pref.schedule_entry[i].stream_URL 
							&& *temp != '/');
						if (file_ext != NULL)
							file_ext++;
						if (file_ext != NULL && strlen(file_ext) > 4)
							strcpy(ext,"mp3");
						else if (file_ext != NULL)
							strncpy(ext,file_ext,31);						
					}
					sprintf(output_filename,"%s\\%s %s %d.%s",tmp,tmp_dir,
						month_ar[cur_time->GetMonth()-1],cur_time->GetDay(),ext);						
					
					thread_params.stream_ptr = stream_array[j];
					///strcpy(thread_params.output_filename,LPCTSTR(output_filename));
					///strcpy(thread_params.stream_URL,LPCTSTR(pref.schedule_entry[i].
					////	stream_URL));

					CopyString(thread_params.output_filename, output_filename);
					CopyString(thread_params.stream_URL, pref.schedule_entry[i].stream_URL);

					if (pref.schedule_entry[i].shoutcast)
						scast = true;
					else 
						scast = false;
					if (pref.schedule_entry[i].reencode)
						enc = true;
					else
						enc = false;
					if (pref.schedule_entry[i].delete_old)
						del_old = true;
					else
						del_old = false;
					if (pref.schedule_entry[i].monitor_mountpoint)
						mp = true;
					else 
						mp = false;
					
					if (pref.schedule_entry[i].monitor_mountpoint
						&& pref.schedule_entry[i].record_now
						&& pref.schedule_entry[i].limited_retry)
						infinite_retry = false;
					else
						infinite_retry = true;
					//Copy the parameters:
					CopyString(psound_file, m_psound_file);
					stream_array[j]->CopyParams(pref.schedule_entry[i].
						stream_URL,output_filename,scast,enc,del_old,
						pref.schedule_entry[i].encodebr,infinite_retry,ext,
						mp,i,m_play_sound_file,psound_file,NULL,
						pref.enable_sounds,irc,pref.irc.update_topic);
					//Now, spawn the RecordStream thread:
					do
					{
						pref.schedule_entry[i].thread_ptr 
							= AfxBeginThread(RecordStream,(LPVOID)&thread_params,
							THREAD_PRIORITY_NORMAL);	
						if (pref.schedule_entry[i].monitor_mountpoint == 1)
							Sleep(150);	
						else
							Sleep(750);
						changed = true;
					} while (pref.schedule_entry[i].thread_ptr == NULL);
					pref.schedule_entry[i].stream_running = TRUE;
					last_recording = " ";
				}
				else if (stream_count >= MAX_STREAMS)
				{
					MessageBoxA(NULL,"Maximum number of streams reached",PROGRAM_NAME,
						MB_OK|MB_ICONEXCLAMATION);
				}
			}
		} // If the end time has been reached, set terminate flag to TRUE:
		else if (((hour == pref.schedule_entry[i].end_hr
			&& min >= pref.schedule_entry[i].end_min))
			&& pref.schedule_entry[i].stream_idx >= 0
			&& pref.schedule_entry[i].thread_ptr != NULL
			&& pref.schedule_entry[i].stream_running
			&& !pref.schedule_entry[i].monitor_mountpoint)
		{
			if (stream_array[pref.schedule_entry[i].stream_idx] != NULL)
				stream_array[pref.schedule_entry[i].stream_idx]->SetTerminate(TRUE);
			pref.schedule_entry[i].stream_running = FALSE;
		} // Print out information about the currently running streams:
		else if (pref.schedule_entry[i].stream_idx >= 0
			&& pref.schedule_entry[i].stream_idx < pref.num_entries
			&& ((stream_array[pref.schedule_entry[i].stream_idx] != NULL 
			&& stream_array[pref.schedule_entry[i].stream_idx]->Done())
			|| stream_array[pref.schedule_entry[i].stream_idx] == 0))
		{
			
			
			if (stream_array[pref.schedule_entry[i].stream_idx] != NULL)
			{
				CopyString(tempstr, stream_array[pref.schedule_entry[i].stream_idx]->GetStatusMessage());
				if (strcmp(tempstr, "DONE RECORDING") == 0)
				{
					CopyString(lr, last_recording);
					if (strcmp(lr," ") == 0)
						last_recording = stream_array[pref.schedule_entry[i].stream_idx]->GetStatusMessage();
					else
						last_recording += stream_array[pref.schedule_entry[i].stream_idx]->GetStatusMessage();
					last_recording += " - ";
					last_recording += pref.schedule_entry[i].program;
					last_recording += "\r\n";
				}
				if (_strnicmp(tempstr,"IDLE",4) != 0)
				{
					m_status += stream_array[pref.schedule_entry[i].stream_idx]->GetStatusMessage();
					m_status += " - ";
					m_status += pref.schedule_entry[i].program;
					m_status += "\r\n";
				}
			}
			
			if (pref.schedule_entry[i].thread_ptr != NULL)
				WaitForSingleObject(pref.schedule_entry[i].thread_ptr,INFINITE);
			if (stream_array[pref.schedule_entry[i].stream_idx] != NULL)
			{
				delete stream_array[pref.schedule_entry[i].stream_idx];
				stream_array[pref.schedule_entry[i].stream_idx] = NULL;
			}
			if (pref.schedule_entry[i].stream_idx == stream_count-1)
				stream_count--;
			pref.schedule_entry[i].stream_running = FALSE;
			if (pref.schedule_entry[i].repeated || (pref.schedule_entry[i].monitor_mountpoint
				&& !pref.schedule_entry[i].delete_this
				&& !pref.schedule_entry[i].limited_retry))
			{				
				pref.schedule_entry[i].stream_idx = -1;
			}
			else
			{
				pref.schedule_entry[i].days ^= pref.schedule_entry[i].rec_date;
				if ((pref.schedule_entry[i].days == 0
					&& !pref.schedule_entry[i].monitor_mountpoint)
					|| (pref.schedule_entry[i].monitor_mountpoint
					&& pref.schedule_entry[i].delete_this
					&& pref.schedule_entry[i].fail_count++ > FC_MAX))
				{
					pref.schedule_entry[i].stream_idx = -2;
				}
				
				pref.schedule_entry[i].stream_idx = -1;
			
			}
			ConsolidateSchedule(&pref);
			for (j = 0; j < ignore.num_entries; j++)
				if (pref.schedule_entry[i].monitor_mountpoint 
					&& strcmp(pref.schedule_entry[i].stream_URL,ignore.ignore_entry[j].mountpoint_URL) == 0)
					ignore.ignore_entry[j].mountpoint_URL[0] = NULL;
			ConsolidateIgnoreList(&ignore);
			pref.schedule_entry[i].thread_ptr = NULL;
			status_update = true;
			UpdateData(FALSE);
			status_update = false;
		}

		if (pref.schedule_entry[i].stream_idx >= 0
			&& pref.schedule_entry[i].stream_idx < pref.num_entries)
		{
			if (stream_array[pref.schedule_entry[i].stream_idx] != NULL)
			{
				status_info = true;
				if (!pref.schedule_entry[i].monitor_server)
					stream_index[pref.schedule_entry[i].stream_idx] = i;
			}
		}
	}

	if (status_info) 
	{
		for (j = 0; j < num_entries; j++)
		{
			if (stream_index[j] >= 0) // && !pref.schedule_entry[stream_index[j]].monitor_server)
			{
				status_update = true;		
				CopyString(ptemp, stream_array[pref.schedule_entry[stream_index[j]].stream_idx]->GetStatusMessage());
				if (_strnicmp(ptemp,"IDLE",4) != 0 && pref.schedule_entry[stream_index[j]].thread_ptr != NULL)
				{
				
					m_status += stream_array[pref.schedule_entry[stream_index[j]].stream_idx]->GetStatusMessage();
					m_status += " - ";
					m_status += pref.schedule_entry[stream_index[j]].program;
					CopyString(tempstr, m_status);
					m_status += "\r\n";
					is_idle = FALSE;
				
				}
				if (!display_dlg)
				{
					UpdateNotify(tempstr);
				}
				UpdateData(FALSE);
				status_update = false;			
			}
		}
	}
	/*
	bool idle = true;

	for (int ii = 0; ii < pref.num_entries; ii++)
		if (pref.schedule_entry[ii].stream_idx > 0)
			idle = false;

	if (idle)
	{
		m_status = "IDLE\r\n";
		UpdateData(FALSE);
	}
	*/

	if (stream_ptr != NULL) 
	{
		status_update = true;
		m_status += stream_ptr->GetStatusMessage();
		CopyString(tempstr, m_status);
		m_status += "\r\n";
		UpdateData(FALSE);
		if (!display_dlg)
		{
			UpdateNotify(tempstr); //LPCTSTR(tempstr));
		}
		status_update = false;
		is_idle = FALSE;
	}

	CopyString(ctemp, last_recording);
	CopyString(dtemp, m_status);

	if (!status_info && stream_ptr == NULL 
		&& (strcmp(ctemp," ") == 0)
		|| strcmp(dtemp,"") == 0)
	{
		if (strcmp(dtemp,"") == 0
			&& strcmp(ctemp," ") != 0)
		{
			m_status += last_recording;
			CopyString(tempstr, m_status);
		}
		else			
		{
			strcpy(tempstr, "IDLE");
			m_status = "IDLE\r\n";
			if (pref.irc.update_topic && irc != NULL 
				&& pref.irc.enable_posting && !is_idle)
				irc->SetTopic("SSR currently idle");
			if (pref.irc.update_topic)
				is_idle = TRUE;
		}
		status_update = TRUE;
		UpdateData(FALSE);
		if (!display_dlg)
		{
			UpdateNotify(tempstr); //LPCTSTR(tempstr));
		}
		status_update = false;	
	}

	if (stream_ptr != NULL && stream_ptr->Done())
	{
		if (strcmp(ctemp," ") == 0)
			last_recording = stream_ptr->GetStatusMessage();
		else
			last_recording += stream_ptr->GetStatusMessage();
		last_recording += "\r\n";
		delete stream_ptr;
		stream_ptr = NULL;
	}

	if (stream_ptr != NULL)
		delete [] stream_ptr;
	if (cur_time != NULL)
		delete cur_time;

	
	if (!display_dlg)
	{
		recording = false;
		for (i = 0; i < pref.num_entries && !recording; i++)
			if (pref.schedule_entry[i].stream_running > 0)
				recording = true;
		if (!recording)
			ConvertString(minimize_icon, "icon.ico");
			//minimize_icon = "icon.ico";
		_chdir(working_dir);
		UnloadIcon();
		ConvertString(tempwchar, tempstr);
		InitNotifyIconData(GetSafeHwnd(),minimize_icon,tempwchar);
		Redraw(GetSafeHwnd());
	}
	
	////preferences_lock = fal	schedule_mutex.Unlock();

}

void CStreamrecordDlg::OnKillfocusEdit4() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	path_changed = true;
}

void CStreamrecordDlg::OnCheck1()
{
	m_shoutcast = !m_shoutcast;
	UpdateData(FALSE);
}

void CStreamrecordDlg::OnAboutProgram()
{
	CAboutDlg box;

	box.DoModal();
}

void CStreamrecordDlg::OnTimer(UINT_PTR nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	///TStreamTime stime;
	long min=0, sec=0;
	unsigned long i;
	char temp[64];
	static unsigned short move_count = 1;
	bool recording = false;

	switch (nIDEvent)
	{
	case TIMER_ID:
		if (dialog_init)
		{
			CheckForScheduledEvents();
			dialog_params.dlg_ptr = this;
			//AfxBeginThread(CheckSchedule, (LPVOID)&dialog_params, THREAD_PRIORITY_NORMAL);
			if (playing)
			{
				///	player->GetPosition(&stime);
					///min = stime.sec/60;
					////sec = stime.sec%60;
				sprintf(temp, "%2d:%2d", min, sec);
				for (i = 0; i < strlen(temp); i++)
					if (temp[i] == ' ')
						temp[i] = '0';
				m_time_elapsed = temp;
				///	if (((move_count++)%10) == 0)
				///		mplay_ctrl.SetPos(stime.sec*MSEC);
				UpdateData(FALSE);
			}
		}
		break;
	case TIMER_ID_2:
		dialog_params.dlg_ptr = this;
		/////AfxBeginThread(CheckServerStatus, (LPVOID)&dialog_params, THREAD_PRIORITY_NORMAL);
		CheckServer();

		break;
	case TIMER_ID_3:
		if (stop_box != NULL && stop_box->IsDestroyed())
		{
			stop_box->DestroyWindow();
			delete stop_box;
			stop_box = NULL;
		}
		break;
	case TIMER_ID_4:
		last_recording = " ";
		break;
	case TIMER_ID_5:
		if (dialog_init && pref.database) // && pref.no_load)
		{
			LoadDatabase(pref);
			CopySchedule(pref);
		}
			
			break;
			//	LoadDatabase(pref);
				//AfxBeginThread(LoadPref, (LPVOID)&parse_params, THREAD_PRIORITY_NORMAL);
				//LoadPreferences(PREF_FILE, pref, ignore, add);
		case TIMER_ID_6:
			if (dialog_init && pref.database)
				ResetStatus(pref);
			break;
		case TIMER_ID_7:
			if (dialog_init && pref.database)
			//	ResetStatus(pref);
		case TIMER_ID_8:
			if (dialog_init && pref.database)
				ResetConnection(pref);
			break;
		case TIMER_ID_9:
			if (dialog_init && pref.database)
				SetStatus(pref);
			break;
		default: break;
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CStreamrecordDlg::OnPlayFile()
{
	// TODO: Add your control notification handler code here
	char tmp[1024];

	CopyString(tmp, m_sound_file);

	if (paused)
	{
	///	player->Resume();
		paused = false;
		m_pause_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP7)));
	}
	/*
	else if (player->OpenFile(tmp,sfAutodetect) != 0)
	{
	///	player->Play();
		paused = false;
	///	player->GetStreamInfo(&stream_info);
		msec = stream_info.Length.ms;
		mplay_ctrl.SetRangeMin(0);
		mplay_ctrl.SetRangeMax(msec);
		mplay_ctrl.SetPos(0);
		UpdateData(FALSE);
		CheckMessagePump();
		samples = stream_info.Length.samples;
		playing = true;
		m_pause_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP7)));
	}
	*/
	else
	{
		sprintf(tmp, "Cannot open %s", tmp); //LPCTSTR(m_sound_file));
		MessageBoxA(NULL,LPCSTR(tmp),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
	}
}

void CStreamrecordDlg::OnStopFile()
{
	// TODO: Add your control notification handler code here
	paused = false;
	playing = false;
	///player->Stop();
	m_pause_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP7)));
}

void CStreamrecordDlg::OnEnKillfocusEdit5()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CStreamrecordDlg::OnPauseFile()
{
	// TODO: Add your control notification handler code

	if (paused)
	{
		///player->Resume();
		m_pause_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP7)));
	}
	else
	{
		///player->Pause();
		m_pause_button.SetBitmap(::LoadBitmap( AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP8)));
	}

	paused = !paused;
}

void CStreamrecordDlg::OnBrowseSoundFile()
{
	// TODO: Add your control notification handler code here
	char szFilters[] = "MP3 files (*.mp3)|*.mp3|OGG files (*.ogg)|*.ogg|FLAC files (*.flac)|*.flac|WAV Files (*.wav)|*.wav|All files (*.*)|*.*||";
	CFileDialog box(TRUE,LPCTSTR("MP3 files"),LPCTSTR("*.mp3"),NULL,LPCTSTR(szFilters),NULL);

	if (box.DoModal() == IDOK)
		m_sound_file = box.GetPathName();

	paused = false;
	playing = false;
	///player->Stop();

	UpdateData(FALSE);
}

void CStreamrecordDlg::OnCancel()
{
	if (MessageBoxA(NULL,"Exit program?",PROGRAM_NAME,MB_YESNO) == IDYES)
	{	
		//strncpy(pref.stream_URL,LPCTSTR(m_stream_URL),255);
		//strncpy(pref.output_filename,LPCTSTR(m_output_file),255);
		//strncpy(pref.path,LPCTSTR(m_path),1023);
		//strncpy(pref.play_file,LPCTSTR(m_sound_file),255);
		if (pref.database)
			SaveDatabase(pref);

		CopyString(pref.stream_URL, m_stream_URL);
		CopyString(pref.output_filename, m_output_file);
		CopyString(pref.path, m_path);
		CopyString(pref.play_file, m_sound_file);
		CopyString(pref.DBpassword, m_password);

		pref.play_sound_file = m_play_sound_file;
		//strncpy(pref.sound_file,LPCTSTR(m_psound_file),255);
		CopyString(pref.sound_file, m_psound_file);
		pref.no_subdirs = m_nosubdirs;
	
		pref.shoutcast = m_shoutcast;
		pref.database = m_database;
		pref.enable_dbox = m_enable_dbox;
		Cleanup();
		SavePreferences(PREF_FILE,pref,ignore,add);	
		KillTimer(TIMER_ID);
		KillTimer(TIMER_ID_2);
		CDialog::OnCancel();
	}
}
void CStreamrecordDlg::OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	long ms;
	/*
	TStreamTime stream_time;
	TTimeFormat ttf = (TTimeFormat)1;
	TSeekMethod tsm = (TSeekMethod)1;

	*pResult = 0;
	if (playing)
	{
		ms = mplay_ctrl.GetPos();
		if (ms >= msec)
		{
			ms--;
		}		
		stream_time.hms.hour = ms/3600000;
		stream_time.hms.millisecond = ((ms%3600000)%60000)%1000;
		stream_time.hms.minute = (ms%3600000)/60000;
		stream_time.hms.second = ((ms%3600000)%60000)/1000;
		stream_time.ms = ms;
		stream_time.samples = samples;
		stream_time.sec = ms/1000;
		///player->Seek(ttf,&stream_time,tsm);
	}
	*/
}



void CStreamrecordDlg::OnEnterActivationCode()
{
	// TODO: Add your control notification handler code here
	ActivationDlg box(&pref);

	box.DoModal();
	GetDlgItem(IDC_BUTTON9)->EnableWindow(!pref.code_is_good);
}


void CStreamrecordDlg::OnBnClickedCheck2()
{
	// TODO: Add your control notification handler code here
	m_play_sound_file = !m_play_sound_file;
	UpdateData(FALSE);
}

void CStreamrecordDlg::OnEnKillfocusEdit7()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
}

void CStreamrecordDlg::OnBnClickedButton10()
{
	// TODO: Add your control notification handler code here
	char szFilters[] = "MP3 files (*.mp3)|*.mp3|OGG files (*.ogg)|*.ogg|FLAC files (*.flac)|*.flac|WAV Files (*.wav)|*.wav|All files (*.*)|*.*||";
	CFileDialog box(TRUE,LPCTSTR("MP3 files"),LPCTSTR("*.mp3"),NULL,LPCTSTR(szFilters),NULL);

	//CFileDialog box(TRUE,);

	if (box.DoModal() == IDOK)
		m_psound_file = box.GetPathName();

	UpdateData(FALSE);
}

void CStreamrecordDlg::OnBnClickedCheck3()
{
	// TODO: Add your control notification handler code here
	m_nosubdirs = !m_nosubdirs;
	pref.no_subdirs = m_nosubdirs;
	UpdateData(FALSE);
}

void CStreamrecordDlg::OnBnClickedCheck4()
{
	// TODO: Add your control notification handler code here
	m_enable_sounds = !m_enable_sounds;
	pref.enable_sounds = m_enable_sounds;
	UpdateData(FALSE);
}

void CStreamrecordDlg::OnBnClickedButton11()
{
	// TODO: Add your control notification handler code here
	char szFilters[] = "MP3 files (*.mp3)|*.mp3|OGG files (*.ogg)|*.ogg|FLAC files (*.flac)|*.flac|WAV Files (*.wav)|*.wav|All files (*.*)|*.*||";
	CFileDialog box(TRUE,LPCTSTR("MP3 files"),LPCTSTR("*.mp3"),NULL,LPCTSTR(szFilters),NULL);

	if (box.DoModal() == IDOK)
		m_output_file = box.GetPathName();

	UpdateData(FALSE);
}

void CStreamrecordDlg::OnBnClickedButton12()
{
	// TODO: Add your control notification handler code here
	//StopDlg box(&pref, stream_array);
	//box.DoModal();
	if (stop_box != NULL)
	{
		stop_box->DestroyWindow();
		delete stop_box;
	}
	stop_box = new StopDlg(&pref,stream_array);
	stop_box->Create(IDD_DIALOG6);
	stop_box->ShowWindow(SW_SHOW);
}

void CStreamrecordDlg::OnBnClickedButton13()
{
	// TODO: Add your control notification handler code here
	IRCSettingsDlg box(&pref);

	if (box.DoModal() == IDOK)
	{
		SavePreferences(PREF_FILE,pref,ignore,add);
		if (irc != NULL)
		{
			irc->Stop();
			irc->CopyParams(pref.irc.server,pref.irc.channel,
				pref.irc.nickname,pref.irc.UID,pref.irc.color,
				pref.irc.background);
			if (pref.irc.enable_posting && irc != NULL)
				irc->Start();
		}
	}
}

void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CStreamrecordDlg::OnBnClickedButton14()
{
	// TODO: Add your control notification handler code here
	IRCChatDlg box(irc);

	if (irc != NULL && pref.irc.enable_posting)
		box.DoModal();
	else
		MessageBoxA(NULL,LPCSTR("No IRC session established."),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
}

void CStreamrecordDlg::OnBnClickedDatabaseSettings()
{
	CDatabaseSettingsDlg box(&pref, &ignore, &add);

	box.DoModal();

	//if (box.DoModal() == IDOK);
}

void CStreamrecordDlg::OnBnClickedSync()
{
	// TODO: Add your control notification handler code here
	LoadDatabase(pref);
	CopySchedule(pref);
}


void CStreamrecordDlg::OnBnClickedResetDatabase()
{
	// TODO: Add your control notification handler code here
	if (MessageBoxA(NULL, LPCSTR("This will delete all programs from the database. Continue?"), PROGRAM_NAME, MB_YESNO) == IDYES)
	{
		ResetTable(pref);
		MessageBoxA(NULL, LPCSTR("Database has been reset."), PROGRAM_NAME, MB_OK);
	}
}
