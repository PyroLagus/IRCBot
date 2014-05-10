#include "IRCBot.h"
#include <iostream>
#include <cstdio>
#include <sstream>
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

IrcBot::IrcBot(const string &host, const int &port, const list<string> &channels, const string &nick, const string &usr, const string &owner, const string &trigger)
{
	this->net_client.conn(host, port);

	this->quit = false;
	this->error = false;
	this->connected = this->joined = false;

	this->channels = channels;
	this->nick = nick;
	this->usr = usr;
	this->my_owner = owner;
	this->trigger = trigger;

	this->nick_command = "NICK " + nick;
	this->usr_command = "USER " + usr + " " + usr + " " + usr + " :" + usr;

    addAdmin(my_owner);

	awaiting_names.clear();
	names_channel.clear();

	for(list<string>::iterator it = this->channels.begin(); it != this->channels.end(); ++it)
	{
		this->join_command += "JOIN " + *it + "\r\n";
	}
}

IrcBot::~IrcBot() {

}

void IrcBot::start() {
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

char * IrcBot::timeNow() {//returns the current date and time
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	return asctime (timeinfo);
}

string IrcBot::getNick(const string &buf) {
	return buf.substr(1, buf.find("!")-1);
}

string IrcBot::getChannel(const string &buf) {
    if(buf.find("PRIVMSG") != string::npos) {
	size_t channelNamePos = buf.find("PRIVMSG #")+8;
	return buf.substr(channelNamePos, buf.find(":", channelNamePos)-channelNamePos-1);
    } else if (buf.find("JOIN") != string::npos) {
        size_t channelNamePos = buf.find("JOIN :#")+6;
        return buf.substr(channelNamePos, buf.find_last_not_of(" \r\n")-channelNamePos+1);
    } else {
        return std::string("");
    }
}

void IrcBot::sendData(const string &buf) {
	net_client.send_data(buf + "\r\n");
}

void IrcBot::sendAction(const string &msg, const string &channel) {
	sendMessage(string("\x01") + "ACTION " + msg + "\x01", channel);
}

void IrcBot::sendMessage(const string &msg, const string &channel) {
	net_client.send_data("PRIVMSG " + channel + " :" + msg + "\r\n");
}

string IrcBot::getArgument(const string &buf, const string &command) {
	size_t commandPos = buf.find(trigger+command);

	size_t argumentPos = buf.find_first_not_of(trigger+command, commandPos)+1;

	string argument = buf.substr(argumentPos);

	size_t found = argument.find_last_not_of(" \r\n");
                if (found != string::npos)
                        argument.erase(found+1);
                else argument.clear();

	return argument;
}

vector<string> IrcBot::getArguments(const string &buf, const string &command, const string &delimiters) {
        vector<string> args;

        string s = getArgument(buf, command);
        size_t current;
        size_t next = -1;

        do {
            current = next + 1;
            next = s.find_first_of(delimiters, current);
            args.push_back(s.substr(next, next-current));
        } while (next != string::npos);
        return args;
}

bool IrcBot::checkTrigger(const string &buf, const string &msgChannel, const string &command) {
    string privMsg = "PRIVMSG " + msgChannel + " :";
    return (buf.find(privMsg+trigger+command) != string::npos );
}

void IrcBot::msgHandle(const string &buf, const string &msgChannel, const string &msgNick) {
    string privMsg = "PRIVMSG " + msgChannel + " :";

	if (buf.find("Found your hostname") != string::npos && !connected) {
        	sendData(nick_command);
        	sendData(usr_command);
        	cout << nick_command << usr_command;

		connected = true;
		return;
	}
	if (buf.find("/MOTD") != string::npos && !joined) {
		sendData(join_command);

		joined = true;
		return;
	}
	if (buf.find(":Nickname is already in use.") != string::npos && buf.find("!") == string::npos) {
		cout << "Nickname already in use" << endl;
		error = true;
		return;
	}

	if (buf.substr(0, 6).find("PING :") != string::npos) {
		net_client.send_data("PONG :" + buf.substr(6));
		cout << "PONG :" + buf.substr(6);
		return;
	}



    if (buf.find("huhu " + nick) != string::npos) {
		sendMessage("huhu " + msgNick, msgChannel);
        return;
    }
	if (checkTrigger(buf, msgChannel, "help")) {
        stringstream ss;
        ss << trigger+"cookie <username> gives cookies to <username>" << endl;
        ss << trigger+"channels lists the channels I'm in" << endl;
		sendMessage(ss.str(), msgChannel);
		return;
	}
	if (checkTrigger(buf, msgChannel, "roll")) {
	    int faces = 1;
	    string arg = getArgument(buf, "roll");
	    if(arg.empty())
            faces = 6;
        else
        {
            try {
                faces = stoi(arg);
            } catch (std::invalid_argument) {
                sendMessage("You need to input a valid number", msgChannel);
                return;
            }
        }
		sendMessage(std::to_string(rand() % faces + 1), msgChannel);
		return;
	}
	if (checkTrigger(buf, msgChannel, "cookie")) {
		string argNick = getArgument(buf, "cookie");
		if(argNick.empty())
			argNick = msgNick;

		sendAction("gives " + argNick + " a cookie", msgChannel);
		return;
	}
	if (checkTrigger(buf, msgChannel, "channels")) {
		string message;
		for(list<string>::iterator it = channels.begin(); it != channels.end(); ++it)
		{
			message += " " + *it;
		}
		sendMessage(message.erase(0,1), msgChannel);
		return;
	}
	if (checkTrigger(buf, msgChannel, "channel")) {
		sendMessage(msgChannel, msgChannel);
		return;
	}
	if (checkTrigger(buf, msgChannel, "topic")) {
		if(msgNick[0] != '@')
		{
			sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
			return;
		}
		string topic = getArgument(buf, "!topic");
		net_client.send_data("TOPIC " + msgChannel + " :" + topic + "\r\n");
		return;
	}

    if (checkTrigger(buf, msgChannel, "saychan") && isAdmin(msgNick)) { // put this before say to avoid problems
        string sayThis = getArgument(buf, "saychan");
        size_t found = sayThis.find(" "); // second space
        string channelName = sayThis.substr(0, found); // get the channel name, which is the first argument
        sayThis = sayThis.substr(found+1); // get the message, which is the second argument

        sendMessage(sayThis, channelName);
        return;
    } else if (checkTrigger(buf, msgChannel, "saychan")) {
        sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
        return;
    }

	if (checkTrigger(buf, msgChannel, "say") && isAdmin(msgNick)) {
        string sayThis = getArgument(buf, "say");
        sendMessage(sayThis, msgChannel);
    } else if (checkTrigger(buf, msgChannel, "say")) {
        sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
        return;
    }
    if (checkTrigger(buf, msgChannel, "note")) {
        string message = getArgument(buf, "note");
        size_t found = message.find(" "); // second space
        string nickName = message.substr(0, found); // get the nick name, which is the first argument
        message = message.substr(found+1); // get the message, which is the second argument
        notes[nickName].push_back("note from "+msgNick+": "+message);
        sendMessage("saved note for "+nickName, msgChannel);
	}

	if (checkTrigger(buf, msgChannel, "nowplaying")) {
		sendMessage(">trccnp", msgChannel);
		return;
	}
	if (checkTrigger(buf, msgChannel, "time")) {
		sendMessage(timeNow(), msgChannel);
		return;
	}
	if (checkTrigger(buf, msgChannel, "names")) {
		string channelName = getArgument(buf, "names");

		if(channelName.empty() || channelName == "#")
			channelName = msgChannel;
		if(channelName[0] != '#') {
                        sendMessage("Channel name has to start with a #", msgChannel);
                        return;
                }
                if(!checkWhitelist(channelName, channelWhitelist)) {
                        sendMessage("Channel name contains invalid characters.", msgChannel);
                        return;
                }

		net_client.send_data("NAMES " + channelName + "\r\n");

		awaiting_names = msgChannel;

		return;

	}
	if (checkTrigger(buf, msgChannel, "namemap")) {
		if(nicks.find(msgChannel) != nicks.end()) {
			sendMessage("map: " + nicks.find(msgChannel)->second, msgChannel);
		}
		else {
			sendMessage("sorry ._.", msgChannel);
		}
		return;
	}
	if (buf.find("353 " + nick) != string::npos) {
		string names = buf.substr(buf.find_last_of(":")+1);
		size_t channelNamePos = buf.find("#");
		string channel_string = buf.substr(channelNamePos, buf.find_last_of(":")-channelNamePos-1);
		names_channel = channel_string;

		size_t found = channel_string.find_last_not_of(" \r\n");
		if (found != string::npos)
               		channel_string.erase(found+1);
        	else channel_string.clear();

		nicks.insert( make_pair(channel_string, names) );
		return;
	}
	if (buf.find("366 " + nick) != string::npos && !awaiting_names.empty()) {
	    if(!names_channel.empty())
            sendMessage("Nicks in " + names_channel + " : " + nicks[names_channel], awaiting_names);
        else
            sendMessage("No NAMES available.", awaiting_names);
		awaiting_names.clear();
		names_channel.clear();
		return;
	}

	if (checkTrigger(buf, msgChannel, "addadmin") && msgNick == my_owner) {
        string adminName = getArgument(buf, "addadmin");
        addAdmin(adminName);
        sendMessage(adminName+" was added as an admin", msgChannel);
        return;
	}
	if (checkTrigger(buf, msgChannel, "removeadmin") && msgNick == my_owner) {
        string adminName = getArgument(buf, "removeadmin");
        removeAdmin(adminName);
        sendMessage(adminName+" was removed as an admin", msgChannel);
        return;
	}
	if (checkTrigger(buf, msgChannel, "listadmins")) {
        stringstream ss;
        for(set<string>::iterator it = admins.begin(); it != admins.end(); ++it) {
            ss << *it << " ";
        }
        string adminNames= ss.str();
        adminNames.pop_back();
        sendMessage("Admins: "+adminNames, msgChannel);
        return;
	}

	if (checkTrigger(buf, msgChannel, "join") && isAdmin(msgNick)) {
		string channelName = getArgument(buf, "join");
		if(channelName.empty() || channelName == "#") {
			sendMessage("No channel specified.", msgChannel);
			return;
		}
		if(channelName[0] != '#') {
			sendMessage("Channel name has to start with a #", msgChannel);
			return;
		}
		if(!checkWhitelist(channelName, channelWhitelist)) {
			sendMessage("Channel name contains invalid characters.", msgChannel);
			return;
		}

		sendMessage("Joining " + channelName, msgChannel);
		sendData("JOIN " + channelName);
		channels.push_back(channelName);
		return;
	} else if (checkTrigger(buf, msgChannel, "join")) {
        sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
        return;
	}
	if (checkTrigger(buf, msgChannel, "leave") && isAdmin(msgNick)) {
        string channel = getArgument(buf, "leave");
        if(channel.empty())
            channel = msgChannel;
		sendMessage("Leaving "+channel, msgChannel);
		sendData("PART " + channel);
		channels.remove(channel);
		return;
	} else if (checkTrigger(buf, msgChannel, "leave")) {
        sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
        return;
	}
	if (checkTrigger(buf, msgChannel, "kick") && isAdmin(msgNick)) {
        string name = getArgument(buf, "kick");
        sendData("KICK " + msgChannel + " " + name);
	} else if (checkTrigger(buf, msgChannel, "kick")) {
        sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
	}



	if (checkTrigger(buf, msgChannel, "quit") && isAdmin(msgNick)) {
		sendMessage("Bye!", msgChannel);
		sendData("QUIT :Bye!");
		quit = true;
		return;
	}
	else if (checkTrigger(buf, msgChannel, "quit")) {
		sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
	}

	if (checkTrigger(buf, msgChannel, "sendraw") && msgNick == my_owner) {
        string myData = getArgument(buf, "sendraw");
        sendData(myData);
        return;
	} else if (checkTrigger(buf, msgChannel, "sendraw")) {
        sendMessage("Sorry " + msgNick + ", I'm afraid I can't let you do that", msgChannel);
	}

	if (!notes[msgNick].empty() && !msgChannel.empty() && msgNick != nick) {
        vector<string>::iterator it = notes[msgNick].begin();
        while(!notes[msgNick].empty()) {
            sendMessage(msgNick+": "+*it, msgChannel);
            it = notes[msgNick].erase(it);
        }
	}
}

bool IrcBot::checkWhitelist(const string &buffer, const string &charstring) {
if (buffer.find_first_not_of(charstring) != std::string::npos)
	return false;
else return true;
}

void IrcBot::addAdmin(const string &name) {
    admins.insert(name);
}

void IrcBot::removeAdmin(const string &name) {
    admins.erase(name);
}

bool IrcBot::isAdmin(const string &msgNick) {
    if(admins.find(msgNick) != admins.end()) {
        return true;
    }
    return false;
}
