#include "stdafx.h"
#include "parseserver.h"
#include "loadpref.h"
#include "CRecordSession.h"
#include "StreamInstance.h"

#include <io.h>
#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>
#include <fcntl.h>
#include <afxmt.h>
#include <process.h>


static DWORD dwAccessType = PRE_CONFIG_INTERNET_ACCESS;
static DWORD dwHttpRequestFlags =
	INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT;

const TCHAR szHeaders[] =
	_T("Accept: *.*, */*\r\nUser-Agent: Simple Stream Recorder (compatible; MSIE 6.0; Windows NT 5.1)\r\n");


short ParseServerStatus(STREAMRECORD_PREFERENCES *pref, 
						IGNORE_LIST *ignore,
						const char url[], char mp_list[][128])
{
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	CString strNewLocation;
	long bufsize = 4096;
	char *sz = new char[bufsize];
	char temp[1024], nurl[1024], *tmp;
	long num, j, k;
	long i = 0;
	long idx = 0;
	typedef char* cptr;
	char **mp;
	const long maxstr = 25;
	short n = 0;
	CString c_nurl;

	mp = new cptr[maxstr];

	strcpy(nurl,url);
	if (nurl[strlen(nurl)-1] != '/')
		strcat(nurl,"/");

	strcat(nurl,"status2.xsl");

	CRecordSession session(LPCTSTR(PROGRAM_NAME),dwAccessType);

	if (!AfxParseURL(c_nurl, dwServiceType, strServerName, strObject, nPort)
 		|| dwServiceType != INTERNET_SERVICE_HTTP)
	{
		CopyString(nurl, c_nurl);
		strcpy(temp,"http://");
		strcat(temp,nurl); 
		if (!AfxParseURL(c_nurl, dwServiceType, strServerName, strObject, nPort)
			|| dwServiceType != INTERNET_SERVICE_HTTP)
		{
			MessageBoxA(NULL,"Cannot parse URL",PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
			return -1;
		}	
	}

	pServer = session.GetHttpConnection(strServerName, nPort);

	if (Connect(pServer,pFile,session,dwHttpRequestFlags,
		dwServiceType,szHeaders,strObject,strServerName,nPort))
	{
		i = 0;
		do
		{
			num = pFile->Read(sz,CHUNKSIZE);
			i += num;
			if ((i+CHUNKSIZE) > bufsize)
			{
				bufsize *= 2;
				tmp = new char[bufsize];
				memcpy(tmp,sz,i);
				delete [] sz;
				sz = tmp;
			}
		} while (num >= CHUNKSIZE);

		pFile->Flush();
		pFile->Close();
		delete pFile;
		pServer->Close();
		delete pServer;
	}

	if (i > 0)
	{
		j = 0;
		while (strncmp(sz+j,"<pre>",5) != 0 && j+4 < i)
			j++;
		j += 4;
		while (j < i)
		{			
			if (j < i && sz[j] == '/' && strncmp(sz+j,"/pre>",5) != 0 && idx < maxstr)
			{
				k = j+1;
				while (!isspace(sz[k]) && sz[k] != ',' && (k-j) < 128)
					k++;
				mp[idx] = new char[128];
				strncpy(mp[idx],sz+j,k-j);
				mp[idx++][k-j] = NULL;
				j = k;
			}
			else
			{
				j++;	
			}
		}
	}


	// Now that we have a list of mountpoints, compare them to
	// the mountpoints already being recorded in the scheduler
	// to see if any of the mountpoints are new

	if (idx > 0)
	{
		strcpy(nurl,url);
		if (nurl[strlen(nurl)-1] == '/')
			nurl[strlen(nurl)-1] = NULL;

		for (i = 0; i < idx; i++)
		{
			strcpy(temp,nurl);
			strcat(temp,mp[i]);
			for (j = 0; j < pref->num_entries; j++)
				if (pref->schedule_entry[j].monitor_mountpoint 
					&& strcmp(pref->schedule_entry[j].stream_URL,temp) == 0)
					break;
			if (j == pref->num_entries)
				for (j = 0; j < ignore->num_entries; j++)
					if (strcmp(ignore->ignore_entry[j].mountpoint_URL,temp) == 0)
						break;
			if (j == ignore->num_entries)
				strcpy(mp_list[n++],temp);		
			delete [] mp[i];
		}
	}

	delete [] mp;
	delete [] sz;

	return n;
}


bool Connect(CHttpConnection *pServer, CHttpFile *&pFile,
			CRecordSession& session, DWORD dwHttpRequestFlags, 
			DWORD dwServiceType, const TCHAR szHeaders[], 
			CString strObject, CString strServerName,
			INTERNET_PORT nPort)
{
	DWORD dwRet;
	CString strNewLocation;
	int nPlace;

	TRY
	{
		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			strObject, NULL, 1, NULL, NULL, dwHttpRequestFlags);
	
		pFile->AddRequestHeaders(szHeaders);
		pFile->SendRequest();
		pFile->QueryInfoStatusCode(dwRet);

		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation);

		if (dwRet == HTTP_STATUS_MOVED ||
			dwRet == HTTP_STATUS_REDIRECT ||
			dwRet == HTTP_STATUS_REDIRECT_METHOD)
		{
			pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation);

			nPlace = strNewLocation.Find(_T("Location: "));
			if (nPlace == -1)
			{
				cerr << _T("Error: Site redirects with no new location") << endl;
				return false;
			}

			strNewLocation = strNewLocation.Mid(nPlace + 10);
			nPlace = strNewLocation.Find('\n');
			if (nPlace > 0)
				strNewLocation = strNewLocation.Left(nPlace);

			// close up the redirected site

			pFile->Flush();
			pFile->Close();
			delete pFile;
			pServer->Close();
			delete pServer;

			if (session.GetProgressMode())
			{
				cerr << _T("Caution: redirected to ");
				cerr << (LPCTSTR) strNewLocation << endl;
			}

			// figure out what the old place was
			if (!AfxParseURL(strNewLocation, dwServiceType, strServerName, strObject, nPort))
			{
				cerr << _T("Error: the redirected URL could not be parsed.") << endl;
				return false;
			}

			if (dwServiceType != INTERNET_SERVICE_HTTP)
			{
				cerr << _T("Error: the redirected URL does not reference a HTTP resource.") << endl;
				return false;
			}

			// try again at the new location
			pServer = session.GetHttpConnection(strServerName, nPort);
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,
				strObject, NULL, 1, NULL, NULL, dwHttpRequestFlags);
			pFile->AddRequestHeaders(szHeaders);
			pFile->SendRequest();

			pFile->QueryInfoStatusCode(dwRet);
			if (dwRet != HTTP_STATUS_OK)
			{
				cerr << _T("Error: Got status code ") << dwRet << endl;
				return false;
			}

		}

	}
	CATCH (CInternetException, pEx)
	{
		return false;
	}
	END_CATCH
				

	cerr << _T("Status Code is ") << dwRet << endl;

	return true;
}

