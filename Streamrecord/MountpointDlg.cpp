// MountpointDlg.cpp : implementation file
//

#include "stdafx.h"
#include "streamrecord.h"
#include "MountpointDlg.h"
#include ".\mountpointdlg.h"

#include <ctype.h>


// MountpointDlg dialog
void tokenize(char *str, const char *delimiter, char* &ptr)
{
	if (ptr != NULL)
	{
		ptr = strtok(str,delimiter);
	}

}

//========================================================================
// Given a pointer to a string with a series of tokens separated by a
// delimiter, do the following:
// (1) Find the next token
// (2) Replace the token with the NULL character
// (3) Return a pointer to the token
// (4) Update the original pointer so it points to the next token
//========================================================================
char * tokenizer(char* &ptr, const char *delimiter)
{
	char *retval;

	while (ptr[0] != NULL && isspace(ptr[0]))
		ptr++;
	if (ptr[0] == NULL)
		return NULL;
	retval = ptr;
	while (ptr[0] != NULL && strncmp(ptr,delimiter,strlen(delimiter)) != 0)
		ptr++;
	if (ptr[0] == NULL)
		return retval;
	ptr[0] = NULL;
	ptr++;

	return retval;
}


IMPLEMENT_DYNAMIC(MountpointDlg, CDialog)
MountpointDlg::MountpointDlg(STREAMRECORD_PREFERENCES *pref, 
							 long idx,
							 char enable_ig[], unsigned char ignore_d[],
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
							 CWnd* pParent /*=NULL*/)
	: CDialog(MountpointDlg::IDD, pParent)
	, m_ignore_time1(FALSE)
	, m_ignore_time2(FALSE)
	, m_ignore_time3(FALSE)
	, m_start_hr1(0)
	, m_start_min1(0)
	, m_end_hr1(0)
	, m_end_min1(0)
	, m_start_hr2(0)
	, m_start_min2(0)
	, m_end_hr2(0)
	, m_end_min2(0)
	, m_start_hr3(0)
	, m_start_min3(0)
	, m_end_hr3(0)
	, m_end_min3(0)
	, m_day1(_T(""))
	, m_day2(_T(""))
	, m_day3(_T(""))
	, m_ignore_mp1(FALSE)
	, m_ignore_mp2(FALSE)
	, m_ignore_mp3(FALSE)
	, m_ignore_mp4(FALSE)
	, m_ignore_mp5(FALSE)
	, m_ignore_mp6(FALSE)
	, m_ignore_mp7(FALSE)
	, m_ignore_mp8(FALSE)
	, m_ignore_mp9(FALSE)
	, m_ignore_mp10(FALSE)
	, m_ignore_mp11(FALSE)
	, m_ignore_mp12(FALSE)
	, m_mp1(_T(""))
	, m_mp2(_T(""))
	, m_mp3(_T(""))
	, m_mp4(_T(""))
	, m_mp5(_T(""))
	, m_mp6(_T(""))
	, m_mp7(_T(""))
	, m_mp8(_T(""))
	, m_mp9(_T(""))
	, m_mp10(_T(""))
	, m_mp11(_T(""))
	, m_mp12(_T(""))
	, m_ignorelist(_T(""))
	, m_ignore_ext(_T(""))
{
	long i;
	const char *days[7] = { "Sunday","Monday","Tuesday","Wednesday",
		"Thursday","Friday","Saturday" };
	char list[1024], fileext[1024];

	ppref = pref;
	cur_idx = idx;

	enable_ignore = enable_ig;
	ignore_day = ignore_d;
	ignore_start_hr = ignore_start_h;
	ignore_start_min = ignore_start_m;
	ignore_end_hr = ignore_end_h;
	ignore_end_min = ignore_end_m;
	enable_mountpt_ignore = enable_mp_ignore;
	enable_mountpt_ignore2 = enable_mp_ignore2;
	enable_mountpt_ignore3 = enable_mp_ignore3;
	enable_mountpt_ignore4 = enable_mp_ignore4;

	for (i = 0; i < 6; i++)
	{
		ignore_mountpt[i] = ignore_mp[i];
		ignore_mountpt2[i] = ignore_mp2[i];
	}
	for (i = 0; i < 12; i++)
	{
		ignore_mountpt3[i] = ignore_mp3[i];
		ignore_mountpt4[i] = ignore_mp4[i];
		ignore_mountpt4[i+12] = ignore_mp4[i+12];
	}
	
	m_ignore_time1 = enable_ignore[0];
	m_ignore_time2 = enable_ignore[1];
	m_ignore_time3 = enable_ignore[2];

	m_start_hr1 = ignore_start_hr[0];
	m_start_min1 = ignore_start_min[0];
	m_end_hr1 = ignore_end_hr[0];
	m_end_min1 = ignore_end_min[0];

	m_start_hr2 = ignore_start_hr[1];
	m_start_min2 = ignore_start_min[1];
	m_end_hr2 = ignore_end_hr[1];
	m_end_min2 = ignore_end_min[1];

	m_start_hr3 = ignore_start_hr[2];
	m_start_min3 = ignore_start_min[2];
	m_end_hr3 = ignore_end_hr[2];
	m_end_min3 = ignore_end_min[2];

	m_day1 = m_day2 = m_day3 = "Sunday";

	for (i = 0; i < 7; i++)
		if ((ignore_day[0]>>i)&0x1)
		{
			m_day1 = days[i];
			break;
		}

	for (i = 0; i < 7; i++)
		if ((ignore_day[1]>>i)&0x1)
		{
			m_day2 = days[i];
			break;
		}

	for (i = 0; i < 7; i++)
		if ((ignore_day[2]>>i)&0x1)
		{
			m_day3 = days[i];
			break;
		}

	m_ignore_mp1 = enable_mp_ignore[0];
	m_ignore_mp2 = enable_mp_ignore[1];
	m_ignore_mp3 = enable_mp_ignore[2];
	m_ignore_mp4 = enable_mp_ignore[3];
	m_ignore_mp5 = enable_mp_ignore[4];
	m_ignore_mp6 = enable_mp_ignore[5];

	m_ignore_mp7 = enable_mp_ignore[0];
	m_ignore_mp8 = enable_mp_ignore[1];
	m_ignore_mp9 = enable_mp_ignore[2];
	m_ignore_mp10 = enable_mp_ignore[3];
	m_ignore_mp11 = enable_mp_ignore[4];
	m_ignore_mp12 = enable_mp_ignore[5];

	m_mp1 = ignore_mp[0];
	m_mp2 = ignore_mp[1];
	m_mp3 = ignore_mp[2];
	m_mp4 = ignore_mp[3];
	m_mp5 = ignore_mp[4];
	m_mp6 = ignore_mp[5];

	m_mp7 = ignore_mp[6];
	m_mp8 = ignore_mp[7];
	m_mp9 = ignore_mp[8];
	m_mp10 = ignore_mp[9];
	m_mp11 = ignore_mp[10];
	m_mp12 = ignore_mp[11];

	for (i = 0; i < 12; i++)
	{
		m_ignore[i] = enable_mp_ignore3[i];
		m_mp[i] = ignore_mp3[i];
		m_ignore2[i] = enable_mp_ignore4[i];
		m_mpp2[i] = ignore_mp4[i];
		m_ignore2[i+12] = enable_mp_ignore4[i+12];
		m_mpp2[i+12] = ignore_mp4[i+12];
	}
	for (i = 0; i < IGNORE_EXT_MAX; i++)
	{
		strcpy(ignore_ext_ptr[i],ignore_ext[i]);
	}
	CreateMountpointList(list,fileext);
	m_ignorelist = list;
	m_ignore_ext = fileext;

}

