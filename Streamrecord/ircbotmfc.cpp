/*
 * IrcBotMFC.cpp
 *
 *  Created on: 4-22-2015
 *      Author: David Zientara
 */

#include "stdafx.h"
#include "ircbotmfc.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
///#include <unistd.h>
#include <io.h>
#include <errno.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <signal.h>
#include <time.h>
#include "StreamInstance.h"

#include <afx.h>
#include <afxext.h>
#include <afxsock.h>
#include <iostream>
 
using namespace std;
 
#define MAXDATASIZE 100
#define MAXRINGO	30
#define TEXTCOLOR	"04"
#define IRCPORT		6667
#define MAXBUFSIZE	1024

static char ringoism[][80] = { "Oh heavens\r\n", "Play me tunes\r\n",
"I'm not Francis!\r\n", "What year is this, six?\r\n", "Hi sex\r\n",
"Bruce says hi\r\n", "stop it, blair, or budda will bite you in the ass\r\n",
"Jeff, Irving wants you to buy him a bus ticket\r\n", "Jeff, can I be in your will?\r\n",
"I can't believe they turned gary's house into a national landmark\r\n",
"tune into my show, everyone\r\n", "it's so good to be calling local\r\n",
"Jeff, how is Bommie?\r\n", "did you listen to my show?\r\n",
"I need Pippin to mix for me\r\n", "Jeff, can I have the Lucy TV Guide?\r\n",
"I had prime rib for dinner\r\n", "this is going to wake up the help\r\n",
"blair, do you have any pictures of the tuna?\r\n", "Bruce is in the village tonight\r\n",
"I think I'm going to do Karen Carpenter again\r\n", "I need release\r\n",
"kiss Bommie for me, Jeff\r\n", "bruce luvs byron's greeting\r\n",
"I think the only one worse than shatner is weird al\r\n",
"Stop taht, balir\r\n", "Was that shave close, Ron?\r\n",
"Heavens to bentley\r\n", "sff is not my type...I mean not Bruce's type\r\n",
"I wonder if Pippin is a screamer\r\n"
};

IrcBotMFC::IrcBotMFC(char * _nick, char * _usr)
{
	strcpy(cur_nickname,_nick);
	strcpy(cur_UID,_usr);
	srand(time(NULL));
}

IrcBotMFC::IrcBotMFC(const char server[], const char channel[], 
			   const char nickname[], const char UID[], 
			   const char color, const char background)
{
	strcpy(cur_server,server);
	strcpy(cur_channel,channel);
	strcpy(cur_nickname,nickname);
	strcpy(cur_UID,UID);
	cur_color = color;
	cur_background = background;
	srand(time(NULL));
}

IrcBotMFC::~IrcBotMFC()
{
    //close(s);
	irc_socket.Close();
}

void IrcBotMFC::CopyParams(const char server[], const char channel[], 
			   const char nickname[], const char UID[], 
			   const char color, const char background)
{
	strcpy(cur_server,server);
	strcpy(cur_channel,channel);
	strcpy(cur_nickname,nickname);
	strcpy(cur_UID,UID);
	cur_color = color;
	cur_background = background;
}

bool IrcBotMFC::Start()
{
    struct addrinfo hints, *servinfo;
	int iResult;
	WSADATA wsaData;
	int res;
	//Recv some data
    int numbytes = 1;
    char buf[MAXDATASIZE];
	char cmd[MAXDATASIZE];
	char cnick[MAXDATASIZE*2];
	char error_count = 0;
    int count = 0;
 
	strcpy(cnick,cur_nickname);

	if (AfxSocketInit() == FALSE)
	{
		MessageBoxA(NULL, LPCSTR("Failed to initialize socket."), PROGRAM_NAME, MB_OK);
		return false;
	}
	irc_socket.Create();
	irc_socket.Connect(LPCTSTR(cur_server),IRCPORT);

	do 
	{
		if (count == 3 && numbytes <= 0 && error_count++ < 3)
			strcat(cnick,"_");
		else
			count++;

		switch (count) {
		        case 3:
			        //after 3 recives send data to server (as per IRC protacol)
					sprintf(cmd,"NICK %s\r\n",cnick);
					irc_socket.Send(cmd,strlen(cmd));
					sprintf(cmd,"USER %s alt1 alt2 :%s\r\n",cur_UID,PROGRAM_NAME);
			        irc_socket.Send(cmd,strlen(cmd));
				    break;
	            case 4:
		            //Join a channel after we connect, this time we choose beaker
					sprintf(cmd,"JOIN %s\r\n",cur_channel);
					irc_socket.Send(cmd,strlen(cmd));
					break;
		        default:
			        break;
			}
		numbytes = irc_socket.Receive(buf,MAXDATASIZE-1);

	} while (numbytes != 0 && count < 4 && error_count < 3);

	if (numbytes >= 0)
		buf[numbytes]='\0';

	return true;
}

