#include "stdafx.h"

#ifndef _LOADPREF_H
#define _LOADPREF_H

#define PREF_FILE	"streamrecord.prf"
#define LOG_FILE	"log.txt"
#define CHUNKSIZE	1023
#define	BUFFERSIZE	16
extern long MAX_SCHEDULE_ENTRIES;

#define IGNORE_MP_MAX	6
#define IGNORE_LIST_MIN	32
#define IGNORE_EXT_MAX	6
 

struct SCHEDULE
{
	DWORD start_hr, start_min;
	DWORD end_hr, end_min;
	unsigned char days;
	BOOL repeated;
	char program[256];
	char stream_URL[256];
	long stream_idx;
	CWinThread *thread_ptr;
	BOOL stream_running;
	BOOL shoutcast;
	unsigned char rec_date;
	BOOL reencode;
	BOOL delete_old;
	short encodebr;
	char visible;
	char monitor_mountpoint;
	char enable_ignore[3];
	unsigned char ignore_day[3];
	char ignore_start_hr[3];
	char ignore_start_min[3];
	char ignore_end_hr[3];
	char ignore_end_min[3];
	char monitor_server;
	char monitor_level;
	char limited_retry;
	char record_now;
	long id;
	char enable_mp_ignore[IGNORE_MP_MAX];
	char ignore_mp[IGNORE_MP_MAX][16];
	char oldbuf[BUFFERSIZE];
	char delete_this;
	unsigned short timeout;
	char recorded;
	char no_display;
	char fail_count;
	unsigned short schedule_id;
	char genre;
	char status;
	char password[15];
	char starttime[10];
	char endtime[10];
	
	
	char reserved[12];
	SCHEDULE()
	{
		status = 0;
		strcpy(reserved, "");
		strcpy(endtime, "");
		strcpy(starttime, "");
		strcpy(password, "");
		schedule_id = genre = status = 0;
		fail_count = no_display = recorded = delete_this = monitor_mountpoint = visible = (char)0;
		timeout = 30;
		encodebr = rec_date = stream_idx = days = start_hr = start_min = (unsigned char)0;
		delete_old = reencode = shoutcast = stream_running = repeated = recorded = FALSE;
		
		stream_idx = -1;
		record_now = 0;
		strcpy(program, "");
		strcpy(stream_URL, "");
		thread_ptr = NULL;
		strcpy(oldbuf, "");
		for (int i = 0; i < IGNORE_MP_MAX; i++)
		{
			strcpy(ignore_mp[i], "");
			enable_mp_ignore[i] = 0;
		}
		for (int i = 0; i < 3; i++)
		{
			enable_ignore[i] = ignore_start_hr[i] = ignore_start_min[i] = 0;
			ignore_end_hr[i] = ignore_end_min[i] = 0;
			ignore_day[i] = 0;
		}


	}
};

typedef struct
{
	char enable_mp_ignore[IGNORE_MP_MAX];
	char ignore_mp[IGNORE_MP_MAX][16];
	char enable_mp_ignore2[IGNORE_MP_MAX*2];
	char ignore_mp2[IGNORE_MP_MAX*2][16];
	char enable_mp_ignore3[IGNORE_MP_MAX*4];
	char ignore_mp3[IGNORE_MP_MAX*4][32];
	char ignore_ext[IGNORE_EXT_MAX][5];
	char reserved[2992];
	void SCHEDULE_ADD()
	{
		for (int i = 0; i < IGNORE_EXT_MAX; i++)
		{
			strcpy(ignore_mp[i], "");
			strcpy(ignore_mp2[i], "");
			strcpy(ignore_mp3[i], "");
			strcpy(ignore_ext[i], "");
		}
		strcpy(enable_mp_ignore, "");
		strcpy(enable_mp_ignore2, "");
		strcpy(enable_mp_ignore3, "");
	}
} SCHEDULE_ADD;

