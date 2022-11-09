#pragma once
#include "stdafx.h"
#include "loadpref.h"
#include "StreamInstance.h"
#ifndef _LOGGER_H
#define _LOGGER_H
class Logger
{
public:
	Logger(char filename[] = NULL);
	~Logger();
	bool Log(const char message[]);
private:
	FILE* fp;

}; 

#endif
//pragma once
