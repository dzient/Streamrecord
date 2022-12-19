//-------------------------------------------
// David Zientara
// 12-19-2022
//
// Pushover.cpp
//
// File for the Pushover class
//
//---------------------------------------------


#include "stdafx.h"
#include "loadpref.h"
#include "Pushover.h"
#include "Logger.h"
#include <afxmt.h>
#include <iostream>
#include <string>
#include "curl-7.86.0/include/curl/curl.h"

#define TOKEN	"asbbq4d9ffc2pkut8moi3ati3qoga4"
#define USER	"uRNgDtixLQdTSaXe5tWgFPeMKH2K97"


//--------------------------------------------------------
// Pushover 
// Constructor for the Pushover class
// PARAMS: pref (pointer to STREAMRECORD_PREFERENCES)
// RETURNS: Nothing; Pushover object initialized
//--------------------------------------------------------

Pushover::Pushover(const STREAMRECORD_PREFERENCES *ppref)
{
	pref = (STREAMRECORD_PREFERENCES *)ppref;

}
//----------------------------------------------------------
// PushMessage
// Function takes a message and pushes it to devices
// PARAMS: msg (array of chars)
// RETURNS: Nothing; message is pushed out
//------------------------------------------------------------
void Pushover::PushMessage(char msg[])
{
	char mycommand[1024];
	CURL* curl;
	CURLcode res;
	std::string readBuffer;
	std::string apiKey = pref->api_key;
	std::string apiUser = pref->api_user;
	std::string message = msg;
	std::string url = "https://api.pushover.net/1/messages.json";
	std::string jsonString = "{\" token=" + apiKey + "\ user=" + apiUser + "\ title=Six Stream Recorder \" message =" + message;

	//Put everything in a char array and invoke system, which in turn invokes curl:
	sprintf(mycommand, "curl -s -F \"token=%s\" -F \"user=%s\" -F \"title=Six Stream Recorder\" -F \"message=%s\" https://api.pushover.net/1/messages.json", 
		pref->api_key, pref->api_user, msg);

	system(mycommand);

	//curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // mycommand);
	//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
	//curl_easy_setopt(curl,CURLOPT_P)

	//Sleep(500);
	
}