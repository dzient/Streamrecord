//-------------------------------------------
// David Zientara
// 12-2-2022
//
// StreamInstance.cpp
//
// File for the StreamInstance classs
//
//---------------------------------------------


#include "stdafx.h"
#include "loadpref.h"
#include "Database.h"
#include "Logger.h"
#include <afxmt.h>
#define SCHEMA	"ssr"
#define MS	65536
CMutex db_mutex;
extern CMutex schedule_mutex;
bool preferences_lock = false;
/// = NULL;  
extern StreamPtr stream_array[MS];

//-------------------------------------
// pref_struct
// This is a struct for preferences
//--------------------------------------
struct pref_struct
{
	STREAMRECORD_PREFERENCES* pref;
	STREAMRECORD_PREFERENCES* temp;
	IGNORE_LIST* ignore;
	SCHEDULE_ADD_LIST* add;
	char* url;
	short retval;
	sql::ResultSet* res;
} pref_params;

//------------------------------------
// CopyPref
// Function takes as input a pointer
// to a void, casts this to a 
// pref_struct object, and assigns
// parameters
// PARAMS: pParam (LPVOID)
// RETURNS: 1
//-------------------------------------

UINT CopyPref(LPVOID pParam)
{
	pref_struct* param_ptr = (pref_struct*)pParam;
	schedule_mutex.Lock();
	//preferences_lock = true;
	memcpy(param_ptr->temp, param_ptr->pref, sizeof(STREAMRECORD_PREFERENCES));
	memcpy(param_ptr->temp->schedule_entry, param_ptr->pref->schedule_entry, sizeof(SCHEDULE) * MAX_SCHEDULE_ENTRIES);
	schedule_mutex.Unlock();

	//preferences_lock = false;

	//param_ptr->stream_ptr->RecordStream(); //param_ptr->stream_URL,param_ptr->output_filename);
	return 1;
}
//-----------------------------------------------
// parse_string
// Function takes an array of chars,
// appends a backslash (\) to single 
// quotes (')
// PARAMS: str (array of chars), len (int)
// RETURNS: string representing the parsed
// string
//------------------------------------------------
string parse_string(char str[], int len)
{
	string retval = "";
	for (int i = 0; i < len; i++)
		if (strncmp((const char*)str + i, "'", 1) == 0)
		{
			retval += "\\'";
		}
		else
		{
			retval += str[i];
		}
		
	return retval;
}