MountpointDlg::~MountpointDlg()
{
}

void MountpointDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK1, m_ignore_time1);
	DDX_Check(pDX, IDC_CHECK2, m_ignore_time2);
	DDX_Check(pDX, IDC_CHECK3, m_ignore_time3);
	DDX_Text(pDX, IDC_EDIT1, m_start_hr1);
	DDV_MinMaxByte(pDX, m_start_hr1, 0, 23);
	DDX_Text(pDX, IDC_EDIT2, m_start_min1);
	DDV_MinMaxByte(pDX, m_start_min1, 0, 59);
	DDX_Text(pDX, IDC_EDIT3, m_end_hr1);
	DDV_MinMaxByte(pDX, m_end_hr1, 0, 23);
	DDX_Text(pDX, IDC_EDIT4, m_end_min1);
	DDV_MinMaxByte(pDX, m_end_min1, 0, 59);
	DDX_Text(pDX, IDC_EDIT5, m_start_hr2);
	DDV_MinMaxByte(pDX, m_start_hr2, 0, 23);
	DDX_Text(pDX, IDC_EDIT6, m_start_min2);
	DDV_MinMaxByte(pDX, m_start_min2, 0, 59);
	DDX_Text(pDX, IDC_EDIT7, m_end_hr2);
	DDV_MinMaxByte(pDX, m_end_hr2, 0, 23);
	DDX_Text(pDX, IDC_EDIT8, m_end_min2);
	DDV_MinMaxByte(pDX, m_end_min2, 0, 59);
	DDX_Text(pDX, IDC_EDIT9, m_start_hr3);
	DDV_MinMaxByte(pDX, m_start_hr3, 0, 23);
	DDX_Text(pDX, IDC_EDIT10, m_start_min3);
	DDV_MinMaxByte(pDX, m_start_min3, 0, 59);
	DDX_Text(pDX, IDC_EDIT11, m_end_hr3);
	DDV_MinMaxByte(pDX, m_end_hr3, 0, 23);
	DDX_Text(pDX, IDC_EDIT12, m_end_min3);
	DDV_MinMaxByte(pDX, m_end_min3, 0, 59);
	DDX_CBString(pDX, IDC_COMBO1, m_day1);
	DDV_MaxChars(pDX, m_day1, 16);
	DDX_CBString(pDX, IDC_COMBO2, m_day2);
	DDV_MaxChars(pDX, m_day2, 16);
	DDX_CBString(pDX, IDC_COMBO3, m_day3);
	DDV_MaxChars(pDX, m_day3, 16);
	DDX_Check(pDX, IDC_CHECK4, m_ignore_mp1);
	DDX_Check(pDX, IDC_CHECK5, m_ignore_mp2);
	DDX_Check(pDX, IDC_CHECK6, m_ignore_mp3);
	DDX_Check(pDX, IDC_CHECK7, m_ignore_mp4);
	DDX_Check(pDX, IDC_CHECK13, m_ignore_mp5);
	DDX_Check(pDX, IDC_CHECK14, m_ignore_mp6);
	DDX_Text(pDX, IDC_EDIT13, m_mp1);
	DDX_Text(pDX, IDC_EDIT14, m_mp2);
	DDX_Text(pDX, IDC_EDIT15, m_mp3);
	DDX_Text(pDX, IDC_EDIT16, m_mp4);
	DDX_Text(pDX, IDC_EDIT17, m_mp5);
	DDX_Text(pDX, IDC_EDIT18, m_mp6);

	DDX_Check(pDX, IDC_CHECK8, m_ignore_mp7);
	DDX_Check(pDX, IDC_CHECK9, m_ignore_mp8);
	DDX_Check(pDX, IDC_CHECK10, m_ignore_mp9);
	DDX_Check(pDX, IDC_CHECK11, m_ignore_mp10);
	DDX_Check(pDX, IDC_CHECK12, m_ignore_mp11);
	DDX_Check(pDX, IDC_CHECK16, m_ignore_mp12);
	DDX_Text(pDX, IDC_EDIT19, m_mp7);
	DDX_Text(pDX, IDC_EDIT20, m_mp8);
	DDX_Text(pDX, IDC_EDIT21, m_mp9);
	DDX_Text(pDX, IDC_EDIT22, m_mp10);
	DDX_Text(pDX, IDC_EDIT23, m_mp11);
	DDX_Text(pDX, IDC_EDIT24, m_mp12);

	DDX_Text(pDX, IDC_EDIT25, m_ignorelist);

	DDX_Text(pDX, IDC_EDIT26, m_ignore_ext);
}


