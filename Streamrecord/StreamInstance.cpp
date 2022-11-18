#include "stdafx.h"
#include "StreamInstance.h"

#include <io.h>
#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>
#include <fcntl.h>
#include <afxmt.h>
#include <process.h>

#include <winsock2.h>
#include <ws2tcpip.h> 
#include <signal.h>
#include <time.h>

#include <iostream>
#include <string>
using namespace std;

#include "streamrecordDlg.h"
#include "loadpref.h"
#include "ircbotmfc.h"

#define FFMPEG_APP		"ffmpeg.exe"
#define RETRY_LIMIT		2
#define FLIPZU_WAIT		15
#define RETRY_TIMEOUT	30
#define ALLOWED_STREAMS	3
#define MAXDATASIZE		1024

extern char working_dir[1024];
extern wchar_t minimize_icon[32];
extern char * tokenizer(char* &ptr, const char *delimiter);

static CMutex ffmpeg_mutex, record_mutex; 

static DWORD dwAccessType = PRE_CONFIG_INTERNET_ACCESS;
static DWORD dwHttpRequestFlags =
	INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT;

const TCHAR szHeaders[] =
	_T("Accept: *.*, */*\r\nUser-Agent: Six Stream Recorder (compatible; MSIE 6.0; Windows NT 5.1)\r\n");

const TCHAR szShoutcastHeaders[] =
	_T("Accept: Icy-Metadata, */*\r\nUser-Agent: Simple Stream Recorder\r\n");

bool MatchesExtension(const char stream_url[], const char ignore_ext[])
{
	char *extptr = (char *)stream_url + strlen(stream_url) - strlen(ignore_ext);

	if (_strcmpi(extptr,ignore_ext) == 0)
		return true;
	return false;
}

bool MatchesWildcard(const char stream_url[], const char ignore_mp[])
{
	char ign_copy[1024], *cptr, *substr, *sptr;
	bool match = true;
	// If there's no wildcard in the ignore mountpoint, there can't 
	// be a match, so just return false
	if (strchr(ignore_mp,'*') == NULL)
		return false;
	// Assume * or *.* matches everything, and return true
	if (strcmp(ignore_mp,"*") == 0 || strcmp(ignore_mp,"*.*") == 0
		|| strcmp(ignore_mp+1,"*") == 0 || strcmp(ignore_mp+1,"*.*") == 0)
		return true;
	// If not, we've got some work to do
	strncpy(ign_copy,ignore_mp,1024);
	cptr = ign_copy;
	sptr = (char *)stream_url;
	// Begin by separating the ignore mountpoint string into tokens
	while (match && cptr[0] != 0)
	{
		substr = tokenizer(cptr,"*");
		if (substr != NULL)
		{			
			while (sptr[0] != NULL && strncmp(sptr,substr,strlen(substr)) != 0)
				sptr++;
			match = (bool)(strncmp(sptr,substr,strlen(substr)) == 0);
			if (match)
				sptr += strlen(substr);
		}
		else if (cptr != NULL)
		{
			match = true;
		}
	}
	return match;
}


StreamInstance::StreamInstance(STREAMRECORD_PREFERENCES *ppref):is_done(false),terminate(false),status_message("IDLE")
{
	pref = ppref;
	ircptr = NULL;
}
 
void StreamInstance::CopyParams(const char stream_URL[], 
								const char output_file[],
								bool shoutcast,
								bool reencode, bool delete_old, 
								short encodebr, bool infinite_retry,
								const char file_ext[],
								bool monitor_mp,
								long index, 
								bool play_sound_file,
								const char sound_file[],
								LPVOID player,
								bool es,
								IrcBotMFC *irc,
								bool update_topic)
{
	size_t found;
	string str = stream_URL;

	strcpy(url,stream_URL);
	strcpy(file,output_file);

	if (ext != NULL)
		strncpy(ext,file_ext,31);
	else
		strcpy(ext,"mp3");
	scast = shoutcast;
	encode = reencode;
	del_old = delete_old;
	bitrate = encodebr;
	inf_retry = infinite_retry;
	monitor_mountpoint = monitor_mp;
	if (monitor_mountpoint)
		inf_retry = false;

	flipzu = false;
	found = str.find("flipzu");
	if (found != string::npos)
		flipzu = true;
	found = str.find("ogg");
	if (found)
		inf_retry = false;
	if (found != string::npos)
		flipzu = true;
	stream_idx = index;

	pl_sound_file = play_sound_file;
	if (sound_file != NULL)
		strncpy(start_sound_file,sound_file,255);
	///the_player = player;
	enable_sounds = es;
	ircptr = irc;
	set_topic = update_topic;
}

