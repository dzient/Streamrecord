#pragma once
#include "stdafx.h"
#include "loadpref.h"

#include "sql\include\jdbc\cppconn\driver.h"
#include "sql\include\jdbc\cppconn\exception.h"
#include "sql\include\jdbc\cppconn\resultset.h"
#include "sql\include\jdbc\cppconn\statement.h"


#ifndef _DATABASE_H
#define _DATABASE_H

class Database
{
public:
	Database(const STREAMRECORD_PREFERENCES& pref);
	bool LoadPreferencesA(STREAMRECORD_PREFERENCES& pref);
	bool SavePreferences(const STREAMRECORD_PREFERENCES& pref,int n=-1,bool prune=false);
	bool DeletePreferences(const int id);
	bool LoadPreferences(STREAMRECORD_PREFERENCES& pref);
	bool CopySchedule(STREAMRECORD_PREFERENCES& pref);
	bool ResetConnection(const STREAMRECORD_PREFERENCES& pref);
	bool ResetStatus(const STREAMRECORD_PREFERENCES& pref, bool flip=false);
	bool SetStatus(const STREAMRECORD_PREFERENCES& pref,int n);
	bool SetStatus(const STREAMRECORD_PREFERENCES& pref);
	bool ResetTable(const STREAMRECORD_PREFERENCES& pref);
	bool PruneSchedule(const STREAMRECORD_PREFERENCES& pref);
	void LogError(sql::SQLException& e);
	bool Reset();
	~Database();
private:
	sql::Driver* driver;
	sql::Connection* con;
	sql::Statement* stmt;
	sql::ResultSet* res;
	STREAMRECORD_PREFERENCES* ppref;
	STREAMRECORD_PREFERENCES* temp;
	SCHEDULE *tsched;
	bool db_updated;
	bool pinit;
	int prev;
	int max_id;
	CWinThread** temp_ptr;
	long* temp_streams;
	BOOL* temp_recorded, *temp_stream_running;
	char willpurge[1024];
	

};

#endif