BEGIN_MESSAGE_MAP(MountpointDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK4, OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK5, OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK6, OnBnClickedCheck6)
	ON_BN_CLICKED(IDC_CHECK7, OnBnClickedCheck7)
	ON_BN_CLICKED(IDC_CHECK13, OnBnClickedCheck13)
	ON_BN_CLICKED(IDC_CHECK14, OnBnClickedCheck14)
	ON_EN_KILLFOCUS(IDC_EDIT13, OnEnKillfocusEdit13)
	ON_EN_KILLFOCUS(IDC_EDIT14, OnEnKillfocusEdit14)
	ON_EN_KILLFOCUS(IDC_EDIT15, OnEnKillfocusEdit15)
	ON_EN_KILLFOCUS(IDC_EDIT16, OnEnKillfocusEdit16)
	ON_BN_CLICKED(IDC_CHECK8, OnBnClickedCheck8)
	ON_BN_CLICKED(IDC_CHECK9, OnBnClickedCheck9)
	ON_BN_CLICKED(IDC_CHECK10, OnBnClickedCheck10)
	ON_BN_CLICKED(IDC_CHECK11, OnBnClickedCheck11)
	ON_BN_CLICKED(IDC_CHECK12, OnBnClickedCheck12)
	ON_BN_CLICKED(IDC_CHECK16, OnBnClickedCheck16)
