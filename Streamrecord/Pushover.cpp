#include "stdafx.h"
#include "loadpref.h"
#include "Pushover.h"
#include "Logger.h"
#include <afxmt.h>

#define TOKEN	"asbbq4d9ffc2pkut8moi3ati3qoga4"
#define USER	"uRNgDtixLQdTSaXe5tWgFPeMKH2K97"

Pushover::Pushover(const STREAMRECORD_PREFERENCES *pref)
{
	ppref = (STREAMRECORD_PREFERENCES *)pref;

}
void Pushover::PushMessage(char msg[])
{
	char mycommand[1024];

	sprintf(mycommand, "curl -s -F \"token=%s\" -F \"user=%s\" -F \"title=Six Stream Recorder\" -F \"message=%s\" https://api.pushover.net/1/messages.json", TOKEN, USER, msg);

	system(mycommand);

	Sleep(500);
	
}