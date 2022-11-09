#if !defined(AFX_VIEWSCHEDULEDLG_H__965A7F16_1950_4FA0_AEB4_220942FF87AA__INCLUDED_)
#define AFX_VIEWSCHEDULEDLG_H__965A7F16_1950_4FA0_AEB4_220942FF87AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewScheduleDlg.h : header file
//

#include "loadpref.h"
#include "resource.h"
#include "TRACELST.H"

 
/////////////////////////////////////////////////////////////////////////////
// ViewScheduleDlg dialog
void ConvertTimeToString(DWORD n, char ar[]);
void GetDays(unsigned char days, char ar[]);
long FirstDay(long n);

typedef struct llist
{
	llist *prev, *next;
	long n;
	llist()
	{
		prev = next = NULL;
		n = -1;
	}
} linked_list;

class ScheduleDlg;

class ViewScheduleDlg : public CDialog
{
// Construction
public:
	ViewScheduleDlg(STREAMRECORD_PREFERENCES *pref,
		IGNORE_LIST *ignore,
		SCHEDULE_ADD_LIST *add,
		CWnd* pParent = NULL);   // standard constructor
	~ViewScheduleDlg();
// Dialog Data
	//{{AFX_DATA(ViewScheduleDlg)
	enum { IDD = IDD_DIALOG2 };
	CListBox	m_schedule_ls;
	//}}AFX_DATA
protected:
	void FillList();
	void DeleteList();

private:
	STREAMRECORD_PREFERENCES *ppref; 
	IGNORE_LIST *pignore;
	SCHEDULE_ADD_LIST *padd;
	BOOL init;
	linked_list *list, *temp;
	ScheduleDlg *schedule_box;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewScheduleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ViewScheduleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnLbnDblclkList1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	
	
	///afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnOk();
	afx_msg void OnScheduler();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWSCHEDULEDLG_H__965A7F16_1950_4FA0_AEB4_220942FF87AA__INCLUDED_)
