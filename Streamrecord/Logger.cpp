#include "stdafx.h"
#include "loadpref.h"
#include "Logger.h"
#include "StreamInstance.h"


Logger::Logger(char filename[])
{
	char fn[1024];
	fp = NULL;

	if (filename != NULL)
		strcpy(fn, filename);
	else
		strcpy(fn, LOG_FILE);

	if ((fp = fopen(filename, "a")) == NULL)
		MessageBoxA(NULL, (char*)LPCSTR("Cannot open log file."), PROGRAM_NAME, MB_OK | MB_ICONEXCLAMATION);
}
bool Logger::Log(const char message[])
{
	if (fp != NULL)
	{
		fprintf(fp, "%s\n", message);
		return true;
	}
	return false;
}

Logger::~Logger()
{
	if (fp != NULL)
		fclose(fp);
}