struct SCHEDULE_ADD_LIST
{
	long num_entries;
	SCHEDULE_ADD *schedule_entry;

	SCHEDULE_ADD_LIST(void)
	{
		num_entries = 0;
		schedule_entry = NULL;
	}
	SCHEDULE_ADD_LIST& operator =(SCHEDULE_ADD_LIST schedule)
	{
		long i;
		this->num_entries = schedule.num_entries;
		this->schedule_entry = new SCHEDULE_ADD[MAX_SCHEDULE_ENTRIES];
		for (i = 0; i < this->num_entries; i++)
		{
			strncpy((char *)this->schedule_entry[i].ignore_mp,
				(char *)schedule.schedule_entry[i].ignore_mp,IGNORE_MP_MAX*16);
			strncpy((char *)this->schedule_entry[i].ignore_mp2,
				(char *)schedule.schedule_entry[i].ignore_mp2,IGNORE_MP_MAX*2*16);
			strncpy((char *)this->schedule_entry[i].ignore_mp3,
				(char *)schedule.schedule_entry[i].ignore_mp3,IGNORE_MP_MAX*4*32);
			strncpy((char *)this->schedule_entry[i].ignore_ext,
				(char *)schedule.schedule_entry[i].ignore_ext,IGNORE_EXT_MAX*5);
		}
	}
};

typedef struct
{
	char mountpoint_URL[256];
	long entry_num;
	char reserved[252];
} IGNORE_MP;

struct IGNORE_LIST
{
	long num_entries;
	IGNORE_MP *ignore_entry;

	IGNORE_LIST(void)
	{
		num_entries = 0;
		ignore_entry = NULL;
	}
	IGNORE_LIST& operator =(IGNORE_LIST ignore)
	{
		long i;

		this->num_entries = ignore.num_entries;
		this->ignore_entry = new IGNORE_MP[MAX_SCHEDULE_ENTRIES];
		for (i = 0; i < this->num_entries; i++)
		{
			strcpy(this->ignore_entry[i].mountpoint_URL,
				ignore.ignore_entry[i].mountpoint_URL);
		}

	}
};

typedef struct 
{
	char server[32];
	char nickname[16];
	char channel[24];
	char UID[16];
	char enable_posting;
	char ringo_mode;
	char color, background;
	char update_topic;
} IRC_INFO;

struct STREAMRECORD_PREFERENCES
{
	char stream_URL[256];
	char output_filename[256];
	char path[1024];
	long num_entries;
	BOOL shoutcast;
	char play_file[256];
	char activation_code[17];
	char code_is_good;
	char play_sound_file;
	char sound_file[256];
	char no_subdirs;
	char enable_sounds;
	IRC_INFO irc;
	char database;
	char ip_address[16];
	char username[16];
	char password[16];
	char port[5];
	char schema[16];
	bool no_load;
	char DBpassword[15];
	char DBinterval;
	char datetime[25];
	char enable_dbox;
	char pushover;
	
	char reserved[255]; 

	SCHEDULE *schedule_entry;
	
	

	STREAMRECORD_PREFERENCES(void)
	{
		///completed = FALSE;
		no_load = false;
		pushover = 0;
		strcpy(stream_URL,"sample");
		strcpy(output_filename,"output.mp3");
		strcpy(path,"C:\\");
		strcpy(play_file,"test.mp3");
		strcpy(sound_file,"sound.mp3");
		strcpy(ip_address, "127.0.0.1");
		strcpy(username, "root");
		strcpy(password, "password");
		strcpy(schema, "ssr");
		strcpy(port, "3306");
		strcpy(password, "passw0rd");
		strcpy(datetime, "1-1-1980 00:00:00");
		num_entries = 0;
		strcpy(activation_code," ");
		code_is_good = 1;
		no_subdirs = 0;
		enable_sounds = 1;
		database = 0;
		DBinterval = 30;
		strcpy(this->irc.channel,"#sample");
		strcpy(this->irc.nickname,"test");
		strcpy(this->irc.server,"null");
		strcpy(this->irc.UID,"blank");
		this->irc.enable_posting = 0;
		this->irc.ringo_mode = 0;
		this->irc.color = 3;
		this->irc.background = 0;
		this->irc.update_topic = 0;
		memset(this->reserved,0,49);
		schedule_entry = NULL;
		enable_dbox = TRUE;
	}

