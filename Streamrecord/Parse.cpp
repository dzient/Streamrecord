#include "stdafx.h"
#include "Parse.h"
#include <direct.h>

void ParseFilename(char filename[])
{
	unsigned long i, j;
	const char forbidden[] = "\\/:*\"<>|?";

	if (strlen(filename) > 0)
	{
		for (i = 0; i < strlen(filename); i++)
		{
			for (j = 0; j < strlen(forbidden); j++)
				if (filename[i] == forbidden[j])
					filename[i] = ' ';
		}
		i--;
		while (isspace(filename[i]))
			i--;
		filename[i+1] = NULL;
	}
}

bool ParseDirectory(const char path[])
{
	char temp[1024];
	long i;

	i = 0;
	do
	{
		while (path[i] != NULL && path[i] != '\\')
			i++;
		if (path[i] != NULL)
		{
			strncpy(temp,path,i+1);
			temp[i+1] = NULL;
			if (_chdir(temp) != 0)
				if (_mkdir(temp) != 0)
					return false;
		}
	} while (path[i++] != NULL);

	return true;	
}