short StreamInstance::ParseServerStatus(STREAMRECORD_PREFERENCES* pref,
	IGNORE_LIST* ignore, SCHEDULE_ADD_LIST* add,
	const char url[], char mp_list[][128],
	long server_idx)
{
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	CString strNewLocation;
	long bufsize = 4096;
	char* sz = new char[bufsize];
	char temp[1024], nurl[1024], * tmp;
	long num, j, k, m;
	long i = 0;
	long idx = 0;
	typedef char* cptr;
	char** mp;
	const long maxstr = 25;
	short n = 0;
	short total_streams = 0;
	CString string_url;

	mp = new cptr[maxstr];

	strcpy(nurl, url);
	if (nurl[strlen(nurl) - 1] != '/')
		strcat(nurl, "/");

	strcat(nurl, "status.xsl");

	CRecordSession session(LPCTSTR(PROGRAM_NAME), dwAccessType);
	//session = new CRecordSession(PROGRAM_NAME,dwAccessType);
	CopyString(string_url, nurl);

	if (!AfxParseURL(string_url, dwServiceType, strServerName, strObject, nPort)
		|| dwServiceType != INTERNET_SERVICE_HTTP)
	{
		strcpy(temp, "http://");
		strcat(temp, nurl);
		CopyString(string_url, temp);
		if (!AfxParseURL(string_url, dwServiceType, strServerName, strObject, nPort)
			|| dwServiceType != INTERNET_SERVICE_HTTP)
		{
			MessageBoxA(NULL, LPCSTR("Cannot parse URL"), PROGRAM_NAME, MB_OK | MB_ICONEXCLAMATION);
			return -1;
		}
	}

	pServer = session.GetHttpConnection(strServerName, nPort);

	TRY
	{
		if (Connect(pServer,pFile,session,dwHttpRequestFlags,
			dwServiceType,szHeaders,strObject,strServerName,nPort))
		{
			i = 0;
			do
			{
				num = pFile->Read(sz + i,CHUNKSIZE);
				i += num;
				if ((i + CHUNKSIZE) > bufsize)
				{
					bufsize *= 2;
					tmp = new char[bufsize];
					memcpy(tmp,sz,i);
					delete[] sz;
					sz = tmp;
				}
			} while (num >= CHUNKSIZE);
		}
		else
		{
			pServer = NULL;
		}

		pFile->Flush();
		pFile->Close();
		delete pFile;
		if (pServer != NULL)
		{
			pServer->Close();
			delete pServer;
		}



	}
		CATCH(CInternetException, pEx)
	{

	}
	END_CATCH

		bool done = false;

	if (i > 0)
	{
		j = 0;
		while (j < i)
		{
			done = false;
			while (strncmp(sz + j, "Mount Point", 11) != 0 && j < i && j < 65536)
				j++;
			j += 11;
			while (!done) //j < i)
			{
				if (j < i && (sz[j] == '/')) // && (sz[j-1] == ',' || sz[j-1] == '\r' || sz[j-1] == '\n')) ///	&& strncmp(sz+j,"/pre>",5) != 0 && idx < maxstr)
				{
					k = j + 1;
					while (sz[k] != '<' && (k - j) < 128)
						k++;
					if (!isspace(sz[j]))
					{
						mp[idx] = new char[128];
						strncpy(mp[idx], sz + j, k - j);
						mp[idx++][k - j] = NULL;
						done = true;
					}

					j = k;
				}
				else
				{
					j++;
					if (j >= i)
						done = true;
				}
			}

		}
	}


	// Now that we have a list of mountpoints, compare them to
	// the mountpoints already being recorded in the scheduler
	// to see if any of the mountpoints are new

	if (idx > 0)
	{
		strcpy(nurl, url);
		if (nurl[strlen(nurl) - 1] == '/')
			nurl[strlen(nurl) - 1] = NULL;

		for (i = 0; i < idx; i++)
		{
			strcpy(temp, nurl);
			strcat(temp, mp[i]);
			k = -1;
			for (j = 0; j < pref->num_entries; j++)
				if (pref->schedule_entry[j].monitor_mountpoint &&
					strcmp(pref->schedule_entry[j].stream_URL, temp) == 0)
				{
					total_streams++;
					break;
				}

			if (j == pref->num_entries)
				for (k = 0; k < ignore->num_entries; k++)
					if (strcmp(ignore->ignore_entry[k].mountpoint_URL, temp) == 0
						|| MatchesWildcard(temp, ignore->ignore_entry[k].mountpoint_URL))
						break;

			

			if (k == ignore->num_entries)
				for (m = 0; m < IGNORE_MP_MAX; m++)
				{
					//if ((((strcmp(pref->schedule_entry[server_idx].ignore_mp[m], mp[i]) == 0))))
					//	break;
					
					if ((1 //pref->schedule_entry[server_idx].enable_mp_ignore[m]
						&& ((strcmp(pref->schedule_entry[server_idx].ignore_mp[m], mp[i]) == 0
							|| MatchesWildcard(mp[i], pref->schedule_entry[server_idx].ignore_mp[m]))
							|| (strcmp((char*)(pref->schedule_entry[server_idx].ignore_mp[m]) + 1, mp[i]) == 0
								&& pref->schedule_entry[server_idx].ignore_mp[m][0] == '/')))
						|| (add->schedule_entry[server_idx].enable_mp_ignore[m]
							&& ((strcmp(add->schedule_entry[server_idx].ignore_mp[m], mp[i]) == 0
								|| MatchesWildcard(mp[i], add->schedule_entry[server_idx].ignore_mp[m]))
								|| (strcmp((char*)(add->schedule_entry[server_idx].ignore_mp[m]) + 1, mp[i]) == 0
									&& add->schedule_entry[server_idx].ignore_mp[m][0] == '/')))
						|| (add->schedule_entry[server_idx].enable_mp_ignore2[m]
							&& ((strcmp(add->schedule_entry[server_idx].ignore_mp2[m], mp[i]) == 0
								|| MatchesWildcard(mp[i], add->schedule_entry[server_idx].ignore_mp2[m]))
								|| (strcmp((char*)(add->schedule_entry[server_idx].ignore_mp2[m]) + 1, mp[i]) == 0
									&& add->schedule_entry[server_idx].ignore_mp2[m][0] == '/')))
						|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp2[m + 6]), mp[i]) == 0
							|| MatchesWildcard(mp[i], add->schedule_entry[server_idx].ignore_mp2[m + 6]))
							|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp2[m + 6]) + 1, mp[i]) == 0)
								&& add->schedule_entry[server_idx].ignore_mp2[m + 6][0] == '/'))
						|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m]), mp[i]) == 0
							|| MatchesWildcard(mp[i], add->schedule_entry[server_idx].ignore_mp3[m]))
							|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m]) + 1, mp[i]) == 0)
								&& add->schedule_entry[server_idx].ignore_mp3[m][0] == '/'))
						|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m + 6]), mp[i]) == 0
							|| MatchesWildcard(mp[i], add->schedule_entry[server_idx].ignore_mp3[m + 6]))
							|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m + 6]) + 1, mp[i]) == 0)
								&& add->schedule_entry[server_idx].ignore_mp3[m + 6][0] == '/'))
						|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m + 12]), mp[i]) == 0
							|| MatchesWildcard(mp[i], add->schedule_entry[server_idx].ignore_mp3[m + 12]))
							|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m + 12]) + 1, mp[i]) == 0)
								&& add->schedule_entry[server_idx].ignore_mp3[m + 12][0] == '/'))
						|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m + 18]), mp[i]) == 0
							|| MatchesWildcard(mp[i], add->schedule_entry[server_idx].ignore_mp3[m + 18]))
							|| ((strcmp((char*)(add->schedule_entry[server_idx].ignore_mp3[m + 18]) + 1, mp[i]) == 0)
								&& add->schedule_entry[server_idx].ignore_mp3[m + 18][0] == '/')
							|| (strlen(add->schedule_entry[server_idx].ignore_ext[m]) > 0 &&
								MatchesExtension(mp[i], add->schedule_entry[server_idx].ignore_ext[m]))))
						break;
					
				}

			if (j == pref->num_entries && k == ignore->num_entries
				&& m == IGNORE_MP_MAX && total_streams < ALLOWED_STREAMS)
			{
				total_streams++;
				strcpy(mp_list[n++], temp);
			}
			delete[] mp[i];
		}
	}

	delete[] mp;
	delete[] sz;

	return n;
}