	STREAMRECORD_PREFERENCES& operator =(STREAMRECORD_PREFERENCES pref)
	{
		long i;

		strcpy(this->stream_URL,pref.stream_URL);
		strcpy(this->output_filename,pref.output_filename);
		strcpy(this->path,pref.path);
		this->num_entries = pref.num_entries;
		this->shoutcast = pref.shoutcast;
		strcpy(this->play_file,pref.play_file);
		strcpy(this->password, pref.password);
		strcpy(this->activation_code,pref.activation_code);
		memcpy(this->reserved,pref.reserved,sizeof(char)*490);
		strcpy(this->irc.channel,pref.irc.channel);
		strcpy(this->irc.nickname,pref.irc.nickname);
		strcpy(this->irc.server,pref.irc.server);
		strcpy(this->irc.UID,pref.irc.UID);
		this->irc.enable_posting = pref.irc.enable_posting;
		this->irc.ringo_mode = pref.irc.ringo_mode;
		this->irc.background = pref.irc.background;
		this->irc.color = pref.irc.color;
		this->irc.update_topic = pref.irc.update_topic;
		this->schedule_entry = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
		for (i = 0; i < this->num_entries; i++)
		{
			this->schedule_entry[i] = pref.schedule_entry[i];
		}
		return *this;
	}

	~STREAMRECORD_PREFERENCES(void)
	{
		delete [] schedule_entry;
	}
};

int LoadPreferences(const char *pref_file, STREAMRECORD_PREFERENCES& pref, IGNORE_LIST& ignore, SCHEDULE_ADD_LIST& add);
int SavePreferences(const char *pref_file, const STREAMRECORD_PREFERENCES& pref, const IGNORE_LIST& ignore, const SCHEDULE_ADD_LIST& add, bool database=true);
char * GetWorkingDirectory();
void ConsolidateSchedule(STREAMRECORD_PREFERENCES *pref);
void ConsolidateIgnoreList(IGNORE_LIST *ignore);
void ConsolidateAddList(SCHEDULE_ADD_LIST *add);
static wchar_t* charToWChar(const char* text);
void CopyString(char dest[], CString source, unsigned short maxlen=1024);
void CopyString(CString& dest, char source[], unsigned short maxlen=1024);
void ConvertString(wchar_t conv_str[], char str[], unsigned short maxlen = 1024);
void ConvertString(char conv_str[], wchar_t str[], unsigned short maxlen = 1024);
void LoadDatabaseA(STREAMRECORD_PREFERENCES& pref);
void SaveDatabase(const STREAMRECORD_PREFERENCES& pref, bool thread=false, int n=-1);
void DeleteDatabase(const int id, const STREAMRECORD_PREFERENCES& pref);
void LoadDatabase(STREAMRECORD_PREFERENCES& pref);
void CopySchedule(STREAMRECORD_PREFERENCES& pref);
void ResetDatabase(const STREAMRECORD_PREFERENCES& pref);
void ResetStatus(const STREAMRECORD_PREFERENCES& pref);
void SetStatus(const STREAMRECORD_PREFERENCES& pref,int n);
void ResetConnection(const STREAMRECORD_PREFERENCES& pref);
bool ResetTable(const STREAMRECORD_PREFERENCES& pref);
bool SetStatus(const STREAMRECORD_PREFERENCES& pref);
void PushMessage(const STREAMRECORD_PREFERENCES *pref, char msg[]);

#endif