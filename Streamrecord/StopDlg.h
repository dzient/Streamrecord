#pragma once


// StopDlg dialog
#include "loadpref.h"
#include "afxwin.h"
#include "StreamInstance.h"

class StopDlg : public CDialog
{
	DECLARE_DYNAMIC(StopDlg)

public:
	StopDlg(const STREAMRECORD_PREFERENCES *pref, StreamPtr stream_array[], CWnd* pParent = NULL);   // standard constructor
	virtual ~StopDlg();
	void SetDestroyed(bool val) { destroyed = val; }
	bool IsDestroyed() { return destroyed; }

// Dialog Data
	enum { IDD = IDD_DIALOG6 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//afx_msg void rOnPaint();
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
	void FillList(STREAMRECORD_PREFERENCES *pref);
	//void RebuildStreamList(STREAMRECORD_PREFERENCES *pref, long ar[]);
public:
	CListBox m_recording_list;
private:
	STREAMRECORD_PREFERENCES *ppref;
	BOOL init, update;
	long *alist;
	long *stream_list;
	StreamPtr *stream_ar;
	long slistsize;
	bool destroyed;
public:
	afx_msg void OnLbnDblclkList1();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPaint();
};
