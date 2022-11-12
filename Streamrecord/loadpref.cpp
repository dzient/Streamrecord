#include "stdafx.h"
#include "loadpref.h"
#include "Database.h"

#include <direct.h>
#include <afxmt.h>



extern CMutex pref_mutex;

extern CMutex server_mutex;

char working_dir[1024];

Database* dbase = NULL;

struct db_struct
{
	Database* dbptr;
	STREAMRECORD_PREFERENCES* pref;
	int n;
} db_params;

UINT SaveThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->SavePreferences(*db->pref,db->n))
		Sleep(100);
	return 1;
}
CMutex LDB_mutex;
UINT CopyScheduleThread(LPVOID db_params)
{
	LDB_mutex.Lock();
	db_struct* db = (db_struct*)db_params;
	while (!db->dbptr->CopySchedule(*db->pref))
		Sleep(100);
	LDB_mutex.Unlock();
	return 1;
}

UINT LoadDatabaseThread(LPVOID db_params)
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
	} while (!rval);
	LDB_mutex.Unlock();
		//Sleep(100);
	return 1;
}
UINT SetStatusThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->SetStatus(*db->pref,db->n))
		Sleep(100);
	return 1;
}
UINT SetStatusOnlyThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->SetStatus(*db->pref))
		Sleep(100);
	return 1;
}

UINT ResetStatusThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->ResetStatus(*db->pref))
		Sleep(100);
	return 1;
}
UINT ResetConnectionThread(LPVOID db_params)
{
	db_struct* db = (db_struct*)db_params;

	while (!db->dbptr->ResetConnection(*db->pref))
		Sleep(100);
	return 1;
}


static wchar_t* charToWChar(const char* text)
{
	const size_t size = strlen(text) + 1;
	wchar_t* wText = new wchar_t[size];
	mbstowcs(wText, text, size);
	return wText;
}

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

char * GetWorkingDirectory()
{
	return working_dir;
}

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

void CopyString(CString& dest, char source[], unsigned short maxlen)
{
	unsigned long i;

	dest = "";
	for (i = 0; i < strlen(source) && i < maxlen; i++)
		dest += source[i];
}

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

void LoadDatabaseA(STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);

	
	dbase->LoadPreferences(pref);
}

void SaveDatabase(const STREAMRECORD_PREFERENCES& pref, bool thread, int n)
{
	////if (n != -1) return;
	if (dbase == NULL)
		dbase = new Database(pref);
	
	if (thread)
	{
		db_params.dbptr = dbase;
		db_params.pref = (STREAMRECORD_PREFERENCES *)&pref;
		db_params.n = n;
		AfxBeginThread(SaveThread, (LPVOID)&db_params, THREAD_PRIORITY_NORMAL);
	}
	else
	{
		dbase->SavePreferences(pref, n);
		//	Sleep(100);
	}
}
void DeleteDatabase(const int id, const STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);
	dbase->DeletePreferences(id);
}

void LoadDatabase(STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
	{
		dbase = new Database(pref);
		dbase->LoadPreferences(pref);
	}
	else
	{
		db_params.dbptr = dbase;
		db_params.pref = &pref;

		AfxBeginThread(LoadDatabaseThread, (LPVOID)&db_params, THREAD_PRIORITY_NORMAL);
	}
}

void CopySchedule(STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);
	//dbase->CopySchedule(pref);

	db_params.dbptr = dbase;
	db_params.pref = &pref;

	AfxBeginThread(CopyScheduleThread, (LPVOID)&db_params, THREAD_PRIORITY_NORMAL);
}

void ResetDatabase(const STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);
	dbase->ResetConnection(pref);
}

void ResetStatus(const STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);
	if (pref.no_load)
		return;
	for (int i = 0; i < pref.num_entries; i++)
		if (pref.schedule_entry[i].thread_ptr == NULL)
			pref.schedule_entry[i].status = 0;
	
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES *)&pref;
	
	///AfxBeginThread(ResetStatusThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	if (!dbase->ResetStatus(pref))
		Sleep(1000);
}
void SetStatus(const STREAMRECORD_PREFERENCES& pref, int n)
{
	if (dbase == NULL)
		dbase = new Database(pref);
	
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES *)&pref;
	
	//AfxBeginThread(SetStatusThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	if (!dbase->SetStatus(pref, n))
		Sleep(1000);
}

void ResetConnection(const STREAMRECORD_PREFERENCES& pref)
{
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
	if (dbase == NULL)
		dbase = new Database(pref);
	return dbase->ResetTable(pref);
}
bool SetStatus(const STREAMRECORD_PREFERENCES& pref)
{
	if (dbase == NULL)
		dbase = new Database(pref);
	db_params.dbptr = dbase;
	db_params.pref = (STREAMRECORD_PREFERENCES*)&pref;
	//while (!SetStatus(pref))
	//	Sleep(1000);

	AfxBeginThread(SetStatusOnlyThread, (LPVOID)&db_params, THREAD_PRIORITY_HIGHEST);
	return true;
}