bool IrcBotMFC::Stop()
{
	char buf[MAXDATASIZE];
	long numbytes = 0;

	strcpy(buf,"QUIT\r\n");
	irc_socket.Send(buf,strlen(buf));

	do 
	{
		numbytes = irc_socket.Receive(buf,MAXDATASIZE-1);
		buf[numbytes]='\0';
	} while (numbytes > 0);

	if (numbytes > 0)
		return false;

	return true;
}

bool IrcBotMFC::SendMessage(const char msg[])
{
	char str[1024];
	char colorinfo[8], colormsg[1024];

	colorinfo[0] = 0x03;
	colorinfo[1] = '0';
	colorinfo[2] = cur_color|0x30;
	colorinfo[3] = NULL;
	//colorinfo[4] = cur_background|0x30;
	//colorinfo[5] = NULL;
	strcpy(colormsg,colorinfo);
	strncat(colormsg,msg,1016);
	sprintf(str,"PRIVMSG %s :%s\r\n",cur_channel,colormsg);
	return (bool)irc_socket.Send(str,strlen(str));   //sendData(str);
}

bool IrcBotMFC::SetTopic(const char topic[])
{
	char str[1024];

	sprintf(str,"TOPIC %s :%s\r\n",cur_channel,topic);
	return (bool)(irc_socket.Send(str,strlen(str)));
}

long IrcBotMFC::GetOutput(char buf[])
{
	long numbytes = 0;

	numbytes = irc_socket.Receive(buf,MAXDATASIZE-1);
	buf[numbytes] = NULL;
	if (numbytes <= 0)
		Restart();
	return numbytes;
}

bool IrcBotMFC::Send(const char msg[])
{
	bool retval = irc_socket.Send(msg,strlen(msg));
	if (!retval)
		Restart();
	return retval;
}

bool IrcBotMFC::Ringo()
{
	char str[1024];

	strcpy(str,ringoism[rand()%MAXRINGO]);
	return irc_socket.Send(str,strlen(str));
}
 
bool IrcBotMFC::charSearch(char *toSearch, char *searchFor)
{
    int len = strlen(toSearch);
    int forLen = strlen(searchFor); // The length of the searchfor field
 
    //Search through each char in toSearch
    for (int i = 0; i < len;i++)
    {
        //If the active char is equil to the first search item then search toSearch
        if (searchFor[0] == toSearch[i])
        {
            bool found = true;
            //search the char array for search field
            for (int x = 1; x < forLen; x++)
            {
                if (toSearch[i+x]!=searchFor[x])
                {
                    found = false;
                }
            }
 
            //if found return true;
            if (found == true)
                return true;
        }
    }
 
    return 0;
}
 
bool IrcBotMFC::isConnected(char *buf)
{//returns true if "/MOTD" is found in the input string
    //If we find /MOTD then its ok join a channel
    if (charSearch(buf,"/MOTD") == true)
        return true;
    else
        return false;
}
 
char * IrcBotMFC::timeNow()
{//returns the current date and time
    time_t rawtime;
    struct tm * timeinfo;
 
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
 
    return asctime (timeinfo);
}


bool IrcBotMFC::Restart()
{
	irc_socket.Close();
	return Start();
}