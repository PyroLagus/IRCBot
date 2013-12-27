#include "IRCBot.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <errno.h>
#include <string.h>
/*
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
*/

using namespace std;

IrcBot::IrcBot(const string &host, const int &port, const list<string> &channels, const string &nick, const string &usr, const string &owner)
{
	this->net_client.conn(host, port);

	this->quit = false;
	this->error = false;
	this->connected = this->joined = false;

	this->channels = channels;
	this->nick = nick;
	this->usr = usr;
	this->my_owner = owner;

	this->nick_command = "NICK " + nick + "\r\n";
	this->usr_command = "USER " + usr + " " + usr + " " + usr + " :" + usr + "\r\n";

	for(list<string>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		this->join_command += "JOIN " + *it + "\r\n";
	}
}

IrcBot::~IrcBot()
{

}

void IrcBot::start()
{
	string buffer;	//buffer is the data that is recived

	while (true)
	{
		buffer = net_client.receive(1024);
		
		//loop through buffer and process the lines seperately
		for(string::size_type firstPos = 0, lastPos = 0; lastPos < buffer.length(); lastPos++)
		{
			firstPos = lastPos;
			lastPos = buffer.find("\n", lastPos);

			if(lastPos > buffer.size())
				break;	//break before msgHandle is called
			
			string bufferSubstr = buffer.substr(firstPos, lastPos);
			cout << ">> " << bufferSubstr << endl;
			msgHandle(bufferSubstr, getChannel(bufferSubstr), getNick(bufferSubstr));
		}

		//break if quit
		if(quit)
		{
			cout << "-------------------------------QUIT------------------------------"<< endl;
			cout << timeNow() << endl;
			break;
		}
		//break if error
		if(error)
		{
			cout << "-------------------------------ERROR------------------------------"<< endl;
			cout << timeNow() << endl;
			break;
		}
		//break if connection closed
		if(buffer.empty())
		{
			cout << "----------------------CONNECTION CLOSED---------------------------"<< endl;
			cout << timeNow() << endl;
		
			break;
		}
	}
}

char * IrcBot::timeNow()
{//returns the current date and time
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	return asctime (timeinfo);
}

string IrcBot::getNick(const string &buf)
{
	return buf.substr(1, buf.find("!")-1);
}

string IrcBot::getChannel(const string &buf)
{
	size_t channelNamePos = buf.find("PRIVMSG #")+8;
	return buf.substr(channelNamePos, buf.find(":", channelNamePos)-channelNamePos-1);
}

void IrcBot::sendMessage(const string &msg, const string &channel)
{
	net_client.send_data("PRIVMSG " + channel + " :" + msg + "\r\n");
}

string IrcBot::getArgument(const string &buf, const string &command)
{
	size_t commandPos = buf.find(command);

	size_t argumentPos = buf.find_first_not_of(command, commandPos)+1;
                
	string argument = buf.substr(argumentPos);

	size_t found = argument.find_last_not_of(" \r\n");
                if (found != string::npos)
                        argument.erase(found+1);
                else argument.clear();

	return argument;
}

list<string> IrcBot::getArguments(const string &buf, const string &command)
{
}

