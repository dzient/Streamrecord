#include "stdafx.h"
//#include <iostream>
//using namespace std;
//#include <stdio.h>
#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>

#ifndef _CRECORDSESSION_H
#define _CRECORDSESSION_H

class CRecordSession : public CInternetSession
{
	public:
		CRecordSession(LPCTSTR pszAppName, int nMethod);
		void OnStatusCallback(DWORD dwContext, DWORD dwInternetStatus,
			LPVOID lpvStatusInfomration, DWORD dwStatusInformationLen);
		BOOL GetProgressMode() { return bProgressMode; }
		void SetProgressMode(BOOL val) { bProgressMode = val; }
	private:
		BOOL bProgressMode;

};

#endif