END_MESSAGE_MAP()


// MountpointDlg message handlers

void MountpointDlg::OnBnClickedOk()
{
	long i;
	char day[3][1024];
	char temp1[1024], temp2[1024];
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	CopyString(temp1, m_ignorelist);
	CopyString(temp2, m_ignore_ext);

	ParseMountpointList(temp1, temp2); //(char *)LPCTSTR(m_ignorelist),(char *)LPCTSTR(m_ignore_ext));
	
	enable_ignore[0] = m_ignore_time1;
	enable_ignore[1] = m_ignore_time2;
	enable_ignore[2] = m_ignore_time3;

	ignore_start_hr[0] = m_start_hr1;
	ignore_start_min[0] = m_start_min1;
	ignore_end_hr[0] = m_end_hr1;
	ignore_end_min[0] = m_end_min1;

	ignore_start_hr[1] = m_start_hr2;
	ignore_start_min[1] = m_start_min2;
	ignore_end_hr[1] = m_end_hr2;
	ignore_end_min[1] = m_end_min2;

	ignore_start_hr[2] = m_start_hr3;
	ignore_start_min[2] = m_start_min3;
	ignore_end_hr[2] = m_end_hr3;
	ignore_end_min[2] = m_end_min3;
	
	CopyString(day[0], m_day1);

	if (_strcmpi(day[0],"Sunday") == 0)
		ignore_day[0] = 0x1;
	else if (_strcmpi(day[0],"Monday") == 0)
		ignore_day[0] = 0x2;
	else if (_strcmpi(day[0],"Tuesday") == 0)
		ignore_day[0] = 0x4;
	else if (_strcmpi(day[0],"Wednesday") == 0)
		ignore_day[0] = 0x8;
	else if (_strcmpi(day[0],"Thursday") == 0)
		ignore_day[0] = 0x10;
	else if (_strcmpi(day[0],"Friday") == 0)
		ignore_day[0] = 0x20;
	else if (_strcmpi(day[0],"Saturday") == 0)
		ignore_day[0] = 0x40;

	CopyString(day[1], m_day2);

	if (_strcmpi(day[1],"Sunday") == 0)
		ignore_day[1] = 0x1;
	else if (_strcmpi(day[1],"Monday") == 0)
		ignore_day[1] = 0x2;
	else if (_strcmpi(day[1],"Tuesday") == 0)
		ignore_day[1] = 0x4;
	else if (_strcmpi(day[1],"Wednesday") == 0)
		ignore_day[1] = 0x8;
	else if (_strcmpi(day[1],"Thursday") == 0)
		ignore_day[1] = 0x10;
	else if (_strcmpi(day[1],"Friday") == 0)
		ignore_day[1] = 0x20;
	else if (_strcmpi(day[1],"Saturday") == 0)
		ignore_day[1] = 0x40;

	CopyString(day[2], m_day2);

	if (_strcmpi(day[2],"Sunday") == 0)
		ignore_day[2] = 0x1;
	else if (_strcmpi(day[2],"Monday") == 0)
		ignore_day[2] = 0x2;
	else if (_strcmpi(day[2],"Tuesday") == 0)
		ignore_day[2] = 0x4;
	else if (_strcmpi(day[2],"Wednesday") == 0)
		ignore_day[2] = 0x8;
	else if (_strcmpi(day[2],"Thursday") == 0)
		ignore_day[2] = 0x10;
	else if (_strcmpi(day[2],"Friday") == 0)
		ignore_day[2] = 0x20;
	else if (_strcmpi(day[2],"Saturday") == 0)
		ignore_day[2] = 0x40;

	enable_mountpt_ignore[0] = m_ignore_mp1;
	enable_mountpt_ignore[1] = m_ignore_mp2;
	enable_mountpt_ignore[2] = m_ignore_mp3;
	enable_mountpt_ignore[3] = m_ignore_mp4;
	enable_mountpt_ignore[4] = m_ignore_mp5;
	enable_mountpt_ignore[5] = m_ignore_mp6;

	CopyString(ignore_mountpt[0], m_mp1);
	CopyString(ignore_mountpt[1], m_mp2);
	CopyString(ignore_mountpt[2], m_mp3);
	CopyString(ignore_mountpt[3], m_mp4);
	CopyString(ignore_mountpt[4], m_mp5);
	CopyString(ignore_mountpt[5], m_mp6);
	
	//strncpy(ignore_mountpt[0],LPCTSTR(m_mp1),16);
	//strncpy(ignore_mountpt[1],LPCTSTR(m_mp2),16);
	//strncpy(ignore_mountpt[2],LPCTSTR(m_mp3),16);
	//strncpy(ignore_mountpt[3],LPCTSTR(m_mp4),16);
	//strncpy(ignore_mountpt[4],LPCTSTR(m_mp5),16);
	//strncpy(ignore_mountpt[5],LPCTSTR(m_mp6),16);

	enable_mountpt_ignore2[0] = m_ignore_mp7;
	enable_mountpt_ignore2[1] = m_ignore_mp8;
	enable_mountpt_ignore2[2] = m_ignore_mp9;
	enable_mountpt_ignore2[3] = m_ignore_mp10;
	enable_mountpt_ignore2[4] = m_ignore_mp11;
	enable_mountpt_ignore2[5] = m_ignore_mp12;

	CopyString(ignore_mountpt2[0], m_mp7);
	CopyString(ignore_mountpt2[1], m_mp8);
	CopyString(ignore_mountpt2[2], m_mp9);
	CopyString(ignore_mountpt2[3], m_mp10);
	CopyString(ignore_mountpt2[4], m_mp11);
	CopyString(ignore_mountpt2[5], m_mp12);
	
	//strncpy(ignore_mountpt2[0],LPCTSTR(m_mp7),16);
	//strncpy(ignore_mountpt2[1],LPCTSTR(m_mp8),16);
	//strncpy(ignore_mountpt2[2],LPCTSTR(m_mp9),16);
	//strncpy(ignore_mountpt2[3],LPCTSTR(m_mp10),16);
	//strncpy(ignore_mountpt2[4],LPCTSTR(m_mp11),16);
	//strncpy(ignore_mountpt2[5],LPCTSTR(m_mp12),16);

	for (i = 0; i < 12; i++)
	{
		enable_mountpt_ignore3[i] = m_ignore[i];
		CopyString(ignore_mountpt3[i], m_mp[i]);
		//strcpy(ignore_mountpt3[i],LPCTSTR(m_mp[i]));
		enable_mountpt_ignore4[i] = m_ignore2[i];
		CopyString(ignore_mountpt4[i], m_mpp2[i]);
		//strcpy(ignore_mountpt4[i],LPCTSTR(m_mpp2[i]));
	}
	
	CDialog::OnOK();
}