void AddToSchedule(STREAMRECORD_PREFERENCES *pref,
				   SCHEDULE_ADD_LIST *add,
				   const char url[], BOOL monitor_mp,
				   BOOL limited_retry, long start_hr,
				   long start_min, long end_hr,
				   long end_min, unsigned short timeout)
{
	time_t osBinaryTime;
	CTime *cur_time;
	long i = pref->num_entries;
	long j, k, m;
	unsigned int day;

	time(&osBinaryTime);
	cur_time = new CTime(osBinaryTime);
	day = cur_time->GetDayOfWeek();

	pref->schedule_entry[i].stream_idx = -1;
	strcpy(pref->schedule_entry[i].stream_URL,url);
	if (monitor_mp)
		pref->schedule_entry[i].days = 0;
	else
		pref->schedule_entry[i].days = (1<<(day-1));
	pref->schedule_entry[i].start_hr = start_hr;
	pref->schedule_entry[i].start_min = start_min;
	pref->schedule_entry[i].end_hr = end_hr;
	pref->schedule_entry[i].end_min = end_min;
	pref->schedule_entry[i].delete_old = FALSE;
	pref->schedule_entry[i].encodebr = 128;
	for (j = 0; j < 3; j++)
	{
		pref->schedule_entry[i].enable_ignore[j] = FALSE;
		pref->schedule_entry[i].ignore_day[j] = 0;
		pref->schedule_entry[i].ignore_start_hr[j] = 0;
		pref->schedule_entry[i].ignore_start_min[j] = 0;
		pref->schedule_entry[i].ignore_end_hr[j] = 0;
		pref->schedule_entry[i].ignore_end_min[j] = 0;
	}
	pref->schedule_entry[i].monitor_level = 0;
	pref->schedule_entry[i].monitor_mountpoint = monitor_mp;
	pref->schedule_entry[i].limited_retry = limited_retry;
	pref->schedule_entry[i].monitor_server = FALSE;
	strcpy(pref->schedule_entry[i].password, pref->DBpassword);
	j = strlen(url)-1;
	while (url[j] != '/' && j > 0)
		j--;
	strcpy(pref->schedule_entry[i].program,url+j+1);
	strcat(pref->schedule_entry[i].program," mountpoint");
	for (j = 0; j < (signed)strlen(pref->schedule_entry[i].program); j++)
		if (pref->schedule_entry[i].program[j] == '.'
			|| pref->schedule_entry[i].program[j] == '\\'
			|| pref->schedule_entry[i].program[j] == ',')
			pref->schedule_entry[i].program[j] = ' ';
	pref->schedule_entry[i].rec_date = 0;
	pref->schedule_entry[i].reencode = FALSE;
	pref->schedule_entry[i].repeated = FALSE;
	pref->schedule_entry[i].shoutcast = FALSE;
	pref->schedule_entry[i].stream_running = FALSE;
	pref->schedule_entry[i].thread_ptr = NULL;
	pref->schedule_entry[i].visible = 1;
	pref->schedule_entry[i].record_now = TRUE;
	pref->schedule_entry[i].delete_this = TRUE;
	pref->schedule_entry[i].timeout = timeout;
	pref->schedule_entry[i].recorded = FALSE;
	pref->schedule_entry[i].visible = FALSE;
	////pref->schedule_entry[i].schedule_id = 65535;
	for (k = 0, m = 0; k < (signed)strlen(pref->schedule_entry[i].program); k++)
		m += pref->schedule_entry[i].program[k];
	pref->schedule_entry[i].id = m%100001;
	for (j = 0; j < IGNORE_MP_MAX; j++)
	{
		pref->schedule_entry[i].ignore_mp[j][0] = NULL;
		pref->schedule_entry[i].enable_mp_ignore[j] = FALSE;
		add->schedule_entry[i].enable_mp_ignore[j] = FALSE;
		add->schedule_entry[i].enable_mp_ignore2[j] = FALSE;
		add->schedule_entry[i].enable_mp_ignore2[j+6] = FALSE;
		add->schedule_entry[i].ignore_mp[j][0] = NULL;
		add->schedule_entry[i].ignore_mp2[j][0] = NULL;
		add->schedule_entry[i].ignore_mp2[j+6][0] = NULL;
	}
	pref->num_entries = i+1;

	delete cur_time;
}