//SCHEDULE* sched = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
//SCHEDULE* sched2 = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
//-----------------------------------------------
// Database
// Constructor for the Database class
// This initializes the API for MySQL and some 
// variables
// PARAMS: pref (STREAMRECORD_PREFERENCES object)
// RETURNS: Nothing
//-------------------------------------------------
Database::Database(const STREAMRECORD_PREFERENCES& pref):driver(NULL),con(NULL),stmt(NULL),res(NULL),pinit(false),prev(0)
{
	ppref = (STREAMRECORD_PREFERENCES*)&pref;
	//////temp = new STREAMRECORD_PREFERENCES;
	
	////temp->schedule_entry = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
	try
	{
		char address[128];
		temp = NULL;
		driver = get_driver_instance();
		sprintf(address, "tcp://%s:%s", pref.ip_address, pref.port);

		con = driver->connect(address, pref.username, pref.password);
		///////con = driver->connect("tcp://127.0.0.1:3306", "root", "password");
		/* Connect to the MySQL test database */
		con->setSchema(pref.schema);
		//con->setSchema(SCHEMA);
		stmt = con->createStatement();
		temp_ptr = NULL;
		max_id = 0;
	}
	catch (sql::SQLException& e)
	{
		LogError(e);
		ResetConnection(pref);	
	}
	temp_ptr = NULL;
	temp_streams = NULL;
	
	temp_recorded = temp_stream_running = NULL;

	

	
}
void Database::LogError(sql::SQLException& e)
{
	Logger errlog(LOG_FILE);
	string file = __FILE__;
	long line = __LINE__;
	char linestr[1024];
	sprintf_s(linestr, "%d", line);
	string error = "# ERR: SQLException in " + file + "\r\n";
	errlog.Log(error.c_str());
	string function = __FUNCTION__;
	error = "(" + function + ") on line " + linestr + "\r\n";
	errlog.Log(error.c_str());
	/* what() (derived from std::runtime_error) fetches error message */
	string msg = e.what();
	error = "# ERR: " + msg + "\r\n";;
	errlog.Log(error.c_str());
	msg = e.getErrorCode();
	error = " (MySQL error code: " + msg + "\r\n";
	errlog.Log(error.c_str());
	msg = e.getSQLState();
	error = ", SQLState: " + msg + " )" + "\r\n"; // << endl;
	errlog.Log(error.c_str());

}
bool Database::ResetTable(const STREAMRECORD_PREFERENCES& pref)
{
	string trunctable = "TRUNCATE TABLE "; // sixstder_ssr.schedule; "

	trunctable += pref.schema; 
	trunctable += ".schedule";

	string resettable = "";

	/*
	resettable += "CREATE TABLE sixstder_ssr.schedule(";
	resettable += "ScheduleID int NOT NULL AUTO_INCREMENT,";
	resettable += "ProgramName varchar(45),";
	resettable += "URL varchar(90),";
	resettable += "Day int,";
	resettable += "Daystr varchar(15),";
	resettable += "StartTimeHour int,";
	resettable += "StartTimeMin int,";
	resettable += "EndTimeHour int,";
	resettable += "EndTimeMin int,";
	resettable += "Repeating int,";
	resettable += "Shoutcast int,";
	resettable += "Genre int,";
	resettable += "Status varchar(15),";
	resettable += "Password varchar(15),";
	resettable += "Starttime varchar(10),";
	resettable += "Endtime varchar(10),";
	resettable += "PRIMARY KEY(ScheduleID)";
	resettable += "); \n";
	*/
	bool rv = false;
	db_mutex.Lock();
	try
	{
		rv = stmt->execute(trunctable.c_str());
	}
	catch (sql::SQLException& e)
	{
		LogError(e);
		ResetConnection(pref);
		db_mutex.Unlock();
		return false;
	}

	db_mutex.Unlock();
	return true;
}
bool Database::LoadPreferencesA(STREAMRECORD_PREFERENCES& pref)
{
	//////ResetConnection(pref);
	db_mutex.Lock();
	try
	{
		
		res = stmt->executeQuery("SELECT * FROM Schedule");
		pref.schedule_entry = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
		int i = 0;

		while (res->next())
		{
			string name = res->getString("ProgramName").c_str();
			string url = res->getString("URL").c_str();
			string day = res->getString("Day").c_str();
			string starthr = res->getString("StartTimeHour");
			string startmin = res->getString("StartTimeMin");
			string endhr = res->getString("EndTimeHour");
			string endmin = res->getString("EndTimeMin");
			string repeating = res->getString("Repeating");
			string shoutcast = res->getString("Shoutcast");
			string id = res->getString("ScheduleID");
			string genre = res->getString("Genre");
			string password = res->getString("Password");
			string monitormountpoint = res->getString("MonitorMountpoint");
			string monitorserver = res->getString("MonitorServer");
			string serverlevel = res->getString("ServerLevel");
			string ignoremp1 = res->getString("IgnoreMP1");
			string ignoremp2 = res->getString("IgnoreMP2");
			string ignoremp3 = res->getString("IgnoreMP3");
			string ignoremp4 = res->getString("IgnoreMP4");
			string ignoremp5 = res->getString("IgnoreMP5");
			string ignoremp6 = res->getString("IgnoreMP6");
			string ignoremp7 = res->getString("IgnoreMP7");
			string ignoremp8 = res->getString("IgnoreMP8");
			string ignoremp9 = res->getString("IgnoreMP9");
			string ignoremp10 = res->getString("IgnoreMP10");
			string ignoremp11 = res->getString("IgnoreMP11");
			string ignoremp12 = res->getString("IgnoreMP12");
			strcpy(pref.schedule_entry[i].password, password.c_str());
			strcpy(pref.schedule_entry[i].program, name.c_str());
			strcpy(pref.schedule_entry[i].stream_URL, url.c_str());
			

			pref.schedule_entry[i].days = std::atoi(day.c_str());
			pref.schedule_entry[i].start_hr = std::atoi(starthr.c_str());
			pref.schedule_entry[i].start_min = std::atoi(startmin.c_str());
			pref.schedule_entry[i].end_hr = std::atoi(endhr.c_str());
			pref.schedule_entry[i].end_min = std::atoi(endmin.c_str());

			pref.schedule_entry[i].repeated = std::atoi(repeating.c_str());
			pref.schedule_entry[i].shoutcast = std::atoi(shoutcast.c_str());
			pref.schedule_entry[i].schedule_id = std::atoi(id.c_str());
			if (pref.schedule_entry[i].schedule_id > max_id)
				max_id = pref.schedule_entry[i].schedule_id;
			pref.schedule_entry[i].genre = std::atoi(genre.c_str());
			pref.schedule_entry[i].thread_ptr = NULL;
			pref.schedule_entry[i].stream_running = FALSE;

			pref.schedule_entry[i].visible = 1;
			pref.schedule_entry[i].recorded = FALSE;
			pref.schedule_entry[i].monitor_mountpoint = std::atoi(monitormountpoint.c_str());
			pref.schedule_entry[i].monitor_server = std::atoi(monitorserver.c_str());
			pref.schedule_entry[i].monitor_level = std::atoi(serverlevel.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[0], ignoremp1.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[1], ignoremp2.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[2], ignoremp3.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[3], ignoremp4.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[4], ignoremp5.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[5], ignoremp6.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[6], ignoremp7.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[7], ignoremp8.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[8], ignoremp9.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[9], ignoremp10.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[10], ignoremp11.c_str());
			strcpy(pref.schedule_entry[i].ignore_mp[11], ignoremp12.c_str());
			
			pref.schedule_entry[i].stream_idx = -1;

			for (int ii = 0; ii < IGNORE_MP_MAX; ii++)
				pref.schedule_entry[i].enable_mp_ignore[ii] = 1;



			i++;
			if (i >= MAX_SCHEDULE_ENTRIES)
			{
				SCHEDULE* temp = new SCHEDULE[MAX_SCHEDULE_ENTRIES*2];
				memcpy(temp, pref.schedule_entry, sizeof(SCHEDULE) * MAX_SCHEDULE_ENTRIES);
				delete[] pref.schedule_entry;
				pref.schedule_entry = temp;
				MAX_SCHEDULE_ENTRIES *= 2;
			}	
			pref.schedule_entry[i].encodebr = 96;
			pref.schedule_entry[i].monitor_level = 0;

		}
		pref.num_entries = i;
		for (int j = 0; j < pref.num_entries; j++)
		{
			pref.schedule_entry[j].limited_retry = FALSE;
			pref.schedule_entry[j].delete_this = FALSE; // m_delete_this;
			pref.schedule_entry[j].encodebr = 96;
			//pref.schedule_entry[j].monitor_server = 0;
			//pref.schedule_entry[j].monitor_mountpoint = 0;
			//pref.schedule_entry[j].monitor_level = 0;
			pref.schedule_entry[j].thread_ptr = NULL;
			pref.schedule_entry[j].stream_running = FALSE;
			if (j > pref.num_entries)
				pref.schedule_entry[j].stream_idx = -2;
			memset(pref.schedule_entry[j].oldbuf, 0, BUFFERSIZE);
			pref.schedule_entry[j].timeout = 0;

			pref.schedule_entry[j].record_now = FALSE;
			pref.schedule_entry[j].recorded = FALSE;
			pref.schedule_entry[j].timeout = 0;
			pref.schedule_entry[j].visible = TRUE;
			pref.schedule_entry[j].status = 0;
			for (int k = 0; k < 3; k++)
			{
				pref.schedule_entry[j].enable_ignore[k] = 0; // enable_ignore[i];
				pref.schedule_entry[j].ignore_day[k] = 0; // ignore_day[i];
				pref.schedule_entry[j].ignore_start_hr[k] = 0; // ignore_start_hr[i];
				pref.schedule_entry[j].ignore_start_min[k] = 0; // ignore_start_min[i];
				pref.schedule_entry[j].ignore_end_hr[k] = 0; // ignore_end_hr[i];
				pref.schedule_entry[j].ignore_end_min[k] = 0; // ignore_end_min[i];
			}
		}
		db_updated = true;
		db_mutex.Unlock();
		return true;
	}
	catch (sql::SQLException& e)
	{
		LogError(e);
		ResetConnection(pref);
	}
	db_mutex.Unlock();
	return false;
}
bool Database::SavePreferences(const STREAMRECORD_PREFERENCES& pref,int n, bool prune)
{
	int k, i = 0;
	bool rv = false;

	char update[4096], insert[4096];
	char update2[256], delete_id[256];
	

	

	////db_mutex.Lock();
	
	/////ResetConnection(pref);
	

	if (n == -1)
	{
		for (i = 0; i < pref.num_entries; i++)
		{
			if (prune && strcmp(pref.DBpassword, pref.schedule_entry[i].password) == 0)
			{
				string daystr = "";
				string status = "";
				char dotw[] = "SMTWThFSa";
				k = 0;
				for (int j = 0; j < 8; j++)
				{
					if ((pref.schedule_entry[i].days >> j) & 0x1)
						daystr += dotw[k];
					k++;
					if ((pref.schedule_entry[i].days >> j) & 0x1 && (k == 5 || k == 8))
						daystr += dotw[k++];
					else if (k == 5 || k == 8)
						k++;
				}
				switch (pref.schedule_entry[i].status)
				{
				case 0: status = "Queued";
					break;
				case 1: status = "Recording";
					break;
				case 2: status = "Reconnecting";
					break;
				case 3: status = "Aborted";
					break;
				case 4: status = "Lost connection";
					break;
				}

				strcpy(pref.schedule_entry[i].program, parse_string(pref.schedule_entry[i].program, (int)strlen(pref.schedule_entry[i].program)).c_str());
				strcpy(pref.schedule_entry[i].stream_URL, parse_string(pref.schedule_entry[i].stream_URL, (int)strlen(pref.schedule_entry[i].stream_URL)).c_str());

				sprintf_s(update, "UPDATE schedule SET ProgramName = '%s', URL = '%s', Day = %d, daystr = '%s',StartTimeHour = %d, StartTimeMin = %d, EndTimeHour=%d, EndTimeMin=%d, Repeating=%d, Shoutcast=%d, Genre=%d, Status='%s', Starttime='%s',Endtime='%s', Password='%s', MonitorMountpoint='%d',MonitorServer='%d',ServerLevel='%d',IgnoreMP1='%s',IgnoreMP2='%s',IgnoreMP3='%s',IgnoreMP4='%s',IgnoreMP5='%s',IgnoreMP6='%s', IgnoreMP7='%s', IgnoreMP8='%s',IgnoreMP9='%s',IgnoreMP10='%s',IgnoreMP11='%s',IgnoreMP12='%s',Timeout='%d', Lastmod='%s' WHERE SCHEDULEID = %d",
					pref.schedule_entry[i].program, pref.schedule_entry[i].stream_URL, pref.schedule_entry[i].days, daystr.c_str(), pref.schedule_entry[i].start_hr, pref.schedule_entry[i].start_min, pref.schedule_entry[i].end_hr,
					pref.schedule_entry[i].end_min, pref.schedule_entry[i].repeated, pref.schedule_entry[i].shoutcast, pref.schedule_entry[i].genre, status.c_str(),
					pref.schedule_entry[i].starttime, pref.schedule_entry[i].endtime, pref.schedule_entry[i].password,
					pref.schedule_entry[i].monitor_mountpoint, pref.schedule_entry[i].monitor_server, pref.schedule_entry[i].monitor_level,
					pref.schedule_entry[i].ignore_mp[0], pref.schedule_entry[i].ignore_mp[1], pref.schedule_entry[i].ignore_mp[2],
					pref.schedule_entry[i].ignore_mp[3], pref.schedule_entry[i].ignore_mp[4], pref.schedule_entry[i].ignore_mp[5],
					pref.schedule_entry[i].ignore_mp[6], pref.schedule_entry[i].ignore_mp[7], pref.schedule_entry[i].ignore_mp[8],
					pref.schedule_entry[i].ignore_mp[9], pref.schedule_entry[i].ignore_mp[10], pref.schedule_entry[i].ignore_mp[11],
					pref.schedule_entry[i].timeout,
					pref.datetime,
					pref.schedule_entry[i].schedule_id);

				sprintf_s(insert, "INSERT INTO schedule (PROGRAMNAME,URL,DAY,DAYSTR,STARTTIMEHOUR,STARTTIMEMIN,ENDTIMEHOUR,ENDTIMEMIN,REPEATING,SHOUTCAST,GENRE,STATUS,STARTTIME,ENDTIME,PASSWORD,MONITORMOUNTPOINT,MONITORSERVER,SERVERLEVEL, IGNOREMP1,IGNOREMP2,IGNOREMP3,IGNOREMP4,IGNOREMP5,IGNOREMP6,IGNOREMP7,IGNOREMP8,IGNOREMP9,IGNOREMP10,IGNOREMP11,IGNOREMP12,TIMEOUT,LASTMOD) VALUES ('%s','%s',%d,'%s',%d,%d,%d,%d,%d,%d,%d,'%s','%s','%s','%s',%d,%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s')",
					pref.schedule_entry[i].program, pref.schedule_entry[i].stream_URL, pref.schedule_entry[i].days, daystr.c_str(), pref.schedule_entry[i].start_hr, pref.schedule_entry[i].start_min,
					pref.schedule_entry[i].end_hr, pref.schedule_entry[i].end_min, pref.schedule_entry[i].repeated, pref.schedule_entry[i].shoutcast, pref.schedule_entry[i].genre, status.c_str(),
					pref.schedule_entry[i].starttime, pref.schedule_entry[i].endtime, pref.schedule_entry[i].password,
					pref.schedule_entry[i].monitor_mountpoint, pref.schedule_entry[i].monitor_server,
					pref.schedule_entry[i].monitor_level, pref.schedule_entry[i].ignore_mp[0], pref.schedule_entry[i].ignore_mp[1], pref.schedule_entry[i].ignore_mp[2],
					pref.schedule_entry[i].ignore_mp[3], pref.schedule_entry[i].ignore_mp[4], pref.schedule_entry[i].ignore_mp[5],
					pref.schedule_entry[i].ignore_mp[6], pref.schedule_entry[i].ignore_mp[7], pref.schedule_entry[i].ignore_mp[8],
					pref.schedule_entry[i].ignore_mp[9], pref.schedule_entry[i].ignore_mp[10], pref.schedule_entry[i].ignore_mp[11],
					pref.schedule_entry[i].timeout,
					pref.datetime
				);

				sprintf_s(update2, "UPDATE recstatus SET LASTMOD = '%s' WHERE STATUSID = 1", pref.datetime);

				try
				{
					if (pref.schedule_entry[i].schedule_id != 0)
					{
						rv = stmt->executeUpdate(update);
					}
					else
					{
						rv = stmt->executeUpdate(insert);
					}

				}
				catch (sql::SQLException& e)
				{
					LogError(e);
					ResetConnection(pref);
					////db_mutex.Unlock();
					return false;
				}
			}
			else if (prune)
			{
				sprintf_s(delete_id, "DELETE FROM schedule WHERE ScheduleID=%d", pref.schedule_entry[i].schedule_id);

				try
				{
					stmt->executeUpdate(delete_id);
				}
				catch (sql::SQLException& e)
				{
					LogError(e);
					return false;
				}
			}
		}

		
	}
	else
	{
		string daystr = "";
		string status = "";
		char dotw[] = "SMTWThFSa";
		k = 0;
		for (int j = 0; j < 8; j++)
		{
			if ((pref.schedule_entry[n].days >> j) & 0x1)
				daystr += dotw[k];
			k++;
			if ((pref.schedule_entry[n].days >> j) & 0x1 && (k == 5 || k == 8))
				daystr += dotw[k++];
			else if (k == 5 || k == 8)
				k++;
		}
		switch (pref.schedule_entry[n].status)
		{
		case 0: status = "Queued";
			break;
		case 1: status = "Recording";
			break;
		case 2: status = "Reconnecting";
			break;
		case 3: status = "Aborted";
			break;
		case 4: status = "Lost connection";
			break;
		}
		strcpy(pref.schedule_entry[n].program, parse_string(pref.schedule_entry[n].program, (int)strlen(pref.schedule_entry[n].program)).c_str());
		strcpy(pref.schedule_entry[n].stream_URL, parse_string(pref.schedule_entry[n].stream_URL, (int)strlen(pref.schedule_entry[n].stream_URL)).c_str());

		sprintf_s(update2, "UPDATE recstatus SET LASTMOD = '%s' WHERE STATUSID = 1", pref.datetime);

		sprintf_s(update, "UPDATE schedule SET ProgramName = '%s', URL = '%s', Day = %d, daystr = '%s',StartTimeHour = %d, StartTimeMin = %d, EndTimeHour=%d, EndTimeMin=%d, Repeating=%d, Shoutcast=%d, Genre=%d, Status='%s', Starttime='%s',Endtime='%s',Password='%s', MonitorMountpoint='%d',MonitorServer='%d',ServerLevel='%d',IgnoreMP1='%s',IgnoreMP2='%s',IgnoreMP3='%s',IgnoreMP4='%s',IgnoreMP5='%s',IgnoreMP6='%s',IgnoreMP7='%s',IgnoreMP8='%s',IgnoreMP9='%s',IgnoreMP10='%s',IgnoreMP11='%s',IgnoreMP12='%s',Timeout=%d, Lastmod='%s' WHERE SCHEDULEID = %d",
			pref.schedule_entry[n].program, pref.schedule_entry[n].stream_URL, pref.schedule_entry[n].days, daystr.c_str(), pref.schedule_entry[n].start_hr, pref.schedule_entry[n].start_min, pref.schedule_entry[n].end_hr,
			pref.schedule_entry[n].end_min, pref.schedule_entry[n].repeated, pref.schedule_entry[n].shoutcast, pref.schedule_entry[n].genre, status.c_str(), 
			pref.schedule_entry[n].starttime,pref.schedule_entry[n].endtime, pref.schedule_entry[n].password,
			pref.schedule_entry[n].monitor_mountpoint, pref.schedule_entry[n].monitor_server,
			pref.schedule_entry[n].monitor_level, pref.schedule_entry[n].ignore_mp[0], pref.schedule_entry[n].ignore_mp[1], pref.schedule_entry[n].ignore_mp[2],
			pref.schedule_entry[n].ignore_mp[3], pref.schedule_entry[n].ignore_mp[4], pref.schedule_entry[n].ignore_mp[5],
			pref.schedule_entry[n].ignore_mp[6], pref.schedule_entry[n].ignore_mp[7], pref.schedule_entry[n].ignore_mp[8],
			pref.schedule_entry[n].ignore_mp[9], pref.schedule_entry[n].ignore_mp[10], pref.schedule_entry[n].ignore_mp[11],
			pref.schedule_entry[n].timeout,
			pref.datetime,
			pref.schedule_entry[n].schedule_id);

		sprintf_s(insert, "INSERT INTO schedule (PROGRAMNAME,URL,DAY,DAYSTR,STARTTIMEHOUR,STARTTIMEMIN,ENDTIMEHOUR,ENDTIMEMIN,REPEATING,SHOUTCAST,GENRE,STATUS,STARTTIME,ENDTIME,PASSWORD,MONITORMOUNTPOINT,MONITORSERVER,SERVERLEVEL,IGNOREMP1,IGNOREMP2,IGNOREMP3,IGNOREMP4,IGNOREMP5,IGNOREMP6,IGNOREMP7,IGNOREMP8,IGNOREMP9,IGNOREMP10,IGNOREMP11,IGNOREMP12,TIMEOUT,LASTMOD) VALUES ('%s','%s',%d,'%s',%d,%d,%d,%d,%d,%d,%d,'%s','%s','%s','%s',%d,%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',%d,'%s')",
			pref.schedule_entry[n].program, pref.schedule_entry[n].stream_URL, pref.schedule_entry[n].days, daystr.c_str(), pref.schedule_entry[n].start_hr, pref.schedule_entry[n].start_min,
			pref.schedule_entry[n].end_hr, pref.schedule_entry[n].end_min, pref.schedule_entry[n].repeated, pref.schedule_entry[n].shoutcast, pref.schedule_entry[n].genre, status.c_str(), 
			pref.schedule_entry[n].starttime,pref.schedule_entry[n].endtime,pref.schedule_entry[i].password,
			pref.schedule_entry[n].monitor_mountpoint, pref.schedule_entry[n].monitor_server,
			pref.schedule_entry[n].monitor_level, 
			pref.schedule_entry[n].ignore_mp[0], 
			pref.schedule_entry[n].ignore_mp[1], pref.schedule_entry[n].ignore_mp[2], pref.schedule_entry[n].ignore_mp[3],
			pref.schedule_entry[n].ignore_mp[4], pref.schedule_entry[n].ignore_mp[5],
			pref.schedule_entry[n].ignore_mp[6],
			pref.schedule_entry[n].ignore_mp[7], pref.schedule_entry[n].ignore_mp[8], pref.schedule_entry[n].ignore_mp[9],
			pref.schedule_entry[n].ignore_mp[10], pref.schedule_entry[n].ignore_mp[11],
			pref.schedule_entry[n].timeout,pref.datetime);
		
		/*
		sprintf_s(update, "UPDATE schedule SET ProgramName = '%s', URL = '%s', Day = %d, daystr = '%s',StartTimeHour = %d, StartTimeMin = %d, EndTimeHour=%d, EndTimeMin=%d, Repeating=%d, Shoutcast=%d, Genre=%d, Status='%s', Password='%s' WHERE SCHEDULEID = %d",
			pref.schedule_entry[n].program, pref.schedule_entry[n].stream_URL, pref.schedule_entry[n].days, daystr.c_str(), pref.schedule_entry[n].start_hr, pref.schedule_entry[n].start_min, pref.schedule_entry[n].end_hr,
			pref.schedule_entry[n].end_min, pref.schedule_entry[n].repeated, pref.schedule_entry[n].shoutcast, pref.schedule_entry[n].genre, status.c_str(), pref.DBpassword, pref.schedule_entry[n].schedule_id);

		sprintf_s(insert, "INSERT INTO schedule (PROGRAMNAME,URL,DAY,DAYSTR,STARTTIMEHOUR,STARTTIMEMIN,ENDTIMEHOUR,ENDTIMEMIN,REPEATING,SHOUTCAST,GENRE,STATUS,PASSWORD) VALUES ('%s','%s',%d,'%s',%d,%d,%d,%d,%d,%d,%d,'Queued','%s')",
			pref.schedule_entry[i].program, pref.schedule_entry[n].stream_URL, pref.schedule_entry[n].days, daystr.c_str(), pref.schedule_entry[n].start_hr, pref.schedule_entry[n].start_min,
			pref.schedule_entry[n].end_hr, pref.schedule_entry[n].end_min, pref.schedule_entry[n].repeated, pref.schedule_entry[n].shoutcast, pref.schedule_entry[n].genre, pref.DBpassword);
		*/

		try
		{
			if (pref.schedule_entry[n].schedule_id != 0)
			{
				rv = stmt->executeUpdate(update);
			}
			else
			{
				rv = stmt->executeUpdate(insert);
			}
		}
		catch (sql::SQLException& e)
		{
			LogError(e);
			ResetConnection(pref);
			/////db_mutex.Unlock();
			return false;
		}

		try
		{
			stmt->executeUpdate(update2);
		}
		catch (sql::SQLException& e)
		{
			preferences_lock = false;
			LogError(e);
			ResetConnection(pref);
			/////db_mutex.Unlock();
			return false;
		}

	}
	///db_mutex.Unlock();
	
	return true;
}
bool Database::DeletePreferences(const int id)
{
	char delete_id[256];

	///////ResetConnection(*ppref);

	sprintf_s(delete_id,"DELETE FROM schedule WHERE ScheduleID=%d", id);
	
	try
	{
		if (id != 0)
			stmt->executeUpdate(delete_id);
	}
	catch (sql::SQLException& e)
	{
		LogError(e);
		return false;
	}
	return true;
}
bool Database::LoadPreferences(STREAMRECORD_PREFERENCES& pref)
{
	char update[1024],update2[1024];
	bool rv = false;
	char delentry[256];
	

	if (db_updated)
	{
		
		sprintf_s(update2, "UPDATE schedule SET Lastmod='%s' WHERE SCHEDULEID = %d",
			pref.datetime,
			pref.schedule_entry[0].schedule_id);
	}
	sprintf_s(update, "UPDATE recstatus SET LASTMOD = '%s' WHERE STATUSID = 1", pref.datetime);

	try
	{
		if (stmt != NULL)
		{
			rv = stmt->executeUpdate(update);
			if (db_updated)
				rv = stmt->executeUpdate(update2);
			res = stmt->executeQuery("SELECT * FROM Schedule");
		}
	}
	catch (sql::SQLException& e)
	{

		LogError(e);
		ResetConnection(pref);
		preferences_lock = false;
		return false;
	}
	
	
	long i = 0;
	long max = 0;

	
	////pref_struct param_ptr;
	///param_ptr.pref = &pref;
	///param_ptr.temp = temp;

	//schedule_mutex.Lock();
	if (temp == NULL)
	{
		temp = new STREAMRECORD_PREFERENCES;
		temp->schedule_entry = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
	}
	

	///AfxBeginThread(CopyPref, (LPVOID)&param_ptr, THREAD_PRIORITY_NORMAL);
	
	if (!pinit) //pref.schedule_entry == NULL)
	{
		pinit = true;
		pref.schedule_entry = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
		for (int i = 0; i < MAX_SCHEDULE_ENTRIES; i++)
			pref.schedule_entry[i].thread_ptr = NULL;
	}
	else
	{
		for (i = 0; i < pref.num_entries; i++)
			if (pref.schedule_entry[i].schedule_id > max)
				max = pref.schedule_entry[i].schedule_id;
		willpurge = new char[max*2 + 1];
		memset(willpurge, 0, max*2 + 1);
		for (i = 0; i < pref.num_entries; i++)
			willpurge[pref.schedule_entry[i].schedule_id] = pref.schedule_entry[i].willpurge;
	}
	pref.pruning = 0;
	i = 0;
	///memcpy(temp,&pref, sizeof(STREAMRECORD_PREFERENCES));
	//memcpy(temp->schedule_entry, pref.schedule_entry, sizeof(SCHEDULE) * MAX_SCHEDULE_ENTRIES);
	
	//preferences_lock = false;

	try
	{
		if (res != NULL)
		{
			while (res->next())
			{
				string name = res->getString("ProgramName").c_str();
				string url = res->getString("URL").c_str();
				string day = res->getString("Day").c_str();
				string starthr = res->getString("StartTimeHour");
				string startmin = res->getString("StartTimeMin");
				string endhr = res->getString("EndTimeHour");
				string endmin = res->getString("EndTimeMin");
				string repeating = res->getString("Repeating");
				string shoutcast = res->getString("Shoutcast");
				string id = res->getString("ScheduleID");
				string genre = res->getString("Genre");
				string status = res->getString("Status");
				string password = res->getString("Password");
				string starttime = res->getString("Starttime");
				string endtime = res->getString("Endtime");
				string monitormountpoint = res->getString("MonitorMountpoint");
				string monitorserver = res->getString("MonitorServer");
				string serverlevel = res->getString("ServerLevel");
				string ignoremp1 = res->getString("IgnoreMP1");
				string ignoremp2 = res->getString("IgnoreMP2");
				string ignoremp3 = res->getString("IgnoreMP3");
				string ignoremp4 = res->getString("IgnoreMP4");
				string ignoremp5 = res->getString("IgnoreMP5");
				string ignoremp6 = res->getString("IgnoreMP6");
				string ignoremp7 = res->getString("IgnoreMP7");
				string ignoremp8 = res->getString("IgnoreMP8");
				string ignoremp9 = res->getString("IgnoreMP9");
				string ignoremp10 = res->getString("IgnoreMP10");
				string ignoremp11 = res->getString("IgnoreMP11");
				string ignoremp12 = res->getString("IgnoreMP12");

				string timeout = res->getString("Timeout");

				if (strcmp(url.c_str(), "") == 0)
					continue;
				strcpy(temp->schedule_entry[i].password, password.c_str());
				temp->schedule_entry[i].schedule_id = std::atoi(id.c_str());
				
				if (max != 0)
					temp->schedule_entry[i].willpurge = willpurge[temp->schedule_entry[i].schedule_id];
				
				if (temp->schedule_entry[i].willpurge == 1)
				{
					//sprintf_s(delentry, "DELETE FROM schedule WHERE ScheduleID=%d", temp->schedule_entry[i].schedule_id);
					//stmt->executeUpdate(delentry);
					DeletePreferences(temp->schedule_entry[i].schedule_id);
					temp->schedule_entry[i].willpurge = 0;
					temp->schedule_entry[i].monitor_mountpoint = 0;
					temp->schedule_entry[i].schedule_id = 0;
					memset(&temp->schedule_entry[i], 0, sizeof(temp->schedule_entry[i]));
					pref.pruning = 1;
					return false;
					////continue;
				}
				
			
				if (strcmp(temp->schedule_entry[i].password, pref.DBpassword) != 0) //|| temp->schedule_entry[i].willpurge)
				{
					DeletePreferences(temp->schedule_entry[i].schedule_id);
					///memset(((LPVOID)temp->schedule_entry[i]), 0, sizeof(temp->schedule_entry[i]));
					continue;
				}
				
				strcpy(temp->schedule_entry[i].program, name.c_str());
				strcpy(temp->schedule_entry[i].stream_URL, url.c_str());
				temp->schedule_entry[i].days = std::atoi(day.c_str());

				temp->schedule_entry[i].start_hr = std::atoi(starthr.c_str());
				temp->schedule_entry[i].start_min = std::atoi(startmin.c_str());
				temp->schedule_entry[i].end_hr = std::atoi(endhr.c_str());
				temp->schedule_entry[i].end_min = std::atoi(endmin.c_str());

				temp->schedule_entry[i].repeated = std::atoi(repeating.c_str());
				temp->schedule_entry[i].shoutcast = std::atoi(shoutcast.c_str());
				
				temp->schedule_entry[i].genre = std::atoi(genre.c_str());
				strcpy(temp->schedule_entry[i].starttime, starttime.c_str());
				strcpy(temp->schedule_entry[i].endtime, endtime.c_str());
				temp->schedule_entry[i].monitor_mountpoint = atoi(monitormountpoint.c_str());
				temp->schedule_entry[i].monitor_server = atoi(monitorserver.c_str());
				temp->schedule_entry[i].monitor_level = atoi(serverlevel.c_str());

				strcpy(temp->schedule_entry[i].ignore_mp[0], ignoremp1.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[1], ignoremp2.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[2], ignoremp3.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[3], ignoremp4.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[4], ignoremp5.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[5], ignoremp6.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[6], ignoremp7.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[7], ignoremp8.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[8], ignoremp9.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[9], ignoremp10.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[10], ignoremp11.c_str());
				strcpy(temp->schedule_entry[i].ignore_mp[11], ignoremp12.c_str());
				temp->schedule_entry[i].timeout = atoi(timeout.c_str());
				////temp->schedule_entry[i].status = 0; // std::atoi(status.c_str());
				temp->schedule_entry[i].thread_ptr = NULL;
				temp->schedule_entry[i].stream_running = FALSE;
				temp->schedule_entry[i].stream_idx = -1;
				temp->schedule_entry[i].visible = 1;
				temp->schedule_entry[i].recorded = FALSE;

				if (_strcmpi(status.c_str(), "Queued") == 0)
					temp->schedule_entry[i].status = 0;
				else if (_strcmpi(status.c_str(), "Recording") == 0)
					temp->schedule_entry[i].status = 1;
				else if (_strcmpi(status.c_str(), "Reconnecting") == 0)
					temp->schedule_entry[i].status = 2;
				else if (_strcmpi(status.c_str(), "Aborted") == 0)
					temp->schedule_entry[i].status = 3;
				else if (_strcmpi(status.c_str(), "Lost connection") == 0)
					temp->schedule_entry[i].status = 4;
				else if (_strcmpi(status.c_str(), "Done recording") == 0)
					temp->schedule_entry[i].status = 5;
				i++;

				if (i >= MAX_SCHEDULE_ENTRIES)
				{
					SCHEDULE* temp = new SCHEDULE[MAX_SCHEDULE_ENTRIES * 2];
					memcpy(temp, pref.schedule_entry, sizeof(SCHEDULE) * MAX_SCHEDULE_ENTRIES);
					delete[] pref.schedule_entry;
					pref.schedule_entry = temp;
					MAX_SCHEDULE_ENTRIES *= 2;
				}
			}
		}
	}
	catch (sql::SQLException& e)
	{
		
		LogError(e);
		ResetConnection(pref);
		
		return false;
	}
	prev = pref.num_entries;
	temp->num_entries = i;
	//pref.num_entries = i;
	
	for (int j = 0; j < temp->num_entries; j++)
	{
		temp->schedule_entry[j].limited_retry = FALSE;
		temp->schedule_entry[j].delete_this = FALSE; // m_delete_this;
		temp->schedule_entry[j].encodebr = 96;
		//temp->schedule_entry[j].monitor_server = 0;
		//temp->schedule_entry[j].monitor_mountpoint = 0;
		//temp->schedule_entry[j].monitor_level = 0;
		temp->schedule_entry[j].thread_ptr = NULL;
		temp->schedule_entry[j].stream_running = FALSE;
		if (j > pref.num_entries)
			temp->schedule_entry[j].stream_idx = -2;
		/////memset(temp->schedule_entry[j].oldbuf, 0, BUFFERSIZE);

		temp->schedule_entry[j].record_now = FALSE;
		temp->schedule_entry[j].recorded = FALSE;
		
		temp->schedule_entry[j].visible = TRUE;
		for (int k = 0; k < 3; k++)
		{
			temp->schedule_entry[j].enable_ignore[k] = 0; // enable_ignore[i];
			temp->schedule_entry[j].ignore_day[k] = 0; // ignore_day[i];
			temp->schedule_entry[j].ignore_start_hr[k] = 0; // ignore_start_hr[i];
			temp->schedule_entry[j].ignore_start_min[k] = 0; // ignore_start_min[i];
			temp->schedule_entry[j].ignore_end_hr[k] = 0; // ignore_end_hr[i];
			temp->schedule_entry[j].ignore_end_min[k] = 0; // ignore_end_min[i];
		}
		
	}
	
	
	//memcpy(&pref, temp, sizeof(STREAMRECORD_PREFERENCES));
	//memcpy(pref.schedule_entry, temp->schedule_entry, sizeof(SCHEDULE) * MAX_SCHEDULE_ENTRIES);
	///////schedule_mutex.Unlock();
	delete[] willpurge;
	
	db_updated = true;
	return true;
}
typedef CWinThread* wptr;

