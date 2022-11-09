#pragma once


// RecordMountpointDlg dialog

class RecordMountpointDlg : public CDialog
{
	DECLARE_DYNAMIC(RecordMountpointDlg)

public:
	RecordMountpointDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~RecordMountpointDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG5 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_monitor_cont;
	BOOL m_monitor_until_disc;
	BOOL m_record_from;
	long m_start_hr;
	long m_start_min;
	long m_end_hr;
	long m_end_min;
	BOOL ContinuousMonitor() { return m_monitor_cont; }
	BOOL MonitorUntilDisc() { return m_monitor_until_disc; }
	BOOL RecordFrom() { return m_record_from; }
	long GetStartHour() { return m_start_hr; }
	long GetStartMin() { return m_start_min; }
	long GetEndHour() { return m_end_hr; }
	long GetEndMin() { return m_end_min; }
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
};
