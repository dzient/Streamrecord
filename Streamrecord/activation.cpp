#include "stdafx.h"
#include <stdio.h>
#include "activation.h"

bool check_activation_code(const char ac[])
{
	long i, total = 0;
	char temp[3], ar[11];
	const long DIVISOR = 23;

	if (strlen(ac) < 16)
		return false;

	strncpy(ar,ac,5);
	strncpy(ar+5,ac+11,5);
	ar[10] = NULL;

	for (i = 0; i < 5; i++)
	{
		if (i != 4)
		{
			strncpy(temp,ar+i*2,2);
			temp[2] = NULL;
			total += atoi(temp);
		}
	}

	return !(total%DIVISOR);
}