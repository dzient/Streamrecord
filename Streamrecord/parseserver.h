#include "stdafx.h"

#include <io.h>
#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>
#include <fcntl.h>
#include <afxmt.h>
#include <process.h>
#include "loadpref.h"
#include "CRecordSession.h"

#ifndef _PARSESERVER_H
#define _PARSESERVER_H

short ParseServerStatus(STREAMRECORD_PREFERENCES *pref, 
						IGNORE_LIST *ignore,
						const char url[], char mp_list[][128]);
bool Connect(CHttpConnection *pServer, CHttpFile *&pFile,
			CRecordSession& session, DWORD dwHttpRequestFlags, 
			DWORD dwServiceType, const TCHAR szHeaders[], 
			CString strObject, CString strServerName,
			INTERNET_PORT nPort);
void AddToSchedule(STREAMRECORD_PREFERENCES *pref,
				   SCHEDULE_ADD_LIST *add,
				   const char url[], BOOL monitor_mp,
				   BOOL limited_retry, long start_hr,
				   long start_min, long end_hr,
				   long end_min, unsigned short timeout);

#endif
