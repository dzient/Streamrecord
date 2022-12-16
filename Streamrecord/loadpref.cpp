#include "stdafx.h"
#include "loadpref.h"
#include "Database.h"
#include "Pushover.h"

#include <direct.h>
#include <afxmt.h>

//-------------------------------------------
// David Zientara
// 11-15-2022
//
// Streamrecord.Dlg.cpp
//
// File for the StreamrecordDlg classs
//
//---------------------------------------------

extern CMutex pref_mutex;

extern CMutex server_mutex;

char working_dir[1024];

Database* dbase = NULL;
Pushover* push = NULL;

struct db_struct
{
	Database* dbptr;
	Pushover* poptr;
	STREAMRECORD_PREFERENCES* pref;
	int n;
	char msg[256];
	bool prune;
	bool flip;
} db_params;


//------------------------------------
// SaveThread
// Function calls SavePreferences
// from a thread
// PARAMS: db_params: pointer to 
// struct for parameters
// RETURNS: 1 if succcessful
//--------------------------------------
CMutex LDB_mutex, push_mutex, prune_mutex;

void WaitForThreads()
{
	for (int i = 0; i < 20; i++)
	{
		LDB_mutex.Lock();
		LDB_mutex.Unlock();
	}
}

UINT LoadandCopyThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;
	bool rval;
	LDB_mutex.Lock();


	do
	{
		rval = db->dbptr->LoadPreferences(*db->pref);
		if (!rval)
		{
			delete dbase;
			dbase = new Database(*db->pref);
			db->dbptr = dbase;
		}
		while (!db->dbptr->CopySchedule(*db->pref))
			Sleep(100);
	} while (!rval);

	LDB_mutex.Unlock();
	return 1;
}
UINT PruneThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;
	LDB_mutex.Lock();
	prune_mutex.Lock();
	db->pref->pruning = 1;
	while (!db->dbptr->PruneSchedule(*db->pref))
		Sleep(100);
	db->pref->pruning = 0;
	prune_mutex.Unlock();
	LDB_mutex.Unlock();
	return 1;

}
UINT SaveThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;
	LDB_mutex.Lock();

	db->pref->pruning = db->prune;
	while (!db->dbptr->SavePreferences(*db->pref, db->n, db->prune))
		Sleep(100);
	
	LDB_mutex.Unlock();
	return 1;
}
//------------------------------------------
//CopyScheduleThread
// Function calls CopySchedule from a thread
// PARAMS: db_params: pointer to 
// struct for parameters
// RETURNS: 1 if succcessful
//--------------------------------------

bool status_lock = false;
bool ssr_init = false;