void MountpointDlg::OnBnClickedCheck4()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck5()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck6()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck7()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck13()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck14()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnEnKillfocusEdit13()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnEnKillfocusEdit14()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnEnKillfocusEdit15()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnEnKillfocusEdit16()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck8()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck9()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck10()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck11()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck12()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::OnBnClickedCheck16()
{
	// TODO: Add your control notification handler code here
}

void MountpointDlg::CreateMountpointList(char list[], char extlist[])
{
	long i;
	char temp[128];

	strcpy(list," ");

	if (m_ignore_mp1)
	{
		CopyString(temp, m_mp1);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp1));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp2)
	{	
		CopyString(temp, m_mp2);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp2));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp3)
	{
		CopyString(temp, m_mp3);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp3));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp4)
	{
		CopyString(temp, m_mp4);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp4));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp5)
	{
		CopyString(temp, m_mp5);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp5));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp6)
	{
		CopyString(temp, m_mp6);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp6));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp7)
	{
		CopyString(temp, m_mp7);
		{
			strcat(list, temp); //LPCTSTR(m_mp7));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp8)
	{
		CopyString(temp, m_mp8);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp8));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp9)
	{
		CopyString(temp, m_mp9);

		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp9));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp10)
	{
		CopyString(temp, m_mp10);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp10));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp11)
	{
		CopyString(temp, m_mp11);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp11));
			strcat(list, ", ");
		}
	}
	if (m_ignore_mp12)
	{
		CopyString(temp, m_mp12);
		if (strcmp(temp, "") != 0)
		{
			strcat(list, temp); //LPCTSTR(m_mp12));
			strcat(list, ", ");
		}
	}

	for (i = 0; i < 12; i++)
	{
		if (m_ignore[i])
		{
			CopyString(temp, m_mp[i]);
			if (strcmp(temp, "") != 0)
			{
				strcat(list, temp); //LPCTSTR(m_mp[i]));
				strcat(list, ", ");
			}
		}
	}

	for (i = 0; i < 12; i++)
	{
		if (m_ignore2[i])
		{
			CopyString(temp, m_mpp2[i]); //mp5);
			if (strcmp(temp, "") != 0)
			{
				strcat(list, temp); //LPCTSTR(m_mpp2[i]));
				strcat(list, ", ");
			}
		}
	}
	
	for (i = strlen(list); i > 0; i--)
		if (list[i] == ',')
		{
			list[i] = NULL;
			i = 0;
		}

	if (strlen(list) > 1)
		strcpy(list,list+1);
	else
		list[0] = NULL;

	strcpy(extlist," ");

	for (i = 0; i < IGNORE_EXT_MAX; i++)
	{
		if (strlen(ignore_ext_ptr[i]) > 0)
		{
			CopyString(temp, ignore_ext_ptr[i]);
			strcat(extlist, temp); //LPCTSTR(ignore_ext_ptr[i]));
			if (i < IGNORE_EXT_MAX-1 && strlen(ignore_ext_ptr[i+1]) > 0)
				strcat(extlist,", ");
		}
	}

	if (strlen(extlist) > 1)
		strcpy(extlist,extlist+1);
	else
		extlist[0] = NULL;

	i = strlen(list);
	if (list[i - 2] == ',')
		list[i - 2] = NULL;


}

