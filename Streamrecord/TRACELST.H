// TraceList.h : header file
//

#ifndef __tracelst_h__
#define __tracelst_h__
/////////////////////////////////////////////////////////////////////////////
// CTraceList window

class CTraceList : public CListBox
{
// Construction
public:
	CTraceList();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTraceList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTraceList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTraceList)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

public:
	void Clear()
	{
		ResetContent();
		UpdateWindow();
	}
	void AddString(const char *str)
	{
		int itemno = CListBox::AddString(LPCTSTR(str)); //(char *)str);
		SetTopIndex(itemno);
		UpdateWindow();
	}
};

/////////////////////////////////////////////////////////////////////////////
#endif
