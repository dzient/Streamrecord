// RecordMountpointDlg.cpp : implementation file
//
#include "pch.h"
#include "stdafx.h"
#include "streamrecord.h"
#include "RecordMountpointDlg.h"
#include ".\recordmountpointdlg.h"


// RecordMountpointDlg dialog

IMPLEMENT_DYNAMIC(RecordMountpointDlg, CDialog)
RecordMountpointDlg::RecordMountpointDlg(CWnd* pParent /*=NULL*/)
	: CDialog(RecordMountpointDlg::IDD, pParent)
	, m_monitor_cont(FALSE)
	, m_monitor_until_disc(FALSE)
	, m_record_from(FALSE)
	, m_start_hr(0)
	, m_start_min(0)
	, m_end_hr(0)
	, m_end_min(0)
{
}

RecordMountpointDlg::~RecordMountpointDlg()
{
}

void RecordMountpointDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_start_hr);
	DDV_MinMaxLong(pDX, m_start_hr, 0, 23);
	DDX_Text(pDX, IDC_EDIT2, m_start_min);
	DDV_MinMaxLong(pDX, m_start_min, 0, 59);
	DDX_Text(pDX, IDC_EDIT3, m_end_hr);
	DDV_MinMaxLong(pDX, m_end_hr, 0, 23);
	DDX_Text(pDX, IDC_EDIT4, m_end_min);
	DDV_MinMaxLong(pDX, m_end_min, 0, 59);
	DDX_Check(pDX, IDC_RADIO1, m_monitor_cont);
	DDX_Check(pDX, IDC_RADIO2, m_monitor_until_disc);
	DDX_Check(pDX, IDC_RADIO3, m_record_from);
}


BEGIN_MESSAGE_MAP(RecordMountpointDlg, CDialog)
	ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnBnClickedRadio3)
END_MESSAGE_MAP()


// RecordMountpointDlg message handlers

void RecordMountpointDlg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	m_monitor_cont = TRUE;
	m_monitor_until_disc = m_record_from = FALSE;
}

void RecordMountpointDlg::OnBnClickedRadio2()
{
	// TODO: Add your control notification handler code here
	m_monitor_until_disc = TRUE;
	m_monitor_cont = m_record_from = FALSE;
}

void RecordMountpointDlg::OnBnClickedRadio3()
{
	// TODO: Add your control notification handler code here
	m_record_from = TRUE;
	m_monitor_cont = m_monitor_until_disc = FALSE;
}
