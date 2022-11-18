#pragma once

#include "stdafx.h"
#include "loadpref.h"

#ifndef _PUSHOVER_H
#define _PUSHOVER_H

class Pushover
{
public:
	Pushover(const STREAMRECORD_PREFERENCES *pref);
	void PushMessage(char msg[]);
private:
	STREAMRECORD_PREFERENCES* ppref;
};
#endif // !_PUSHOVER_H
