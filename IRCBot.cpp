#include "IRCBot.h"
#include <iostream>
#include <cstdio>
#include <sstream>
#include <algorithm>
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
}

IrcBot::~IrcBot() {
}

void IrcBot::start() {
    string buffer;	//buffer is the data that is recived

    while (true) {
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

void IrcBot::sendMessage(const string &msg, const string &msgTarget) {
    net_client.send_data("PRIVMSG " + msgTarget + " :" + msg + "\r\n");
}

void IrcBot::sendNotice(const string &msg, const string &nick) {
    net_client.send_data("NOTICE " + nick + " " + msg + "\r\n");
}

vector<string> IrcBot::splitString(const string &s, const string &delimiters) {
    vector<string> strings;
    size_t current;
    size_t next = -1;
    
    do {
	current = next + 1;
	next = s.find_first_of(delimiters, current);
	strings.push_back(s.substr(current, next-current));
    } while (next != string::npos);
    return strings;
}

vector<string> IrcBot::splitString(const string &s, const string &delimiters, int n) {
    vector<string> strings;
    size_t current;
    size_t next = -1;
    int c = 1;
    
    if(n < 1)
	n = 1;
    
    do {
	if(c == n) {
	    break;
	}
	c++;
	current = next + 1;
	next = s.find_first_of(delimiters, current);
	strings.push_back(s.substr(current, next-current));
    } while (next != string::npos);
    if(next != string::npos) {
	strings.push_back(s.substr(next+1));
    }
    return strings;
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
    string s = getArgument(buf, command);
    return splitString(s, delimiters);
}

vector<string> IrcBot::getArguments(const string &buf, const string &command, const string &delimiters, int argc) {
    vector<string> args;
    string s = getArgument(buf, command);
    size_t current;
    size_t next = -1;
    int c = 1;
    
    if(argc < 1)
	argc = 1;
    
    do {
	if(c == argc) {
	    break;
	}
	c++;
	current = next + 1;
	next = s.find_first_of(delimiters, current);
	args.push_back(s.substr(current, next-current));
    } while (next != string::npos);
    if(next != string::npos) {
	args.push_back(s.substr(next+1));
    }
    return args;
}

void IrcBot::msgHandle(const string &buf, const string &msgChannel, const string &msgNick) {
    BotFunctArgs args;
    args.buf = buf;
    args.msgChannel = msgChannel;
    args.msgNick = msgNick;
    args.trigger = trigger;

    string privMsg = "PRIVMSG " + msgChannel + " :";
    string head = privMsg + trigger;
    
    if(buf.find(head) != string::npos) {
	std::size_t first = buf.find(' ');
	first = buf.find(' ', first+1);
	first = buf.find(' ', first+1);
	first += 3;
	std::size_t last = buf.find_first_of(" \n\r", first+1);
	last;

	string command = buf.substr(first, last-first);
	args.arg = getArgument(buf, command);
	args.args = getArguments(buf, command, " ");

	try {
	    triggerFunctions.at(command)(this, args);
	}
	catch (std::out_of_range)
	    {
		cout << "|" << command << "|" << " is not a valid command." << endl;
	    }
	return; //we don't want to call other stuff if it's a PRIVMSG (we can handle PRIVMSG by using the triggerFunctions)
    }
    extraHandle(buf, msgChannel, msgNick);

    if (buf.find("Found your hostname") != string::npos && !connected) {
	sendData(nick_command);
	sendData(usr_command);
	cout << nick_command << usr_command;
	    
	connected = true;
	return;
    }
    if (buf.find("/MOTD") != string::npos && !joined) {
	//sendData(join_command);
	for(auto f : channels) {
	    sendData("JOIN " + f);
	    sendData("NAMES " + f);
	}
	
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
    if (buf.find("JOIN") != string::npos) {
	size_t channel_start = buf.find_last_of("#");
	size_t channel_end = buf.find_last_not_of("\r\n")+1;
	string channel = buf.substr(channel_start, channel_end-channel_start);
	try {
	    nicks.at(channel).insert(msgNick);
	    for(auto f : channels)
		sendMessage(msgNick+" joined "+channel, f);
	}
	catch (std::out_of_range)
	    {
	    }
    }
    if (buf.find("PART") != string::npos) {
	size_t channel_start = buf.find_last_of("#");
	size_t channel_end = buf.find_last_not_of("\r\n")+1;
	string channel = buf.substr(channel_start, channel_end-channel_start);
	try {
	    nicks.at(channel).erase(msgNick);
	    for(auto f : channels)
		sendMessage(msgNick+" left "+channel, f);
	}
	catch (std::out_of_range)
	    {
	    }
    }
    if (buf.find("QUIT") != string::npos) {
	// loop through map, search for nick, and erase each
	for(auto &f : nicks) {
	    f.second.erase(msgNick);
	}
    }
    if (buf.find("353 " + nick) != string::npos) {
	size_t names_start = buf.find_last_of(":")+1;
	size_t names_end = buf.find_last_not_of("\r\n")+1;
	string names_string = buf.substr(names_start, names_end-names_start);
	size_t channelNamePos = buf.find("#");
	string channel_string = buf.substr(channelNamePos, buf.find_last_of(":")-channelNamePos-1);
	names_channel = channel_string;

	size_t found = channel_string.find_last_not_of(" \r\n");
	if (found != string::npos)
	    channel_string.erase(found+1);
	else channel_string.clear();

	vector<string> names_vector = splitString(names_string, " ");
	unordered_set<string> names_set;
	for(auto f : names_vector) {
	    names_set.insert(f);
	}

	nicks.insert( make_pair(channel_string, names_set) );
	return;
    }
    if (buf.find("366 " + nick) != string::npos && !awaiting_names.empty()) {
	if(!names_channel.empty()) { 
	    stringstream ss;
	    unordered_set<string>::iterator names_end = nicks[names_channel].end();
	    for(auto f : nicks[names_channel]) {
		ss << f << " ";
	    }
	    sendMessage("Nicks in " + names_channel + " : " + ss.str(), awaiting_names);
	}
	else
            sendMessage("No NAMES available.", awaiting_names);
	awaiting_names.clear();
	names_channel.clear();
	return;
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

bool IrcBot::isMod(const string &msgNick, const string &msgChannel) {
    if(nicks.find(msgChannel)->second.find("@"+msgNick) != nicks.find(msgChannel)->second.end()) {
	return true;
    }
    return false;
}
// 0=everything good
// 1=no channel name
// 2=doesn't start with #
// 3=has invalid characters
// 4=already in channel/not in channel
int IrcBot::checkChannelName(const string &channel) {
    if(channel.empty() || channel == "#") {
	return 1;
    }
    if(channel[0] != '#') {
	return 2;
    }
    if(!checkWhitelist(channel, channelWhitelist)) {
	return 3;
    }
    return 0;
}

int IrcBot::join(const string &channel) {
    int retVal = checkChannelName(channel);
    if(retVal != 0) {
	return retVal;
    }
    if(find(channels.begin(), channels.end(), channel) != channels.end()) {
	return 4;
    }
    sendData("JOIN " + channel);
    channels.push_back(channel);
    sendData("NAMES " + channel);
    return 0;
}

int IrcBot::leave(const string &channel) {
    if(channel.empty()) {
	return 1;
    } else if(find(channels.begin(), channels.end(), channel) == channels.end()) {
	return 4;
    }
    sendData("PART " + channel);
    channels.remove(channel);
    return 0;
}

void IrcBot::registerFunction(const string &name, function<void(IrcBot*, BotFunctArgs&)> funct) {
    triggerFunctions[name] = funct;
}

bool IrcBot::activateFunction(const string &name) {
    try {
	pair<string,function<void(IrcBot*, BotFunctArgs&)>> funct(name, deactivatedFunctions.at(name));
	deactivatedFunctions.erase(name);
	return get<1>(triggerFunctions.insert(funct));
    }
    catch (std::out_of_range)
	{
	    return false;
	}
}

bool IrcBot::deactivateFunction(const string &name) {
    try {
	pair<string,function<void(IrcBot*, BotFunctArgs&)>> funct(name, triggerFunctions.at(name));
	triggerFunctions.erase(name);
	return get<1>(deactivatedFunctions.insert(funct));
    }
    catch (std::out_of_range)
	{
	    return false;
	}
}
