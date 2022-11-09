#include "stdafx.h"
#include "CRecordSession.h"



CRecordSession::CRecordSession(LPCTSTR pszAppName, int nMethod)
	: CInternetSession(pszAppName, 1, nMethod)
{
	bProgressMode = FALSE;
}


void CRecordSession::OnStatusCallback(DWORD dwContext, DWORD dwInternetStatus,
	LPVOID lpvStatusInfomration, DWORD dwStatusInformationLen)
{
	if (!bProgressMode)
		return;

	if (dwInternetStatus == INTERNET_STATUS_CONNECTED_TO_SERVER)
		cerr << _T("Connection made!") << endl;
}
