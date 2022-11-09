#pragma once

#include "loadpref.h"
// MountpointDlg dialog

char * tokenizer(char* &ptr, const char *delimiter);

class MountpointDlg : public CDialog
{
	DECLARE_DYNAMIC(MountpointDlg)

public:
	MountpointDlg(STREAMRECORD_PREFERENCES *pref,  
		long idx, char enable_ig[], unsigned char ignore_d[],
		char ignore_start_h[], char ignore_start_m[],
		char ignore_end_h[], char ignore_end_m[],
		char enable_mp_ignore[],
		char ignore_mp[][16],
		char enable_mp_ignore2[],
		char ignore_mp2[][16],
		char enable_mp_ignore3[],
		char ignore_mp3[][16],
		char enable_mp_ignore4[],
		char ignore_mp4[][32],
		char ignore_ext[][5],
		CWnd* pParent = NULL);   // standard constructor
	virtual ~MountpointDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG4 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	char *enable_ignore;
	unsigned char *ignore_day;
	char *ignore_start_hr;
	char *ignore_start_min;
	char *ignore_end_hr;
	char *ignore_end_min;
	char *enable_mountpt_ignore;
	char *enable_mountpt_ignore2;
	char *enable_mountpt_ignore3;
	char *enable_mountpt_ignore4;
	char *ignore_mountpt[IGNORE_MP_MAX];
	char *ignore_mountpt2[IGNORE_MP_MAX];
	char *ignore_mountpt3[IGNORE_MP_MAX*2];
	char *ignore_mountpt4[IGNORE_MP_MAX*4];

	DECLARE_MESSAGE_MAP()
public:
	// // enables the first ignore mask
	BOOL m_ignore_time1;
	// // enable the second ignore mask
	BOOL m_ignore_time2;
	// // enables the third ignore mask
	BOOL m_ignore_time3;
	// // first start hour
	BYTE m_start_hr1;
	// First start min
	BYTE m_start_min1;
	// // First end hour
	BYTE m_end_hr1;
	// // first end hour
	BYTE m_end_min1;
	// // ignore start hour 2
	BYTE m_start_hr2;
	// // ignore start min 2
	BYTE m_start_min2;
	// // ignore end hour 2
	BYTE m_end_hr2;
	// ignore end min 2
	BYTE m_end_min2;
	// ignore start hour 3
	BYTE m_start_hr3;
	// ignore start min 3
	BYTE m_start_min3;
	// ignore end hour 3
	BYTE m_end_hr3;
	BYTE m_end_min3;
protected:
	STREAMRECORD_PREFERENCES *ppref;
	IGNORE_LIST *pignore;
	SCHEDULE_ADD_LIST *padd;
	long cur_idx;
	
public:
	afx_msg void OnBnClickedOk();
	// Ignore day 1
	CString m_day1;
	// Ignore day 2
	CString m_day2;
	// Ignore day 3
	CString m_day3;
	BOOL m_ignore_mp1;
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	BOOL m_ignore_mp2;
	BOOL m_ignore_mp3;
	afx_msg void OnBnClickedCheck6();
	BOOL m_ignore_mp4;
	afx_msg void OnBnClickedCheck7();
	BOOL m_ignore_mp5;
	afx_msg void OnBnClickedCheck13();
	BOOL m_ignore_mp6;
	afx_msg void OnBnClickedCheck14();
	CString m_mp1;
	CString m_mp2;
	afx_msg void OnEnKillfocusEdit13();
	afx_msg void OnEnKillfocusEdit14();
	afx_msg void OnEnKillfocusEdit15();
	CString m_mp3;
	CString m_mp4;
	afx_msg void OnEnKillfocusEdit16();
	CString m_mp5;
	CString m_mp6;
	afx_msg void OnBnClickedCheck8();
	afx_msg void OnBnClickedCheck9();
	afx_msg void OnBnClickedCheck10();
	afx_msg void OnBnClickedCheck11();
	afx_msg void OnBnClickedCheck12();
	afx_msg void OnBnClickedCheck16();
	CString m_mp7;
	CString m_mp8;
	CString m_mp9;
	CString m_mp10;
	CString m_mp11;
	CString m_mp12;
	CString m_mp[12];
	BOOL m_ignore[12];
	CString m_mpp2[24];
	BOOL m_ignore2[24];
	BOOL m_ignore_mp7;
	BOOL m_ignore_mp8;
	BOOL m_ignore_mp9;
	BOOL m_ignore_mp10;
	BOOL m_ignore_mp11;
	BOOL m_ignore_mp12;
	CString m_ignore_ext;
	char ignore_ext_ptr[IGNORE_EXT_MAX][5];
private:
	void CreateMountpointList(char list[], char extlist[]);
	void ParseMountpointList(char list[], char extlist[]);
public:
	CString m_ignorelist;
};