void IrcBot::msgHandle(const string &buf, const string &msgChannel, const string &msgNick)
{
	if (buf.find("Found your hostname") != string::npos && !connected)
	{	
        	net_client.send_data(nick_command);
        	net_client.send_data(usr_command);
        	cout << nick_command << usr_command;

		connected = true;
		return;
	}
	if (buf.find("/MOTD") != string::npos && !joined)
        {
		net_client.send_data(join_command);

		joined = true;
		return;
	}
	if (buf.find(":Nickname is already in use.") != string::npos && buf.find("!") == string::npos)
	{
		cout << "Nickname already in use" << endl;
		error = true;
		return;
	}

	if (buf.substr(0, 6).find("PING :") != string::npos)
	{
		net_client.send_data("PONG :" + buf.substr(6));
		cout << "PONG :" + buf.substr(6);
		return;
	}

        if (buf.find("huhu " + nick) != string::npos)
        {
		sendMessage("huhu " + msgNick, msgChannel);
                return;
        }
	if (buf.find("!help") != string::npos)
	{
		sendMessage("No help from me.", msgChannel);
		return;
	}
	if (buf.find("!dice") != string::npos)
	{
		sendMessage(std::to_string(rand() % 6 + 1), msgChannel);
		return;
	}
	if (buf.find("!cookie") != string::npos)
	{
		string argNick = getArgument(buf, "!cookie");
		if(argNick.empty())
			argNick = msgNick;

		sendMessage(string("\x01") + "ACTION gives " + argNick + " a cookie" + "\x01", msgChannel);
		return;
	}
	if (buf.find("!channels") != string::npos)
	{
		string message;
		for(list<string>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			message += " " + *it;
		}
		sendMessage(message.erase(0,1), msgChannel);
		return;
	}
	if (buf.find("!channel") != string::npos)
	{
		sendMessage(msgChannel, msgChannel);
		return;
	}
	if (buf.find("!topic") != string::npos)
	{
		if(msgNick[0] != '@')
		{
			sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
			return;
		}
		string topic = getArgument(buf, "!topic");
		net_client.send_data("TOPIC " + msgChannel + " :" + topic + "\r\n");
		return;
	}
	if (buf.find("!nowplaying") != string::npos)
	{
		sendMessage("!np", msgChannel);
		return;
	}
	if (buf.find("!time") != string::npos)
	{
		sendMessage(timeNow(), msgChannel);
		return;
	}
	if (buf.find("!names") != string::npos)
	{
		string channelName = getArgument(buf, "!names");

		if(channelName.empty() || channelName == "#")
			channelName = msgChannel;
		if(channelName[0] != '#')
                {
                        sendMessage("Channel name has to start with a #", msgChannel);
                        return;
                }
                if(!checkWhitelist(channelName, channelWhitelist))
                {
                        sendMessage("Channel name contains invalid characters.", msgChannel);
                        return;
                }
		
		net_client.send_data("NAMES " + channelName + "\r\n");

		//awaiting_names = msgChannel;

		return;

	}
	if (buf.find("!namemap") != string::npos)
	{
		if(nicks.find(msgChannel) != nicks.end())
		{
			sendMessage("map: " + nicks.find(msgChannel)->second, msgChannel);
		}
		else
		{
			sendMessage("sorry ._.", msgChannel);
		}
		return;
	}
	if (buf.find("353 " + nick) != string::npos && false)
	{
		string names = buf.substr(buf.find_last_of(":")+1);
		size_t channelNamePos = buf.find("#");
		string channel_string = buf.substr(channelNamePos, buf.find_last_of(":")-channelNamePos-1);

		size_t found = channel_string.find_last_not_of(" \r\n");
		if (found != string::npos)
               		channel_string.erase(found+1);
        	else channel_string.clear();

		nicks.insert( make_pair(channel_string, names) );

		return;
	}
	if (buf.find("366 " + nick) != string::npos && !awaiting_names.empty())
	{
		sendMessage("No NAMES available.", awaiting_names);
		awaiting_names.clear();
		return;
	}
	
	if (buf.find("!join") != string::npos && msgNick == owner)
	{	
		string channelName = getArgument(buf, "!join");
		if(channelName.empty() || channelName == "#")
		{
			sendMessage("No channel specified.", msgChannel);
			return;
		}
		if(channelName[0] != '#')
		{
			sendMessage("Channel name has to start with a #", msgChannel);
			return;
		}
		if(!checkWhitelist(channelName, channelWhitelist))
		{
			sendMessage("Channel name contains invalid characters.", msgChannel);
			return;
		}

		sendMessage("Joining " + channelName, msgChannel);
		net_client.send_data("JOIN " + channelName + "\r\n");
		channels.push_back(channelName);
		return;
	}
	if (buf.find("!leave") != string::npos && msgNick == owner)
	{
		sendMessage("Leaving", msgChannel);
		net_client.send_data("PART " + msgChannel + "\r\n");
		channels.remove(msgChannel);
		return;
	}
	if (buf.find("!quit") != string::npos && msgNick == owner)
	{
		sendMessage("Bye!", msgChannel);
		quit = true;
		return;
	}
	else if (buf.find("!quit") != string::npos)
	{
		sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
	}		

/*	if (buf.find("!bugme") != string::npos)
        {
                bugstring = getNick(buf);
                time( &bugtime );
                sendMessage(getNick(buf) + ": got it!", channel);
                return;
        }       
	if ( bugtime+5 < time(NULL) )
	{
		sendMessage(bugstring + ": hello!!!", channel);
		time( &bugtime );
		return;
	}
*/
}

bool IrcBot::checkWhitelist(const string &buffer, const string &charstring)
{
if (buffer.find_first_not_of(charstring) != std::string::npos)
	return false;
else return true;
}