UINT PushMessageThread(LPVOID db_params)
{
	///while (status_lock)
	///	Sleep(7000);
	db_struct* db = (db_struct*)db_params;

	char msg[256];
	strcpy(msg, db->msg);
	///LDB_mutex.Lock();
	push_mutex.Lock();


	//while (status_lock)
	//	Sleep(5000);

	

	if (!ssr_init) //!db->pref->init)
	{
		Sleep(2000);
		ssr_init = TRUE;
	}
	else
		Sleep(150);
	db->poptr->PushMessage(msg);

	push_mutex.Unlock();
	////LDB_mutex.Unlock();
	return 1;
}
UINT CopyScheduleThread(LPVOID db_params)
{
	
	LDB_mutex.Lock();
	
	db_struct* db = (db_struct*)db_params;
	///while (db->pref->pruning)
	///	Sleep(100);
	while (!db->dbptr->CopySchedule(*db->pref))
		Sleep(100);
	
	LDB_mutex.Unlock();
	return 1;
}
//------------------------------------------
// LoadDatabaseThread
// Function calls LoadDatabase from a thread
// PARAMS: db_params: pointer to 
// struct for parameters
// RETURNS: 1 if succcessful
//--------------------------------------
UINT LoadDatabaseThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;
	bool rval;
	LDB_mutex.Lock();
	
	do
	{
		rval = db->dbptr->LoadPreferences(*db->pref);
		if (!rval)
			Sleep(3000);

		return 1;
		/*
		if (!rval)
		{
			delete dbase;
			dbase = new Database(*db->pref);
			db->dbptr = dbase;
		}
		*/
	} while (!rval);
	
	///rval = db->dbptr->LoadPreferences(*db->pref);
	LDB_mutex.Unlock();
		//Sleep(100);
	return 1;
}
UINT DeleteDatabaseThread(LPVOID db_params)
{
	bool rval;
	db_struct* db = (db_struct*)db_params;
	int n = db->n;
	LDB_mutex.Lock();
	do
	{
		rval = db->dbptr->DeletePreferences(n);
	} while (!rval);
	return 1;
}
//------------------------------------------
// SetStatusThread
// Function calls SetStatus from a thread
// It takes a second parameter, indicating 
// the # of the schedule entry to be updated
// PARAMS: db_params: pointer to 
// struct for parameters
// RETURNS: 1 if succcessful
//--------------------------------------
UINT SetStatusThread(LPVOID db_params)
{
	status_lock = true;
	db_struct* db = (db_struct*)db_params;
	int n = db->n;
	LDB_mutex.Lock();
	

	while (!db->dbptr->SetStatus(*db->pref,n))
		Sleep(5000);
	LDB_mutex.Unlock();
	status_lock = false;
	return 1;
}
//----------------------------------------
// SetStatusOnlyThread
// Function calls SetStatus from a thread
// Calling this function sets the status
// for all programs
// PARAMS: db_params: pointer to 
// struct for parameters
// RETURNS: 1 if succcessful
//--------------------------------------
UINT SetStatusOnlyThread(LPVOID db_params)
{
	LDB_mutex.Lock();
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->SetStatus(*db->pref))
		Sleep(100);
	LDB_mutex.Unlock();
	return 1;
}
//----------------------------------------
// SetStatusOnlyThread
// Function calls SetStatus from a thread
// Calling this function sets the status
// for all programs
// PARAMS: db_params: pointer to 
// struct for parameters
// RETURNS: 1 if succcessful
//--------------------------------------
UINT ResetStatusThread(LPVOID db_params)
{
	LDB_mutex.Lock();
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->ResetStatus(*db->pref,db->flip))
		Sleep(100);
	LDB_mutex.Unlock();
	return 1;
}
//-------------------------------------------------
// ResetConnectionThread
// Function calls ResetConnection from a thread
// PARAMS: db_params: pointer to 
// struct for parameters
// RETURNS: 1 if succcessful
//--------------------------------------
UINT ResetConnectionThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->ResetConnection(*db->pref))
		Sleep(100);
	return 1;
}
//-----------------------------------------------
// chartoWchar
// Function converts an array of chars to an
// array of wide chars
// PARAMS: text (array of chars)
// RETURNS: array of wchar_t
//------------------------------------------------

