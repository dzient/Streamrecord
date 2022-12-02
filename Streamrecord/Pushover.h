#pragma once

#include "stdafx.h"
#include "loadpref.h"

#ifndef _PUSHOVER_H
#define _PUSHOVER_H

class Pushover
{
public:
	Pushover(const STREAMRECORD_PREFERENCES *ppref);
	void PushMessage(char msg[]);
private:
	STREAMRECORD_PREFERENCES* pref;
};
#endif // !_PUSHOVER_H
