/*
 * IrcBotMFC.h
 *
 *  Created on: 4-22-2015
 *      Author: David Zientara
 */
#include "stdafx.h"
#include "afxsock.h"
 
#ifndef IRCBOTMFC_H_
#define IRCBOTMFC_H_
 
class IrcBotMFC
{
public:
    IrcBotMFC(char * _nick, char * _usr);
	IrcBotMFC(const char server[], const char channel[], 
		const char nickname[], const char UID[],
		const char color, const char background);
	void CopyParams(const char server[], const char channel[], 
		const char nickname[], const char UID[], 
		const char color, const char background);
	bool SendMessage(const char msg[]);
	bool SetTopic(const char topic[]);
	long GetOutput(char buf[]);
	bool Send(const char msg[]);
	bool Ringo();
	bool Restart();
    virtual ~IrcBotMFC();
 
    bool setup;
 
    bool Start();
	bool Stop();
    bool charSearch(char *toSearch, char *searchFor);
 
private:
    char *port;
    int s; //the socket descriptor
 
    char *nick;
    char *usr;

	char cur_server[32];
	char cur_nickname[16];
	char cur_channel[24];
	char cur_UID[16];
	char cur_color, cur_background;
	CSocket irc_socket;
 
    bool isConnected(char *buf);
    char * timeNow();
    bool sendData(const char msg[]);
    void sendPong(char *buf);
    void msgHandel(char *buf);
};
 
#endif /* IRCBOTMFC_H_ */