bool Database::CopySchedule(STREAMRECORD_PREFERENCES& pref)
{
	int i = 0;
	static bool init = false;
	while (temp == NULL) 
		Sleep(100);
	if (!db_updated)
		return false;

	//BOOL* visible = NULL;
	///char* willpurge = NULL;

	
	
	///schedule_mutex.Lock();
	if (temp != NULL && temp->num_entries > 0)
	{
 		for (i = 0, max_id = 0; i < temp->num_entries; i++)
			if (temp->schedule_entry[i].schedule_id > max_id)
				max_id = temp->schedule_entry[i].schedule_id;
		temp_ptr = new wptr[max_id*2+1];
		temp_streams = new long[max_id*2 + 1];
		temp_recorded = new BOOL[max_id*2 + 1];
		temp_stream_running = new BOOL[max_id*2 + 1];
		///willpurge = new char[max_id + 1];
		//visible = new BOOL[max_id + 1];
		memset(temp_ptr, 0, sizeof(wptr) * (max_id+1));
		memset(temp_streams, -1, sizeof(long) * (max_id+1));
		memset(temp_recorded, 0, sizeof(BOOL) * (max_id + 1));
		memset(temp_stream_running, 0, sizeof(BOOL) * (max_id + 1));
		///memset(willpurge, 0, sizeof(char) * (max_id + 1));
		//memset(visible, 0, sizeof(BOOL) * (max_id + 1));
		if (init)
		{
			for (i = 0; i < pref.num_entries; i++)
			{
				//if (pref.schedule_entry[i].schedule_id == 65535)
				//	pref.schedule_entry[i].schedule_id = temp->schedule_entry[i].schedule_id;
				temp_ptr[pref.schedule_entry[i].schedule_id] = pref.schedule_entry[i].thread_ptr;
				temp_streams[pref.schedule_entry[i].schedule_id] = pref.schedule_entry[i].stream_idx;
				temp_recorded[pref.schedule_entry[i].schedule_id] = pref.schedule_entry[i].recorded;
				temp_stream_running[pref.schedule_entry[i].schedule_id] = pref.schedule_entry[i].stream_running;
				////willpurge[pref.schedule_entry[i].schedule_id] = pref.schedule_entry[i].willpurge;
				//visible[pref.schedule_entry[i].schedule_id] = pref.schedule_entry[i].visible;
			}
		}

		pref.num_entries = temp->num_entries;

		for (i = 0; i < temp->num_entries; i++)
		{

			//if (strcmp(pref.schedule_entry[i].program, temp->schedule_entry[i].program) != 0 && temp->schedule_entry[i].thread_ptr == NULL)
			//	pref.schedule_entry[i].thread_ptr = NULL;
			strcpy(pref.schedule_entry[i].program, temp->schedule_entry[i].program); // , name.c_str());
			strcpy(pref.schedule_entry[i].stream_URL, temp->schedule_entry[i].stream_URL); // , url.c_str());
			strcpy(pref.schedule_entry[i].starttime, temp->schedule_entry[i].starttime);
			strcpy(pref.schedule_entry[i].endtime, temp->schedule_entry[i].endtime);
			strcpy(pref.schedule_entry[i].password,temp->schedule_entry[i].password);
			pref.schedule_entry[i].days = temp->schedule_entry[i].days; // = //std::atoi(day.c_str());

			pref.schedule_entry[i].start_hr = temp->schedule_entry[i].start_hr;
			pref.schedule_entry[i].start_min = temp->schedule_entry[i].start_min; // = std::atoi(startmin.c_str());
			pref.schedule_entry[i].end_hr = temp->schedule_entry[i].end_hr; // = std::atoi(endhr.c_str());
			pref.schedule_entry[i].end_min = temp->schedule_entry[i].end_min; // = std::atoi(endmin.c_str());

			pref.schedule_entry[i].repeated = temp->schedule_entry[i].repeated; // = std::atoi(repeating.c_str());
			pref.schedule_entry[i].shoutcast = temp->schedule_entry[i].shoutcast; // = std::atoi(shoutcast.c_str());
			
			pref.schedule_entry[i].schedule_id = temp->schedule_entry[i].schedule_id; // = std::atoi(id.c_str());
			pref.schedule_entry[i].thread_ptr = temp_ptr[temp->schedule_entry[i].schedule_id];
			pref.schedule_entry[i].stream_idx = temp_streams[temp->schedule_entry[i].schedule_id];
			pref.schedule_entry[i].recorded = temp_recorded[temp->schedule_entry[i].schedule_id];
			////pref.schedule_entry[i].willpurge = willpurge[temp->schedule_entry[i].schedule_id];
			
			
			/*
			if (pref.schedule_entry[i].willpurge)
			{
				DeletePreferences(pref.schedule_entry[i].schedule_id);
				pref.schedule_entry[i].willpurge = 0;
				continue;
			}
			*/
			
			
			pref.schedule_entry[i].stream_running = temp_stream_running[temp->schedule_entry[i].schedule_id];
			pref.schedule_entry[i].timeout = temp->schedule_entry[i].timeout;

			pref.schedule_entry[i].genre = temp->schedule_entry[i].genre; // = std::atoi(genre.c_str());

			pref.schedule_entry[i].status = temp->schedule_entry[i].status;

			pref.schedule_entry[i].monitor_mountpoint = temp->schedule_entry[i].monitor_mountpoint;
			pref.schedule_entry[i].monitor_server = temp->schedule_entry[i].monitor_server;
			pref.schedule_entry[i].monitor_level = temp->schedule_entry[i].monitor_level;
			strcpy(pref.schedule_entry[i].ignore_mp[0], temp->schedule_entry[i].ignore_mp[0]);
			strcpy(pref.schedule_entry[i].ignore_mp[1], temp->schedule_entry[i].ignore_mp[1]);
			strcpy(pref.schedule_entry[i].ignore_mp[2], temp->schedule_entry[i].ignore_mp[2]);
			strcpy(pref.schedule_entry[i].ignore_mp[3], temp->schedule_entry[i].ignore_mp[3]);
			strcpy(pref.schedule_entry[i].ignore_mp[4], temp->schedule_entry[i].ignore_mp[4]);
			strcpy(pref.schedule_entry[i].ignore_mp[5], temp->schedule_entry[i].ignore_mp[5]);
			strcpy(pref.schedule_entry[i].ignore_mp[6], temp->schedule_entry[i].ignore_mp[6]);
			strcpy(pref.schedule_entry[i].ignore_mp[7], temp->schedule_entry[i].ignore_mp[7]);
			strcpy(pref.schedule_entry[i].ignore_mp[8], temp->schedule_entry[i].ignore_mp[8]);
			strcpy(pref.schedule_entry[i].ignore_mp[9], temp->schedule_entry[i].ignore_mp[9]);
			strcpy(pref.schedule_entry[i].ignore_mp[10], temp->schedule_entry[i].ignore_mp[10]);
			strcpy(pref.schedule_entry[i].ignore_mp[11], temp->schedule_entry[i].ignore_mp[11]);

			for (int ii = 0; ii < IGNORE_MP_MAX; ii++)
				pref.schedule_entry[i].enable_mp_ignore[ii] = 1;

			if (pref.schedule_entry[i].thread_ptr == NULL)
			{
				pref.schedule_entry[i].limited_retry = FALSE;
				pref.schedule_entry[i].delete_this = FALSE; // m_delete_this;
				pref.schedule_entry[i].encodebr = 96;
				//pref.schedule_entry[i].monitor_server = 0;
				//pref.schedule_entry[i].monitor_mountpoint = 0;
				//pref.schedule_entry[i].monitor_level = 0;			
				pref.schedule_entry[i].stream_idx = -1;
				pref.schedule_entry[i].record_now = FALSE; // FALSE;
				//pref.schedule_entry[i].recorded = FALSE;
				//pref.schedule_entry[i].stream_running = FALSE;
				pref.schedule_entry[i].visible = TRUE;
				pref.schedule_entry[i].fail_count = 0;
				for (int k = 0; k < 3; k++)
				{
					pref.schedule_entry[i].enable_ignore[k] = 0; // enable_ignore[i];
					pref.schedule_entry[i].ignore_day[k] = 0; // ignore_day[i];
					pref.schedule_entry[i].ignore_start_hr[k] = 0; // ignore_start_hr[i];
					pref.schedule_entry[i].ignore_start_min[k] = 0; // ignore_start_min[i];
					pref.schedule_entry[i].ignore_end_hr[k] = 0; // ignore_end_hr[i];
					pref.schedule_entry[i].ignore_end_min[k] = 0; // ignore_end_min[i];
				}
			}

			
			///temp->schedule_entry[j].thread_ptr = NULL;
			//temp->schedule_entry[j].stream_running = FALSE;
			
		//}
		}
		if (temp_ptr != NULL)
			delete[] temp_ptr;
		if (temp_streams != NULL)
			delete[] temp_streams;
		if (temp_recorded != NULL)
			delete[] temp_recorded;
		if (temp_stream_running != NULL)
			delete[] temp_stream_running;
		//if (willpurge != NULL)
		//	delete[] willpurge;
		//if (visible != NULL)
		//	delete[] visible;

		////schedule_mutex.Unlock();
		
	}
	
	db_updated = true;
	init = true;
	
	/////schedule_mutex.Unlock();
	////pref.num_entries = i; // temp->num_entries;
	return true;
}

