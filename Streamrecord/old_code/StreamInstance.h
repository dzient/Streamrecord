#include "stdafx.h"
#include "CRecordSession.h"
#include "loadpref.h"
#include ".\\include\\libzplay.h"
#include <afx.h>
#include <afxwin.h>
#include <afxinet.h>
#include "ircbotmfc.h"

#ifndef _RECORDSTREAM_H
#define _RECORDSTREAM_H

///using namespace libZPlay;

class CStreamrecordDlg;

#define PROGRAM_NAME "Six Stream Recorder v. 1.36"



class StreamInstance
{
	public:
		StreamInstance(STREAMRECORD_PREFERENCES *ppref=NULL);
		void CopyParams(const char stream_URL[], const char output_file[],
			bool shoutcast=false, bool reencode=false, bool delete_old=false, 
			short encodebr=96, bool infinite_retry=true, const char file_ext[]=NULL,
			bool monitor_mp=false, long index=-1, bool play_sound_file=false,
			const char sound_file[]=NULL, LPVOID player=NULL,
			bool es=true, IrcBotMFC *irc=NULL, bool update_topic=FALSE);
		short ParseServerStatus(STREAMRECORD_PREFERENCES *pref, 
			IGNORE_LIST *ignore, SCHEDULE_ADD_LIST *add, 
			const char url[], char mp_list[][128], long server_idx);
		bool RecordStream();
		void SetTerminate(bool val) { terminate = val; }
		bool GetTerminate() { return terminate; }
		bool Done() { return is_done; }
		CString GetStatusMessage() {
			return (CString)status_message; }
	protected:
		bool Connect(CHttpConnection* pServer, CHttpFile *&pFile, 
			CRecordSession& session, DWORD dwHttpRequestFlags, 
			DWORD dwServiceType, const TCHAR szHeaders[], 
			CString strObject, CString strServerName,
			INTERNET_PORT nPort);
		long ParseShoutcast(const char buffer[], long size, long& mp3_idx);
		bool ParsePlaylist(const char buffer[], long size, char new_URL[]);

		bool StreamConnect(const char server[], long nPort, int *s);
	private:
		bool terminate;
		bool is_done;
		CString status_message;
		char url[1024], file[1024];
		char ext[32];
		bool scast;
		bool encode, del_old, inf_retry;
		bool monitor_mountpoint;
		short bitrate;
		STREAMRECORD_PREFERENCES *pref;
		bool flipzu;
		long stream_idx;
		bool pl_sound_file;
		char start_sound_file[256];
		///ZPlay *the_player;
		bool enable_sounds;
		IrcBotMFC *ircptr;
		bool set_topic;

};

typedef StreamInstance* StreamPtr;
				 

#endif
