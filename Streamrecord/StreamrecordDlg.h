//-------------------------------------------
// David Zientara
// 10-27-2022
//
// StreamrecordDlg.h
//
// Header file for StreamrecordDlg class
//
//---------------------------------------------

// streamrecordDlg.h : header file
//

#if !defined(AFX_STREAMRECORDDLG_H__0CE597BF_3610_4DCE_A08E_BAD0D2D66186__INCLUDED_)
#define AFX_STREAMRECORDDLG_H__0CE597BF_3610_4DCE_A08E_BAD0D2D66186__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "resource.h"

#include "loadpref.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "afxext.h"

#include "StopDlg.h"
#include "ircbotMFC.h"

/////////////////////////////////////////////////////////////////////////////
// CStreamrecordDlg dialog
UINT ParseServerData(LPVOID pParam);
UINT RecordStream(LPVOID pParam);

class CStreamrecordDlg : public CDialog
{
// Construction
public:
	CStreamrecordDlg(CWnd* pParent = NULL);	// standard constructor
	void Cleanup();

// Dialog Data
	//{{AFX_DATA(CStreamrecordDlg)
	enum { IDD = IDD_STREAMRECORD_DIALOG };
	CString	m_stream_URL;
	CString	m_output_file;
	CString	m_status;
	CString	m_path;
	BOOL m_shoutcast;
	CString m_time_elapsed;
	BOOL m_database;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStreamrecordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	STREAMRECORD_PREFERENCES pref;
	IGNORE_LIST ignore;
	SCHEDULE_ADD_LIST add;
	BOOL shutdown;
	CString last_recording;
private:
	StopDlg *stop_box;
	IrcBotMFC *irc;

protected:

	// Generated message map functions
	//{{AFX_MSG(CStreamrecordDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnConnect();
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnStop();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSchedule();
	afx_msg void OnKillfocusEdit4();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCheck1();
	afx_msg void OnAboutProgram();
	afx_msg void OnBnClickedDatabase();
	//}}AFX_MSG
	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	
public:
	void CheckForScheduledEvents();
	void CheckServer();
	void CheckMessagePump();
	// Path + name of sound file
	CString m_sound_file;
	afx_msg void OnPlayFile();
	afx_msg void OnStopFile();
	afx_msg void OnEnKillfocusEdit5();
	afx_msg void OnPauseFile();
	afx_msg void OnBrowseSoundFile();
	CSliderCtrl mplay_ctrl;
	afx_msg void OnNMReleasedcaptureSlider1(NMHDR *pNMHDR, LRESULT *pResult);
	
	// // Play button for the MP3 player

	CButton m_play_button;
	// // pause button for the MP3 player
	CButton m_pause_button;
	CButton m_stop_button; 
	afx_msg void OnEnterActivationCode();
	// // play a sound file instead of having speaker beep
	BOOL m_play_sound_file;
	afx_msg void OnBnClickedCheck2();
	// // Sound file to play when recording starts
	CString m_psound_file;
	afx_msg void OnEnKillfocusEdit7();
	afx_msg void OnBnClickedButton10();
	BOOL m_nosubdirs;
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	// // Enable sounds to signal start of recording
	BOOL m_enable_sounds;
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	afx_msg void OnBnClickedDatabaseSettings();

	afx_msg void OnBnClickedSync();
	CString m_password;
	afx_msg void OnBnClickedResetDatabase();
	// //Enable database settings dialog box on startup
	BOOL m_enable_dbox;
	// //Enables or disables push notifications
	BOOL m_pushover;
	afx_msg void OnBnClickedEnableDatabaseSettings();
	afx_msg void OnBnClickedCheck7();
	afx_msg void OnBnClickedButton18();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STREAMRECORDDLG_H__0CE597BF_3610_4DCE_A08E_BAD0D2D66186__INCLUDED_)