static wchar_t* charToWChar(const char* text)
{
	const size_t size = strlen(text) + 1;
	wchar_t* wText = new wchar_t[size];
	mbstowcs(wText, text, size);
	return wText;
}
//--------------------------------------------------
// LoadPreferences
// Function loads preferencs from disk and
// loads databse/copies to preferences if database
// is enabled
// PARAMS: pref_file (array of chars), pref (preferences),
// ignore (IGNORE_LIST), add (SCHEDULE_ADD_LIST)
// RETURNS: 1 if successful, 0 otherwise
//---------------------------------------------------
int LoadPreferences(const char *pref_file, STREAMRECORD_PREFERENCES& pref,
					IGNORE_LIST& ignore, SCHEDULE_ADD_LIST& add)
{
	FILE *fp;
	long i, j;
	long pos;
	SCHEDULE temp;

	_getcwd(working_dir,1024);

	if ((fp = fopen(pref_file,"rb")) != NULL)
	{
		pref_mutex.Lock();
		server_mutex.Lock();
		fread(&pref,sizeof(pref),1,fp);
		///pref.database = false;
		if (pref.database)
		{
			/*
			
			for (i = 0; i < pref.num_entries && !feof(fp); i++)
			{
				fread(&temp, sizeof(SCHEDULE), 1, fp);
			}
			fclose(fp);
			*/
			pos = ftell(fp);
			fclose(fp);
			if (dbase == NULL)
				dbase = new Database(pref);

			dbase->LoadPreferences(pref);
			CopySchedule(pref);

		

			ignore.ignore_entry = new IGNORE_MP[MAX_SCHEDULE_ENTRIES];

			memset(ignore.ignore_entry, 0, sizeof(IGNORE_MP) * MAX_SCHEDULE_ENTRIES);

			
			add.schedule_entry = new SCHEDULE_ADD[MAX_SCHEDULE_ENTRIES];
			memset(&add.schedule_entry[0], 0, sizeof(SCHEDULE_ADD) * MAX_SCHEDULE_ENTRIES);
			/*
			fp = fopen(pref_file, "rb");
			if (fp == NULL) return 0;
			fseek(fp, pos, SEEK_SET);
			*/
		}
		else
		{
			if (pref.num_entries > MAX_SCHEDULE_ENTRIES)
				while (pref.num_entries > MAX_SCHEDULE_ENTRIES)
					MAX_SCHEDULE_ENTRIES *= 2;

			pref.schedule_entry = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
			for (i = 0; i < pref.num_entries && !feof(fp); i++)
			{
				fread(&pref.schedule_entry[i], sizeof(SCHEDULE), 1, fp);
				pref.schedule_entry[i].status = 0;
				pref.schedule_entry[i].thread_ptr = NULL;
				pref.schedule_entry[i].stream_running = FALSE;
				pref.schedule_entry[i].stream_idx = -1;
				pref.schedule_entry[i].visible = 1;
				pref.schedule_entry[i].recorded = FALSE;
			}



			if (!feof(fp))
				fread(&ignore, sizeof(ignore), 1, fp);
			ignore.ignore_entry = new IGNORE_MP[MAX_SCHEDULE_ENTRIES];
			for (i = 0; i < ignore.num_entries && !feof(fp); i++)
			{
				fread(&ignore.ignore_entry[i], sizeof(IGNORE_MP), 1, fp);
			}

			for (i = 0; i < MAX_SCHEDULE_ENTRIES; i++)
			{
				pref.schedule_entry[i].thread_ptr = NULL;
				pref.schedule_entry[i].stream_running = FALSE;
				if (i > pref.num_entries)
					pref.schedule_entry[i].stream_idx = -2;
				memset(pref.schedule_entry[i].oldbuf, 0, BUFFERSIZE);
			}
			if (!feof(fp))
			{
				fread(&add, sizeof(add), 1, fp);
				j = add.num_entries;
			}
			else
			{
				j = add.num_entries = pref.num_entries;
			}
			add.schedule_entry = new SCHEDULE_ADD[MAX_SCHEDULE_ENTRIES];
			for (i = 0; i < j && i < MAX_SCHEDULE_ENTRIES && !feof(fp); i++)
			{
				fread(&add.schedule_entry[i], sizeof(SCHEDULE_ADD), 1, fp);
			}

			/*
			for (i = 0; i < j && !feof(fp); i++)
			{
				fread(&add.schedule_entry[i],sizeof(SCHEDULE_ADD),1,fp);
			}
			*/

			if (add.num_entries < pref.num_entries)
				add.num_entries = pref.num_entries;
			while (i < add.num_entries && i < MAX_SCHEDULE_ENTRIES)
			{
				memset(&add.schedule_entry[i++], 0, sizeof(SCHEDULE_ADD));
			}
			server_mutex.Unlock();
			pref_mutex.Unlock();
			fclose(fp);
		}
		
	}
	else
	{
		pref_mutex.Lock();
		pref.schedule_entry = new SCHEDULE[MAX_SCHEDULE_ENTRIES];
		ignore.ignore_entry = new IGNORE_MP[MAX_SCHEDULE_ENTRIES];
		add.schedule_entry = new SCHEDULE_ADD[MAX_SCHEDULE_ENTRIES];
		memset(&pref.schedule_entry[0],0,sizeof(SCHEDULE)* MAX_SCHEDULE_ENTRIES);
		memset(&ignore.ignore_entry[0],0,sizeof(IGNORE_MP)* MAX_SCHEDULE_ENTRIES);
		memset(&add.schedule_entry[0],0,sizeof(SCHEDULE_ADD)* MAX_SCHEDULE_ENTRIES);
		pref_mutex.Unlock();
		return 0;
	}

	return 1; // Success! 
}
//--------------------------------------------
// SavePreferences
// Function saves the preferences to disk
// If database is enabled, it saves the
// database
// PARAMS: pref_file (array of chars),
// pref (preferences), ignore (IGNORE_LIST),
// add (SCHEDULE_ADD_LIST)
// RETURNS: 1 if successful, 0 otherwise
//--------------------------------------------
int SavePreferences(const char *pref_file, 
					const STREAMRECORD_PREFERENCES& pref,
					const IGNORE_LIST& ignore,
					const SCHEDULE_ADD_LIST& add,
					bool database)
{
	FILE *fp;
	long i;

	_chdir(working_dir);
	if (pref.database && database)
	{
		if (dbase == NULL)
			dbase = new Database(pref);

		dbase->SavePreferences(pref);
	}

	if ((fp = fopen(pref_file,"wb")) != NULL)
	{
		pref_mutex.Lock();
		fwrite(&pref,sizeof(pref),1,fp);
		for (i = 0; i < pref.num_entries; i++)
		{
			fwrite(&pref.schedule_entry[i],sizeof(SCHEDULE),1,fp);
		}
		fwrite(&ignore,sizeof(ignore),1,fp);
		for (i = 0; i < ignore.num_entries; i++)
		{
			fwrite(&ignore.ignore_entry[i],sizeof(IGNORE_MP),1,fp);
		}
		fwrite(&add,sizeof(add),1,fp);
		for (i = 0; i < add.num_entries && i < MAX_SCHEDULE_ENTRIES; i++)
		{
			fwrite(&add.schedule_entry[i],sizeof(SCHEDULE_ADD),1,fp);
		}
		pref_mutex.Unlock();
		fclose(fp);
	}
	else
	{
		return 0;
	}

	return 1; // Success!
}
//-----------------------------------
// GetWorkingDirectory
// Function returns the working
// directory
// PARAMS: None
// RETURNS: The working directory
//--------------------------------------
char * GetWorkingDirectory()
{
	return working_dir;
}
//----------------------------------------------
// ConsolidateSchedule
// Function takes all entries with
// a stream_idx of -2 and deletes them
// Effectively, this deletes all programs that
// [1] have been flagged as one-time recordings 
// and [2] have been recorded
// PARAMS: pref (preferences)
// RETURNS: Nothing; schedule is consolidated
//----------------------------------------------
void ConsolidateSchedule(STREAMRECORD_PREFERENCES *pref)
{
	long i = 0, j;

	pref_mutex.Lock();
	if (pref->num_entries > 0)
	{
		while (i < pref->num_entries)
		{
			if (pref->schedule_entry[i].stream_idx == -2)
			{
				for (j = i+1; j < pref->num_entries; j++)
					pref->schedule_entry[j-1] = pref->schedule_entry[j];
				pref->num_entries--;
			}
			i++;
		}
	}
	pref_mutex.Unlock();
}
//----------------------------------------------
// ConsolidateIgnoreList
// Function takes all ignore entries with a 
// mountpoint_URL of NULL and deletes them
// PARAMS: ignore (IGNORE_LIST)
// RETURNS: Nothing; ignore list is consolidated
//-----------------------------------------------
void ConsolidateIgnoreList(IGNORE_LIST *ignore)
{
	long i = 0, j;

	if (ignore->num_entries > 0)
	{
		while (i < ignore->num_entries)
		{
			if (ignore->ignore_entry[i].mountpoint_URL[0] == NULL)
			{
				for (j = i+1; j < ignore->num_entries; j++)
					ignore->ignore_entry[j-1] = ignore->ignore_entry[j];
				ignore->num_entries--;
			}
			i++;
		}
	}
}
//----------------------------------------------
// ConsolidateAddSchedule
// Function takes all entries with
// a stream_idx of -2 and deletes them
// Effectively, this deletes all programs that
// [1] have been flagged as one-time recordings 
// and [2] have been recorded
// Does the same thing, only for added schedule 
// members
// PARAMS: pref (preferences)
// RETURNS: Nothing; schedule is consolidated
//----------------------------------------------
void ConsolidateAddList(SCHEDULE_ADD_LIST *add)
{
	long i = 0, j;

	if (add->num_entries > 0)
	{
		while (i < add->num_entries)
		{
			if (add->schedule_entry[i].enable_mp_ignore[0] == -2)
			{	
				for (j = i+1; j < add->num_entries; j++)
					add->schedule_entry[j-1] = add->schedule_entry[j];
				add->num_entries--;
			}
			i++;
		}
	}
}
//----------------------------------------------------------------
// CopyString
// Function takes a CString argument and converts it to an array
// of chars
// PARAMS: dest (array of chars), source (CString), maxlen(USHORT)
// (maxlen is optional)
// RETURNS: Nothing; source is copied to dest
//-----------------------------------------------------------------
void CopyString(char dest[], CString source, unsigned short maxlen)
{
	unsigned long i;

	for (i = 0; i < source.GetLength() && i < maxlen; i++)
		dest[i] = source.GetAt(i);
	if (i < maxlen)
		dest[source.GetLength()] = NULL;
	else
		dest[maxlen - 1] = NULL;
}
//----------------------------------------------------------------
// CopyString
// Function takes an array of chars and converts it to CString 
// variable
// PARAMS: dest (CString), source (array of chars), maxlen(USHORT)
// (maxlen is optional)
// RETURNS: Nothing; source is copied to dest
//----------------------------------------------------------------
void CopyString(CString& dest, char source[], unsigned short maxlen)
{
	unsigned long i;

	dest = "";
	for (i = 0; i < strlen(source) && i < maxlen; i++)
		dest += source[i];
}
//----------------------------------------------------------------
// ConvertString 
// Function takes a char array and converts it to a wchar array
// (Needed for mimizing to systray)
// PARAMS: conv_str (array of wchar), str (array of char),
// maxlen (unsigned short)
// RETURNS: Nothing; string is converted to array of wchars
//----------------------------------------------------------------
void ConvertString(wchar_t conv_str[], char str[], unsigned short maxlen)
{
	unsigned long i;

	for (i = 0; i < strlen(str) && i < maxlen; i++)
		conv_str[i] = (wchar_t)str[i];
	if (i < maxlen)
		conv_str[strlen(str)] = NULL;
	else
		conv_str[maxlen - 1] = NULL;
}
//----------------------------------------------------------------
// ConvertString 
// Function takes a wchar array and converts it to a char array
// (Needed for mimizing to systray)
// PARAMS: conv_str (array of wchar), str (array of char),
// maxlen (unsigned short)
// RETURNS: Nothing; string is converted to array of wchars
//----------------------------------------------------------------