void MountpointDlg::ParseMountpointList(char list[], char extlist[])
{
	char *ptr, *cptr;
	char copy[1024];
	long i;
	long j = 0;

	m_ignore_mp1 = m_ignore_mp2 = m_ignore_mp3 = m_ignore_mp4 = FALSE;
	m_ignore_mp5 = m_ignore_mp6 = m_ignore_mp7 = m_ignore_mp8 = FALSE;
	m_ignore_mp9 = m_ignore_mp10 = m_ignore_mp11 = m_ignore_mp12 = FALSE;
	m_mp1 = m_mp2 = m_mp3 = m_mp4 = "";
	m_mp5 = m_mp6 = m_mp7 = m_mp8 = "";
	m_mp9 = m_mp10 = m_mp11 = m_mp12 = "";

	strcpy(copy,list);
	cptr = copy;

	ptr = tokenizer(cptr,",");


	for (i = 0; ptr != NULL && i < 12; i++)
	{
		m_ignore[i] = FALSE;
		m_mp[i] = "";
		m_ignore2[i] = FALSE;
		m_mpp2[i] = "";
		m_ignore2[i+12] = FALSE;
		m_mpp2[i+12] = "";
	}

	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp1 = TRUE;
		m_mp1 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp2 = TRUE;
		m_mp2 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp3 = TRUE;
		m_mp3 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp4 = TRUE;
		m_mp4 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp5 = TRUE;
		m_mp5 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp6 = TRUE;
		m_mp6 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp7 = TRUE;
		m_mp7 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp8 = TRUE;
		m_mp8 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp9 = TRUE;
		m_mp9 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp10 = TRUE;
		m_mp10 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}

	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp11 = TRUE;
		m_mp11 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	if (ptr != NULL && strlen(ptr) < 16)
	{
		m_ignore_mp12 = TRUE;
		m_mp12 = ptr;
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}
	else if (ptr != NULL && j < 24)
	{
		m_ignore2[j] = TRUE;
		m_mpp2[j++] = ptr;
		ptr = tokenizer(cptr,",");
	}
	else
	{
		ptr = tokenizer(cptr,",");
	}
	for (i = 0; ptr != NULL && i < 12; i++)
	{
		if (strlen(ptr) < 16)
		{
			m_ignore[i] = TRUE;
			m_mp[i] = ptr;
		}
		else if (ptr != NULL && j < 24)
		{
			m_ignore2[j] = TRUE;
			m_mpp2[j++] = ptr;
		}
		ptr = tokenizer(cptr,",");
		//tokenize(NULL,", ",ptr);
	}


	strcpy(copy,extlist);
	cptr = copy;

	ptr = tokenizer(cptr,",");

	for (i = 0; ptr != NULL && i < IGNORE_EXT_MAX; i++)
	{
		ignore_ext_ptr[i][0] = NULL;
	}

	i = 0;
	while (i < IGNORE_EXT_MAX && ptr != NULL)
	{
		if (ptr != NULL && strlen(ptr) < 5)
		{
			strcpy(ignore_ext_ptr[i],ptr);
		}
		i++;
		ptr = tokenizer(cptr,",");
	}


}