Database::~Database()
{
	
	if (temp != NULL)
		delete temp;

}
bool Database::ResetConnection(const STREAMRECORD_PREFERENCES& pref)
{
	//if (con != NULL)
	//	delete con;
	//if (stmt != NULL)
	//	delete stmt;
	//if (res != NULL)
	//	delete res;
	if (temp != NULL)
		delete temp;
	///return true;

	try
	{
		char address[128];
		temp = NULL;
		if (con != NULL)	
			con->close();
		///con->reconnect();
		
		Sleep(1000);
		driver = get_driver_instance();
		sprintf(address, "tcp://%s:%s", pref.ip_address, pref.port);

		con = driver->connect(address, pref.username, pref.password);
		//con = driver->connect("tcp://127.0.0.1:3306", "root", "password");
		//Connect to the MySQL test database 
		con->setSchema(pref.schema);
		//con->setSchema(SCHEMA);
		stmt = con->createStatement();


	
	}
	catch (sql::SQLException& e)
	{
		LogError(e);
		////preferences_lock = false;

	}
	////preferences_lock = false;
	return true;
}

bool Database::ResetStatus(const STREAMRECORD_PREFERENCES& pref, bool flip)
{
	int i = 0;
	string status;
	char update[4096];
	bool rv;

	
	//preferences_lock = true;

	///ResetConnection(pref);
	
	
	for (i = 0; i < pref.num_entries; i++)
	{
		if ((flip && pref.schedule_entry[i].status == 5) || (pref.schedule_entry[i].status != 5 && pref.schedule_entry[i].thread_ptr == NULL))
			pref.schedule_entry[i].status = 0;
		//else if (!flip && pref.schedule_entry[i].thread_ptr == NULL)
		//	pref.schedule_entry[i].status = 5;

		if (pref.schedule_entry[i].thread_ptr != NULL && pref.schedule_entry[i].status == 5)
			pref.schedule_entry[i].status = 1;

		switch (pref.schedule_entry[i].status)
		{
		case 0:
		//case 5:
		default:
			status = "Queued";
			break;
		case 1: status = "Recording";
			break;
		case 2: status = "Reconnecting";
			break;
		case 3: status = "Aborted";
			break;
		case 4: status = "Lost connection";
			break;
		case 5: status = "Done recording";
			break;
		}
	//	if (pref.schedule_entry[i].status == 5 || pref.schedule_entry[i].thread_ptr == NULL)
	//		pref.schedule_entry[i].status = 0;
	

		sprintf_s(update, "UPDATE schedule SET Status='%s' WHERE SCHEDULEID = %d",
			status.c_str(), pref.schedule_entry[i].schedule_id);

		try
		{
			if (stmt != NULL)
				rv = stmt->executeUpdate(update);			
		}
		catch (sql::SQLException& e)
		{
			LogError(e);
			ResetConnection(pref);
			
			return false;
		}
		
	}
	
	return true;
}
bool Database::SetStatus(const STREAMRECORD_PREFERENCES& pref,int n)
{
	char update[1024];
	bool rv = false;
	string status;
	///while (preferences_lock)
	///	Sleep(500);
	

	//if (pref.schedule_entry != NULL && pref.schedule_entry[n].thread_ptr == NULL)
	//	return true;


	while (preferences_lock)
		Sleep(500);
	

	///ResetConnection(pref);


	switch (pref.schedule_entry[n].status)
	{
	case 0:
	default:
		status = "Queued";
		break;
	case 1: status = "Recording";
		break;
	case 2: status = "Reconnecting";
		break;
	case 3: status = "Aborted";
		break;
	case 4: status = "Lost connection";
		break;
	case 5: status = "Done recording";
		break;
	}

	sprintf_s(update, "UPDATE schedule SET Status='%s' WHERE SCHEDULEID = %d",
		status.c_str(), pref.schedule_entry[n].schedule_id);

	try
	{
		rv = stmt->executeUpdate(update);
	}
	catch (sql::SQLException& e)
	{
		LogError(e);
		ResetConnection(pref);
		
		return false;
	}
	
	return true;
}

bool Database::SetStatus(const STREAMRECORD_PREFERENCES& pref)
{
	
	for (int i = 0; i < pref.num_entries; i++)
		if (pref.schedule_entry[i].thread_ptr != NULL)
			pref.schedule_entry[i].status = 0;

	
	return true;

}

bool Database::Reset()
{
	return true;
}
bool Database::PruneSchedule(const STREAMRECORD_PREFERENCES& pref)
{
	static int i = 0;
	char delete_id[256];

	//for (i = 0; i < pref.num_entries; i++)
	{
		if (strcmp(pref.DBpassword, pref.schedule_entry[i].password) != 0)
		{
			sprintf_s(delete_id, "DELETE FROM schedule WHERE ScheduleID=%d", pref.schedule_entry[i].schedule_id);

			try
			{
				stmt->executeUpdate(delete_id);
			}
			catch (sql::SQLException& e)
			{
				LogError(e);
				return false;
			}
		}
		i++;
		if (i >= pref.num_entries)
			i = 0;
	}
	return true;
}