void ConvertString(char conv_str[], wchar_t str[], unsigned short maxlen)
{
	unsigned long i;

	for (i = 0; str[i] != NULL && i < maxlen; i++)
		conv_str[i] = (char)str[i];
	if (i < maxlen)
		conv_str[i] = NULL;
	else
		conv_str[maxlen - 1] = NULL;
}
//----------------------------------------------------------------
// LoadDatabaseA
// Function takes a preferences variable and invokes 
// Database::LoadPreferences
// PARAMS: pref (STREAMRECORD_PREFERENCES)
// RETURNS: Nothing; Database::LoadPreferences is invoked
//----------------------------------------------------------------
void LoadDatabaseA(STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);

	
	dbase->LoadPreferences(pref);
}
//----------------------------------------------------------------
// SaveDatabase
// Function takes a preferences varaible, a thread variable, and an 
// integer, and spawns a thread to save the database or invokes
// Database::SavePreferences 
// PARAMS: pref (STREAMRECORD_PREFERENCES), thread (bool), n (int)
// RETURNS: Nothing
//-----------------------------------------------------------------
void SaveDatabase(const STREAMRECORD_PREFERENCES& pref, bool thread, int n, bool prune)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	
	if (thread)
	{
		db_params.dbptr = dbase;
		db_params.pref = (STREAMRECORD_PREFERENCES *)&pref;
		db_params.n = n;
		db_params.prune = prune;
		if (n == -1)
			AfxBeginThread(SaveThread, (LPVOID)&db_params, THREAD_PRIORITY_LOWEST);
		else
			AfxBeginThread(SaveThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	}
	else
	{
		dbase->SavePreferences(pref, n);
		//	Sleep(100);
	}
}
//----------------------------------------------------------------
// DeleteDatabase
// Function takes an integer representing the ID and invokes
// DeletePreferences, a function that deletes an entry from the 
// database
// PARAMS: id (const int), pref (STREAMRECORD_PREFERENCES)
// RETURNS: Nothing; Database::DeletePreferences is invoked
//-----------------------------------------------------------------
void DeleteDatabase(const int id, const STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	
	Sleep(2000);
	dbase->DeletePreferences(id);
	Sleep(1000);
	
	//db_params.dbptr = dbase;
	//db_params.n = id;
	//AfxBeginThread(DeleteDatabaseThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	
}
//----------------------------------------------------------------
// LoadDatabase
// Function takes a preferences variable and spawns a thread
// To load the database into a temporary variable
// PARAMS: pref (STREAMRECORD_PREFRENCES)
// RETURNS: Nothing; thread is spawned
//-----------------------------------------------------------------
void LoadDatabase(STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
	{
		dbase = new Database(pref);
		dbase->LoadPreferences(pref);
	}
	else
	{
		db_params.dbptr = dbase;
		db_params.pref = &pref;

		AfxBeginThread(LoadDatabaseThread, (LPVOID)&db_params, THREAD_PRIORITY_BELOW_NORMAL);
	}
}
//----------------------------------------------------------------
// CopySchedule
// Function takes a preferences pointer and spawns a thread
// to copy the data from a temporary variable to the main
// preferences variable
// PARAMS: pref (STREAMRECORD_PREFERENCES)
// RETURNS: Nothing; CopyScheduleThread is invoked
//-----------------------------------------------------------------
void CopySchedule(STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	//dbase->CopySchedule(pref);

	db_params.dbptr = dbase;
	db_params.pref = &pref;

	AfxBeginThread(CopyScheduleThread, (LPVOID)&db_params, THREAD_PRIORITY_BELOW_NORMAL);
}
void LoadandCopy(STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);
	//dbase->CopySchedule(pref);

	db_params.dbptr = dbase;
	db_params.pref = &pref;

	AfxBeginThread(LoadandCopyThread, (LPVOID)&db_params, THREAD_PRIORITY_BELOW_NORMAL);
}
//-------------------------------------------------------------
// ResetDatabase
// Function takes a preferences pointer and resets the
// connection
// PARAMS: pref (STREAMRECORD_PREFERENCES object)
// RETURNS: Nothing; connection is reset
//-------------------------------------------------------------
void ResetDatabase(const STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	dbase->ResetConnection(pref);
}
//-----------------------------------------------------------------
// ResetStatus
// Function takes a preferences object and whether or not to flip 
// everything to "Queued" and spawns a thread to reset Recording 
void ResetStatus(const STREAMRECORD_PREFERENCES& pref, bool flip)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	
	
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES *)&pref;
	db_params.flip = flip;
	
	
	AfxBeginThread(ResetStatusThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	///if (!dbase->ResetStatus(pref))
	//	Sleep(1000);
}
void SetStatus(const STREAMRECORD_PREFERENCES& pref, int n,char msg[])
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES *)&pref;
	db_params.n = n;

	
	
	AfxBeginThread(SetStatusThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	//if (!dbase->SetStatus(pref, n))
	//	Sleep(1000);

	if (pref.pushover && msg != NULL)
		PushMessage(&pref, msg);
}

