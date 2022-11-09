#if !defined(AFX_SCHEDULEDLG_H__6F0047E9_8FAB_48EE_9383_5514C8FC7DDD__INCLUDED_)
#define AFX_SCHEDULEDLG_H__6F0047E9_8FAB_48EE_9383_5514C8FC7DDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScheduleDlg.h : header file
//

#include "loadpref.h"
#include "ViewScheduleDlg.h"

/////////////////////////////////////////////////////////////////////////////
// ScheduleDlg dialog

class ScheduleDlg : public CDialog
{
// Construction
public:
	ScheduleDlg(STREAMRECORD_PREFERENCES *pref, long idx = -1,
		IGNORE_LIST *ignore = NULL,
		SCHEDULE_ADD_LIST *add = NULL,
		CWnd* pParent = NULL);   // standard constructor

	bool IsDestroyed() { return destroyed; }
	void SetDestroyed(bool val) { destroyed = val; }
	bool IsUpdated() { return updated; }
	void SetUpdated(bool val) { updated = val; }
	void MoveToEntry(long idx);
	void UpdateTime(STREAMRECORD_PREFERENCES& pref, int idx);
protected:
	void DisableControls();

public:

// Dialog Data
	//{{AFX_DATA(ScheduleDlg)
	enum { IDD = IDD_DIALOG1 };
	BOOL	dow[7];
	BOOL	m_sunday;
	BOOL	m_monday;
	BOOL	m_tuesday;
	BOOL	m_wednesday;
	BOOL	m_thursday;
	BOOL	m_friday;
	BOOL	m_saturday;
	DWORD	m_start_hour;
	DWORD	m_start_minute;
	DWORD	m_end_hour;
	DWORD	m_end_minute;
	BOOL	m_one_time;
	BOOL	m_repeating;
	CString	m_program;
	CString	m_stream_URL;
	BOOL	m_shoutcast;
	//CListBox m_bitrate;
	CString	m_encode_list;
	BOOL	m_reencode;
	BOOL	m_delete_old;
	CString m_genre;
	//}}AFX_DATA
private:
	BOOL repeating;
	STREAMRECORD_PREFERENCES *ppref; 
	IGNORE_LIST *pignore;
	SCHEDULE_ADD_LIST *padd;
	long pref_counter;
	long cur_idx;
	bool entry_changed;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ScheduleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	private:
		void CopyScheduleInfo(const STREAMRECORD_PREFERENCES *pref, 
			const SCHEDULE_ADD_LIST *add, long i);
		void ConsolidateSchedule(STREAMRECORD_PREFERENCES *pref, 
			SCHEDULE_ADD_LIST *add, long idx);
		void CommitChanges(BOOL dialog_box=TRUE);
	protected:
	// Generated message map functions
	//{{AFX_MSG(ScheduleDlg)
	afx_msg void OnAdd();
	afx_msg void OnCheck1();
	afx_msg void OnCheck2();
	afx_msg void OnCheck3();
	afx_msg void OnCheck4();
	afx_msg void OnCheck5();
	afx_msg void OnCheck6();
	afx_msg void OnCheck7();
	afx_msg void OnKillfocusEdit1();
	afx_msg void OnKillfocusEdit2();
	afx_msg void OnKillfocusEdit3();
	afx_msg void OnKillfocusEdit4();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void OnKillfocusEdit5();
	virtual void OnOK();
	afx_msg void OnKillfocusEdit6();
	virtual void OnCancel();
	afx_msg void OnViewSchedule();
	afx_msg void OnBack();
	afx_msg void OnForward();
	afx_msg void OnEditCurrent();
	afx_msg void OnDelete();
	afx_msg	void OnCheck8();
	afx_msg	void OnCheck9();
	afx_msg	void OnCheck10();
	afx_msg void OnKillfocusCombo1();
	afx_msg void OnCbnKillfocusCombo4();
	//}}AFX_MSG
	// Generated message map functions
	//{{AFX_MSG(ViewScheduleDlg)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	private: 
		bool destroyed;
		bool updated;
		char enable_ignore[3];
		unsigned char ignore_day[3];
		char ignore_start_hr[3];
		char ignore_start_min[3];
		char ignore_end_hr[3];
		char ignore_end_min[3];
		char enable_mp_ignore[IGNORE_MP_MAX];
		char ignore_mp[IGNORE_MP_MAX][16];
		char enable_mp_ignore2[IGNORE_MP_MAX];
		char ignore_mp2[IGNORE_MP_MAX][16];
		char enable_mp_ignore3[IGNORE_MP_MAX*2];
		char ignore_mp3[IGNORE_MP_MAX*2][16];
		char enable_mp_ignore4[IGNORE_MP_MAX*4];
		char ignore_mp4[IGNORE_MP_MAX*4][32];
		char ignore_ext[IGNORE_EXT_MAX][5];
public:
	// Allows for continuous monitoring of a mountpoint on an Icecast or Shoutcast server
	
	// // Enables or disables continuous monitoring of a mountpoint
	BOOL m_monitor_mountpoint;
	afx_msg void OnBnClickedCheck11();
	afx_msg void OnAdvancedMountpointOptions();
	// Check this to monitor a server
	BOOL m_monitor_server;
	afx_msg void OnBnClickedCheck12();
	CString m_level_list;
	afx_msg void OnCbnKillfocusCombo3();
	afx_msg void OnBnClickedCheck15();
	BOOL m_delete_this;
	afx_msg void OnEnKillfocusEdit7();
	// // Timeout (in seconds)
	short m_timeout;
	BOOL m_start_pm;
	BOOL m_end_pm;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCHEDULEDLG_H__6F0047E9_8FAB_48EE_9383_5514C8FC7DDD__INCLUDED_)