bool StreamInstance::RecordStream() 
{
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	char temp[CHUNKSIZE+1];
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	CString strNewLocation;
	FILE *fp, *gp;
	long num = 0;
	long retry_count = 0;
	bool fail = false;
	char ofile[1024], logfile[1024], fn[1024];
	char str[256];
	long idx = 0;
	char tmp[32];
	long int scast_size = 0;
	bool toggle_data = true;
	long mp3_idx;
	unsigned long read_count = 0, rc = 0;
	long n, csize;
	long buf_size = 0;
	char *token;
	long retry = 0;
	unsigned int hour, min, month, date, year;
	unsigned int hr = 100, mn = 100, sc = 100;
	time_t osBinaryTime, theTime;
	CTime *cur_time = NULL, *the_time = NULL;
	char *sz = new char[4096];
	char *buf = new char[8192];
	char *file_ext="";
	char *ptr;
	char mod_ext[32];
	bool parsed = false;
	bool connected = false;
	long lc = 0;
	long sec, total_sec = 0;
	char *lastloc;
	CRecordSession *session;
	bool tout_exceeded = false;
	unsigned short timeout;
	char program[256];
	char surl[256];
	CString string_url;
	char tstr[1024], msg[1024];
	int writecount = 0;

	//CRecordSession session(PROGRAM_NAME,dwAccessType);

	if (stream_idx >= 0)
	{
		strcpy(program,pref->schedule_entry[stream_idx].program);
		strcpy(surl,pref->schedule_entry[stream_idx].stream_URL);
	}
	else
	{
		strcpy(program,"Unscheduled recording");
		strcpy(surl,"Unknown URL");
	}

	if (stream_idx < 0 || pref->schedule_entry[stream_idx].timeout <= 0)
		timeout = RETRY_TIMEOUT;
	else
		timeout = pref->schedule_entry[stream_idx].timeout;

	session = new CRecordSession(LPCTSTR(PROGRAM_NAME),dwAccessType);
	
	time(&osBinaryTime);

	cur_time = new CTime(osBinaryTime);
	is_done = false;
	if (strcmp(url,"") == 0)
	{
		MessageBoxA(NULL, LPCSTR("WARNING: Blank URL."), PROGRAM_NAME, MB_OK | MB_ICONEXCLAMATION);
		pref->schedule_entry[stream_idx].stream_idx = -2;
		return FALSE;
	}
	CopyString(string_url, url);

	if (!AfxParseURL(string_url, dwServiceType, strServerName, strObject, nPort)
 		|| dwServiceType != INTERNET_SERVICE_HTTP)
	{
		strcpy(temp,"http://");
		strcat(temp,url); 
		CopyString(string_url, temp);
		if (!AfxParseURL(string_url, dwServiceType, strServerName, strObject, nPort)
			|| dwServiceType != INTERNET_SERVICE_HTTP)
		{
			MessageBoxA(NULL,LPCSTR("Cannot parse URL"),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
			return FALSE;
		}	
	}

	pServer = session->GetHttpConnection(strServerName, nPort);

	if (!monitor_mountpoint)
		status_message = "CONNECTING";

	strcpy(logfile,working_dir);
	if (logfile[strlen(logfile)-1] != '\\')
		strcat(logfile,"\\");
	strcat(logfile,LOG_FILE);
	gp = fopen(logfile,"a");


	while (!connected && !terminate)
	{
		TRY
		{	
			if (!Connect(pServer,pFile,*session,dwHttpRequestFlags,
				dwServiceType,szHeaders,strObject,strServerName,nPort) 
				&& !terminate)
			{
				if (monitor_mountpoint)
				{
					memset(pref->schedule_entry[stream_idx].oldbuf,0,BUFFERSIZE);
					is_done = true;
					if (gp != NULL)
						fclose(gp);
					pFile->Flush();
					pFile->Close();
					delete pFile;
					pServer->Close();
					delete pServer;
					pServer = NULL;
					delete session;
					if (sz != NULL)
						delete [] sz;
					if (buf != NULL)
						delete [] buf;
					if (cur_time != NULL)
						delete cur_time;
					return FALSE;
				}
				Sleep(1000);
			}
			else
			{
				connected = true;
			}
		}
		CATCH (CInternetException, pEx)
		{

		}

		END_CATCH
	}

	if (monitor_mountpoint)
	{		
		num = pFile->Read(sz,CHUNKSIZE);
		if (num == 0 || (isascii(sz[0]) && isascii(sz[1]) && isascii(sz[2])
			&& sz[0] > 0 && sz[1] > 0 && sz[2] > 0))
		{
			num = pFile->Read(sz,CHUNKSIZE);
		}
		if (num > 0 && flipzu)
		{
			time(&theTime);
			the_time = new CTime(theTime);

			min = the_time->GetMinute();
			sec = the_time->GetSecond();
			do
			{
				num = pFile->Read(sz,CHUNKSIZE);
				delete the_time;
				time(&theTime);
				the_time = new CTime(theTime);
				if (min == cur_time->GetMinute())
					total_sec = the_time->GetSecond()-sec;
				else
					total_sec = 60-sec+the_time->GetSecond();
			} while (num > 0 && total_sec < FLIPZU_WAIT);
			delete the_time;
		}
		//if (strncmp(sz+512,pref->schedule_entry[stream_idx].oldbuf,BUFFERSIZE) == 0)
		//	num = 0;
		if (num == 0)
		{
			is_done = true;
			if (gp != NULL)
				fclose(gp);
			pFile->Flush();
			pFile->Close();
			delete pFile;
			pServer->Close();
			delete pServer;
			pServer = NULL;
			delete session;
			if (sz != NULL)
				delete [] sz;
			if (buf != NULL)
				delete [] buf;
			if (cur_time != NULL)
				delete cur_time;
			return FALSE;
		}
		else
		{
			//if (stream_idx >= 0)
			//	memcpy(pref->schedule_entry[stream_idx].oldbuf,sz+512,BUFFERSIZE);
			if (flipzu)
			{
				pFile->SeekToBegin();
			}
			else
			{
				pFile->Flush();
				pFile->Close();
				delete pFile;
				pFile = NULL;
				if (!Connect(pServer,pFile,*session,dwHttpRequestFlags,
					dwServiceType,szHeaders,strObject,strServerName,nPort))
					return FALSE;
			}
		}
	}

	status_message = "RECORDING";
	if (pref->schedule_entry[stream_idx].status != 1)
	{
		pref->schedule_entry[stream_idx].status = 1;
		SetStatus(*pref, stream_idx);
		if (pref->pushover)
		{
			sprintf(msg, "RECORDING - %s", pref->schedule_entry[stream_idx].program);
			PushMessage(pref, msg);
		}
	}

	time(&osBinaryTime);
	if (cur_time != NULL)
		delete cur_time;
	cur_time = new CTime(osBinaryTime);

	hour = cur_time->GetHour();
	min = cur_time->GetMinute();
	month = cur_time->GetMonth();
	date = cur_time->GetDay();
	year = cur_time->GetYear();

	sprintf(str,"Connected to %s at %2d:%2d on %d-%d-%d\n",
		url,hour,min,month,date,year);

	if (gp != NULL)
	{
		fprintf(gp,"%s",str);
		ConvertString(tstr, (wchar_t *)szHeaders);
		fprintf(gp, "%s\n", tstr); //szHeaders);
		fclose(gp);
		gp = NULL;
	}

	strcpy(ofile,file);
	n = 0;
	do
	{
		if ((fp = fopen(ofile,"r")) != NULL)
		{
			fclose(fp);
			idx = 0;
			while (ofile[idx] != '.')
				idx++;
			ofile[idx] = NULL;
			sprintf(tmp,"%2d",n++);
			idx = 0;
			while (tmp[idx] != NULL)
			{
				if (tmp[idx] == ' ')
					tmp[idx] = '0';
				idx++;
			}
			if (ofile[strlen(ofile)-3] == '_')
				ofile[strlen(ofile)-3] = NULL;
			sprintf(ofile,"%s_%s.%s",ofile,tmp,ext);
			idx = 0; 			
		}
	} while (fp != NULL);

	csize = CHUNKSIZE;


		while ((fp = fopen(ofile,"wb")) == NULL);


	if (fp != NULL)
	{
		status_message = "RECORDING";
		if (ircptr != NULL)
		{
			lastloc = (char *)strrchr((const char *)surl,'.');
			CopyString(tstr, status_message);
			if (lastloc == NULL || (surl+strlen(surl)-lastloc) > 4)
				sprintf(str, "%s - %s ( %s.m3u )", tstr, program, surl); //LPCTSTR(status_message), program, surl);
			else	
				sprintf(str, "%s - %s ( %s )", tstr, program, surl); //LPCTSTR(status_message), program, surl);
			ircptr->SendMessage(str);
			//sprintf(str,"URL: %s",surl);
			//ircptr->SendMessage(str);
			//sprintf(str,"%s - %s ( %s )",LPCTSTR(status_message),program,surl);
			if (set_topic)
				ircptr->SetTopic(str);
		}
		if (!enable_sounds)
		{
			/*
			if (pl_sound_file && the_player != NULL 
				&& the_player->OpenFile(start_sound_file,sfAutodetect))
				the_player->Play();
			else
			*/
			Beep(523,250);
		}
		ConvertString(minimize_icon, "red.ico");
		//minimize_icon = "red.ico";
		do
		{			
			TRY
			{
				if (fail && !terminate)
				{
					time(&osBinaryTime);
					if (cur_time != NULL)
						delete cur_time;
					cur_time = new CTime(osBinaryTime);
					hour = cur_time->GetHour();
					min = cur_time->GetMinute();
					sprintf(str,"Connection to %s failed at %2d:%2d...attempting reconnect\n",
						url,hour,min);
					gp = fopen(logfile,"a");
					if (gp != NULL)
					{
						fprintf(gp,"%s",str);
						fclose(gp);
					}
					if (fp != NULL)
					{
						status_message = "RECONNECTING";
						ConvertString(minimize_icon, "yellow.ico");
						pref->schedule_entry[stream_idx].status = 2;
						if (pref->schedule_entry[stream_idx].status != 2)
						{
							//writecount = 0;
							pref->schedule_entry[stream_idx].status = 2;
							SetStatus(*pref, stream_idx);
							if (pref->pushover)
							{
								sprintf(msg, "RECONNECTING - %s", pref->schedule_entry[stream_idx].program);
								PushMessage(pref, msg);
							}
						}
						//minimize_icon = "yellow.ico";
						Sleep(1000);
						if (inf_retry && retry >= RETRY_LIMIT)
						{
							Sleep(1000);
						}
						session = new CRecordSession(LPCTSTR(PROGRAM_NAME),dwAccessType);
						if (session != NULL)
						{
							pServer = session->GetHttpConnection(strServerName, nPort);
						}
						if (session != NULL && pServer != NULL)
						{
							while (!terminate && !Connect(pServer,pFile,*session,dwHttpRequestFlags,
								dwServiceType,szHeaders,strObject,strServerName,nPort))
							{
								Sleep(1000);
							}

							pref->schedule_entry[stream_idx].status = 1;
							SetStatus(*pref, stream_idx);
						
							hour = cur_time->GetHour();
							min = cur_time->GetMinute();
							sprintf(str,"Reconnected to %s at %d:%d\n",
								url,hour,min);
							gp = fopen(logfile,"a");
							if (gp != NULL)
							{
								fprintf(gp,"%s",str);
								fclose(gp);
							}
						}
					}

					if (the_time != NULL)
					{
						delete the_time;
						time(&theTime);
						the_time = new CTime(theTime);
						if (retry == 1)
						{
							hr = the_time->GetHour();
							mn = the_time->GetMinute();
							sc = the_time->GetSecond();
						}
						if (mn == the_time->GetMinute())
							total_sec = the_time->GetSecond()-sc;
						else if (hr == the_time->GetHour())
							total_sec = ((the_time->GetMinute()-mn)*60)-sc+the_time->GetSecond();
						else if (the_time->GetHour() > (signed)hr)
							total_sec = (the_time->GetHour()-hr)*3600-(mn-the_time->GetMinute())*60-sc+the_time->GetSecond();
						else
							total_sec = 3600+(the_time->GetMinute()-min)*60-sc+the_time->GetSecond();
						if (total_sec >= timeout && monitor_mountpoint)
							tout_exceeded = true;
					}
				}
				if (!terminate && pFile != NULL)
				{
					if (scast && scast_size > 0)
					{
						csize = scast_size;
						TRY
						{
							num = pFile->Read(sz,scast_size);
						} 
						CATCH (CInternetException, pEx)
						{
							status_message = "ERROR";
							num = 0;
						}
						END_CATCH
					}
					else
					{
						csize = CHUNKSIZE;
						TRY
						{
							num = pFile->Read(sz,CHUNKSIZE);
						}
						CATCH (CInternetException, pEx)
						{
							status_message = "ERROR";
							num = 0;
						}
						END_CATCH
					}
					//if (!parsed && stream_idx >= 0 && lc == 0)
					//	memcpy(pref->schedule_entry[stream_idx].oldbuf,sz,BUFFERSIZE);
					n = num;
				}
				else
				{
					num = 0;
					buf[0] = NULL;
				}
				if (num > 0 && !terminate) 
				{
					if (num == CHUNKSIZE)
						tout_exceeded = false;
					mp3_idx = 0;
					if (scast && scast_size >= 0
						&& toggle_data)
					{
						scast_size = ParseShoutcast(sz,num,mp3_idx);
						if (n == CHUNKSIZE)
						{
							strncpy(temp,sz,mp3_idx);
							temp[mp3_idx] = NULL;
							gp = fopen(logfile,"a");
							if (gp != NULL)
							{
								fprintf(gp,"%s\n",temp);
								fclose(gp);
							}
						}
						toggle_data = !toggle_data;
						num -= mp3_idx;
					}
					fail = false;
					retry_count = 0;
					status_message = "RECORDING";
					if (pref->schedule_entry[stream_idx].status != 1)
					{
						//writecount = 0;
						pref->schedule_entry[stream_idx].status = 1;
						SetStatus(*pref, stream_idx);
						if (pref->pushover)
						{
							sprintf(msg, "RECORDING - %s", pref->schedule_entry[stream_idx].program);
							PushMessage(pref, msg);
						}
					}

					ConvertString(minimize_icon, "red.ico");
					//minimize_icon = "red.ico";
					if (!scast)
						buf_size += n;
					if (read_count < 5 && isascii(sz[0]))
					{
						memcpy(buf+csize*read_count,sz,n);
						if (rc != read_count && buf_size < 4096)
						{
							buf_size += n;
							rc = read_count;
						}
					}				
					if (read_count >= 5 || (mp3_idx < n 
						&& (n == CHUNKSIZE || !isascii(sz[mp3_idx]))))
						fwrite(sz+mp3_idx,sizeof(char),num,fp);
					else
						read_count++;
				}
				else if (!terminate)
				{
					if (isascii(buf[0]) && !parsed && !monitor_mountpoint)
					{
						toggle_data = true;
						scast_size = 0;
						ParsePlaylist(buf,buf_size,url);

						if (!scast)
							file_ext = strrchr(url,'.');
						strcpy(mod_ext,ext);
						if (file_ext != NULL 
							&& !scast
							&& strchr(file_ext,'/') == NULL
							&& strchr(file_ext,':') == NULL)
						{
							ptr = file_ext;
							do
							{
								ptr--;
								if (*ptr == '.')
									file_ext = NULL;
							} while (ptr != url && *ptr != '/');
							if (file_ext != NULL)
								file_ext++;
							if (file_ext != NULL)
								strncpy(mod_ext,file_ext,31);
							else
								strcpy(mod_ext,"mp3");
						}
						
						if (strcmp(mod_ext,ext) != 0)
						{
							strcpy(sz,"del \"");
							strcat(sz,ofile);
							strcat(sz,"\"");

							if (fp != NULL)
								fclose(fp);

							if (system(sz))
							{
								status_message = "CANNOT DELETE FILE";
							}
	
							
							strcpy(ofile,file);
							ptr = ofile + strlen(ofile);
							while (*ptr != '.' && ptr != ofile)
								ptr--;
							if (ptr != ofile)
								*ptr = NULL;
							sprintf(ofile,"%s.%s",ofile,mod_ext);

							n = 0;
							do
							{
								if ((fp = fopen(ofile,"r")) != NULL)
								{
									fclose(fp);
									idx = 0;
									while (ofile[idx] != '.')
										idx++;
									ofile[idx] = NULL;
									sprintf(tmp,"%2d",n++);
									idx = 0;
									while (tmp[idx] != NULL)
									{
										if (tmp[idx] == ' ')
											tmp[idx] = '0';
										idx++;
									}
									if (ofile[strlen(ofile)-3] == '_')
										ofile[strlen(ofile)-3] = NULL;
									sprintf(ofile,"%s_%s.%s",ofile,tmp,mod_ext);
									idx = 0; 			
								}
							} while (fp != NULL);

							if (fp != NULL)
								fclose(fp);

							if ((fp = fopen(ofile,"wb")) == NULL)
								return FALSE;
						}

						parsed = true;
						CopyString(string_url, url);

						if (!AfxParseURL(string_url, dwServiceType, strServerName, strObject, nPort)
							|| dwServiceType != INTERNET_SERVICE_HTTP)
						{
							strcpy(temp,"http://");
							strcat(temp,url); //stream_URL);
							CopyString(string_url, temp);
							if (!AfxParseURL(string_url, dwServiceType, strServerName, strObject, nPort)
								|| dwServiceType != INTERNET_SERVICE_HTTP)
							{
								MessageBoxA(NULL,LPCSTR("Cannot parse URL"),PROGRAM_NAME,MB_OK|MB_ICONEXCLAMATION);
								return FALSE;
							}	
						}
						pServer = session->GetHttpConnection(strServerName, nPort);
					}					
					fail = true;
					pFile->Flush();
					pFile->Close();
					pFile = NULL;
					//delete pServer;
					//pServer = NULL;
					delete session;
					session = NULL;
					time(&theTime);
					the_time = new CTime(theTime);
					hour = the_time->GetHour();
					min = the_time->GetMinute();
					sec = the_time->GetSecond();
				}
				if (lc++ > 100)
					parsed = true;
			}
			CATCH (CInternetException, pEx)
			{
				fail = true;
				retry_count++;
				status_message = "ERROR";
				Sleep(2000);
				if (retry_count >= 1000)
				{
					status_message = "LOST CONNECTION";
					pref->schedule_entry[stream_idx].status = 4;
					SetStatus(*pref, stream_idx);
					if (pref->pushover)
					{
						sprintf(msg, "LOST CONNECTION - %s", pref->schedule_entry[stream_idx].program);
						PushMessage(pref, msg);
					}
				}
				if (pFile != NULL)
				{
					pFile->Flush();
					pFile->Close();
					delete pFile;
				}
				pFile = NULL;
				if (pServer != NULL)
					delete pServer;
				pServer = NULL;
				if (session != NULL)
					delete session;
				session = NULL;
				hour = cur_time->GetHour();
				min = cur_time->GetMinute();
				sprintf(str,"Connection to %s lost at %d:%d\n",
					url,hour,min);
				gp = fopen(logfile,"a");
				if (gp != NULL)
				{
					fprintf(gp,"%s",str);
					fclose(gp);
				}
			}

			END_CATCH

			
			
		} while (!terminate 
			&& (!fail || inf_retry || retry++ < RETRY_LIMIT || !tout_exceeded)); 
	}

	if (fp != NULL)
		fclose(fp);

	if (pFile != NULL)
	{
		pFile->Flush();
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}
	/*
	if (pServer != NULL)
	{
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	*/
	if (session != NULL)
		delete session;

	if (!inf_retry && retry > RETRY_LIMIT)
	{
		status_message = "CONNECTION FAILED";
		pref->schedule_entry[stream_idx].status = 4;
		SetStatus(*pref, stream_idx);
		if (pref->pushover)
		{
			sprintf(msg, "CONNECTION FAILED - %s", pref->schedule_entry[stream_idx].program);
			PushMessage(pref, msg);
		}
		
		is_done = true;
		if (sz != NULL)
			delete [] sz;
		if (buf != NULL)
			delete [] buf;
		if (cur_time != NULL)
			delete cur_time;
		if (the_time != NULL)
			delete the_time;
		if (pref->schedule_entry[stream_idx].monitor_mountpoint)
			pref->schedule_entry[stream_idx].fail_count = 0;
		if (monitor_mountpoint && ircptr != NULL)
		{
			sprintf(str,"FINISHED - %s",program);
			ircptr->SendMessage(str);
		}
		return TRUE;
	}

	if (encode)
	{
		status_message = "WAITING TO ENCODE";
		ffmpeg_mutex.Lock();	
	}

	if (encode) // && !fail)
	{
		char p1[128], p2[128], p3[128], p4[128], p5[128], appname[128];

		status_message = "ENCODING";
		if (ircptr != NULL)
		{
			sprintf(str,"ENCODING - %s",program);
			ircptr->SendMessage(str);
		}
		strcpy(sz,working_dir);
		if (sz[strlen(sz)-1] != '\\')
			strcat(sz,"\\");
		strcat(sz,FFMPEG_APP);

		strcpy(appname,sz);

		strcat(sz," -loglevel quiet -i \"");
		strcpy(p1,"-i");

		strcat(sz,ofile);
		strcat(sz,"\"");

		sprintf(p2,"\"%s\"",ofile);

		sprintf(tmp," -ab %dk \"",bitrate);

		sprintf(p3,"-ab");
		sprintf(p4,"%dk",bitrate);

		strcat(sz,tmp);
		strcat(sz,ofile);
		strcat(sz,".mp3\"");	

		sprintf(p5,"\"%s.mp3\"",ofile);

		gp = fopen(logfile,"a");
		if (gp != NULL)
		{
			fprintf(gp,"%s\n","Encoding file");
			fclose(gp);
		}
		
		//
		//if (spawnl(P_WAIT|P_DETACH,sz," ",NULL))
		if (system(sz))
		{
			status_message = "CANNOT START ENCODER";
			del_old = FALSE;
		}
		else
		{			
			status_message = "DONE ENCODING";
		}

		if (del_old)
		{
			strcpy(sz,"del \"");
			strcat(sz,ofile);
			strcat(sz,"\"");
			gp = fopen(logfile,"a");
			if (gp != NULL)
			{
				fprintf(gp,"%s\n","Deleting original file");
				fclose(gp);
			}
			if (system(sz))
			{
				status_message = "CANNOT DELETE ORIGINAL FILE";
			}
			else
			{
				status_message = "DELETING ORIGINAL FILE";
			}
			strcpy(sz,"ren \"");
			strcat(sz,ofile);
			strcat(sz,".mp3\" ");
			strcat(sz,"\"");

			token = strtok(ofile,"\\");
			fn[0] = NULL;
			if (token == NULL)
			{
				strcpy(fn,ofile);
			}
			else
			{
				while (token != NULL)
				{
					token = strtok(NULL,"\\");
					if (token != NULL)
						strcpy(fn,token);
				}
				
			}
			if (fn[0] != NULL)
			{
				gp = fopen(logfile,"a");
				if (gp != NULL)
				{
					fprintf(gp,"%s\n","Renaming encoded file");
					fclose(gp);
				}
				strcat(sz,fn);
				ptr = strrchr(fn,'.');
				if (_strcmpi(ptr,".mp3") == 0)
				{
					if (system(sz))
					{
						status_message = "CANNOT RENAME FILE";
					}
					else
					{
						status_message = "RENAMING FILE";
					}
				}
			}
			else
			{
				status_message = "CANNOT PARSE FILE NAME";
			}
		
		}
	}

	if (encode)
	{
		ffmpeg_mutex.Unlock();
	}
	if (pref->schedule_entry[stream_idx].status <= 2)
	{
		status_message = "DONE RECORDING";
		pref->schedule_entry[stream_idx].status = 5;
		SetStatus(*pref, stream_idx);
		if (pref->pushover)
		{
			sprintf(msg, "DONE RECORDING - %s", pref->schedule_entry[stream_idx].program);
			PushMessage(pref, msg);
		}
	}
	else if (pref->schedule_entry[stream_idx].status == 3)
	{
		status_message = "ABORTED";
		SetStatus(*pref, stream_idx);
	}

	if (ircptr != NULL)	
	{
		if (encode)
			sprintf(str,"DONE ENCODING - %s",program);
		else
			sprintf(str,"FINISHED - %s",program);
		ircptr->SendMessage(str);
	}

	if (pref->database && !pref->schedule_entry[stream_idx].repeated)
	{
		pref->schedule_entry[stream_idx].visible = FALSE;
		DeleteDatabase(pref->schedule_entry[stream_idx].schedule_id,*pref);
	}
	
	is_done = true;

	if (sz != NULL)
		delete [] sz;
	if (buf != NULL)
		delete [] buf;
	if (cur_time != NULL)
		delete cur_time;
	if (the_time != NULL)
		delete the_time;
	
	return TRUE;
}

bool StreamInstance::Connect(CHttpConnection *pServer, CHttpFile *&pFile,
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
		if (scast)
			pFile->AddRequestHeaders(szShoutcastHeaders);
		else
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
			if (scast)
				pFile->AddRequestHeaders(szShoutcastHeaders);
			else
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

bool StreamInstance::StreamConnect(const char server[], long nPort, int *s)
{
	/*
	struct addrinfo hints, *servinfo;
	int iResult;
	WSADATA wsaData;
	int res;
	//Recv some data
    int numbytes = 1;
    char buf[MAXDATASIZE];
	char cmd[MAXDATASIZE];
	int count = 0;
	char port[MAXDATASIZE];

	itoa(nPort,port,10);

	//Ensure that servinfo is clear
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
 
    //setup hints
    hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

	// Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
	{
        printf("WSAStartup failed: %d\n", iResult);
        return false;
    }
 
    //Setup the structs if error print why
    if ((res = getaddrinfo(server,port,&hints,&servinfo)) != 0)
    {
        setup = false;
        fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(res));
		return false;
    }

	//setup the socket
    if ((s = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol)) == -1)
    {
        perror("client: socket");
		return false;
    }
 
    //Connect
    if (connect(s,servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        close (s);
        perror("Client Connect");
		return false;
    }
 
    //We dont need this anymore
    freeaddrinfo(servinfo);
	*/
	return true;

}
/*
bool StreamInstance::StreamClose()
{
	close(s);
	return true;
}
*/
long StreamInstance::ParseShoutcast(const char buffer[], long size, 
									long& mp3_idx)
{
	long i, j;
	long retval = -1;
	const char METAINT_FIELD[] = "icy-metaint:";
	char metaint[128];

	mp3_idx = i = 0;
	while (i < size)
	{
		if (strncmp(buffer+i,METAINT_FIELD,
			strlen(METAINT_FIELD)) == 0)
		{
			i += strlen(METAINT_FIELD); 
			j = 0;
			while (isdigit(buffer[i+j]))
				j++;
			strncpy(metaint,buffer+i,j);
			retval = atol(metaint);
		}
		if (i+2 < size && ((!isascii(buffer[i]) 
			&& !isascii(buffer[i+1]) && !isascii(buffer[i+2])
			|| (buffer[i] < 0))))
			return retval;
		i++;
		mp3_idx = i;
	}

	return retval;
}

bool StreamInstance::ParsePlaylist(const char buffer[], long size, 
								   char new_URL[])
{
	long i, j;

	i = 0;
	while (i < (size-7))
	{
		if (strncmp(buffer+i,"http://",7) == 0)
		{
			j = 0;
			while ((i+j) < size && !isspace(buffer[i+j]))
				j++;
			strncpy(new_URL,buffer+i,j);
			new_URL[j] = NULL;
			return true;
		}
		i++;
	}
	return false;
}
