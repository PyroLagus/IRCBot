#ifndef IRCBOT_H_
#define IRCBOT_H_
#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "socketwrapper.h"

using namespace std;

struct BotFunctArgs
{
    string buf;
    string msgChannel;
    string msgNick;
    string trigger;
    string arg;
    vector<string> args;
};

class IrcBot
{
public:
	IrcBot(const string &host, const int &port, const list<string> &channel, const string &nick, const string &usr, const string &owner, const string &trigger);
	virtual ~IrcBot();

	virtual void registerFunctions() = 0;

	bool quit;
	bool error;

	void start();
	// protected:
	const string channelWhitelist = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_-#";

	Socket net_client;

	bool connected, joined;

	unordered_map<string,unordered_set<string>> nicks;
	list<string> channels;
	string nick;
	string usr;
	string my_owner;
	string trigger;
	unordered_set<string> admins;

	string nick_command;
	string usr_command;

	string awaiting_names;
	string names_channel;

	unordered_map<string,vector<string>> notes;
	unordered_map<string,function<void(IrcBot*, BotFunctArgs&)>> triggerFunctions;
	unordered_map<string,function<void(IrcBot*, BotFunctArgs&)>> deactivatedFunctions;

	void registerFunction(const string &name, function<void(IrcBot*, BotFunctArgs&)> funct);
	bool activateFunction(const string &name);
	bool deactivateFunction(const string &name);

	char * timeNow();

	string getNick(const string &buf);
	string getChannel(const string &buf);
	vector<string> splitString(const string &s, const string &delimiters);
	vector<string> splitString(const string &s, const string &delimiters, int n);
	string getArgument(const string &buf, const string &command);
	vector<string> getArguments(const string &buf, const string &command, const string &delimiters);
	vector<string> getArguments(const string &buf, const string &command, const string &delimiters, int argc);
	void sendData(const string &buf);
	void sendAction(const string &msg, const string &channel);
	void sendMessage(const string &msg, const string &msgTarget);
	void sendNotice(const string &msg, const string &nick);
	void msgHandle(const string &buf, const string &msgChannel, const string &msgNick);
	virtual void extraHandle(const string &buf, const string &msgChannel, const string &msgNick) = 0;
	bool checkWhitelist(const string &buffer, const string &charstring);

	void addAdmin(const string &name);
	void removeAdmin(const string &name);
	bool isAdmin(const string &msgNick);
	bool isMod(const string &msgNick, const string &msgChannel);

	int checkChannelName(const string &channel);

	int join(const string &channel);
	int leave(const string &channel);
};
#endif