void ResetConnection(const STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES *)&pref;

	AfxBeginThread(ResetConnectionThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	//

	//if (!dbase->ResetConnection(pref))
	//	Sleep(1000);
}

bool ResetTable(const STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return false;
	if (dbase == NULL)
		dbase = new Database(pref);
	return dbase->ResetTable(pref);
}
bool SetStatus(const STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return false;
	if (dbase == NULL)
		dbase = new Database(pref);
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES*)&pref;
	////while (!SetStatus(pref))
	///	Sleep(1000);
	

	AfxBeginThread(SetStatusOnlyThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	return true;
}
void PushMessage(const STREAMRECORD_PREFERENCES *pref, char msg[])
{
	if (!pref->pushover)
		return;
	if (push == NULL)
		push = new Pushover(pref);

	db_params.poptr = push;
	
	strcpy(db_params.msg, msg);
	////db_params.pref = (STREAMRECORD_PREFERENCES*)&pref;
	AfxBeginThread(PushMessageThread, (LPVOID)&db_params, THREAD_PRIORITY_LOWEST); // HIGHEST);


	//push->PushMessage(msg);
}

void PruneSchedule(const STREAMRECORD_PREFERENCES& pref)
{
	if (!pref.database)
		return;
	if (dbase == NULL)
		dbase = new Database(pref);
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES*)&pref;
	STREAMRECORD_PREFERENCES* ppref = (STREAMRECORD_PREFERENCES *)&pref;

	//dbase->PruneSchedule(pref);
	//AfxBeginThread(PruneThread, (LPVOID)&db_params, THREAD_PRIORITY_LOWEST);
	

	AfxBeginThread(PruneThread, (LPVOID)&db_params, THREAD_PRIORITY_BELOW_NORMAL);
}