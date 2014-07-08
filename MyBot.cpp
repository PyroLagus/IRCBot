#include "IRCBot.h"
#include "MyBot.h"
#include <sstream>

void MyBot::help(IrcBot* bot, BotFunctArgs arguments) {
    stringstream ss;
    ss << "Hello! I'm IRCBot. You can get my source from https://github.com/C0DEHERO/IRCBot/" << endl;
    ss << "I'm written in C++ and this is what i can do!" << endl;
    ss << arguments.trigger+"cookie <username> gives cookies to <username>" << endl;
    ss << arguments.trigger+"roll <faces> rolls a die with <faces> faces. Faces defaults to 6" << endl;
    ss << arguments.trigger+"channels lists the channels I'm in" << endl;
    ss << arguments.trigger+"time prints my local time" << endl;
    bot->sendMessage(ss.str(), arguments.msgChannel);
}

void MyBot::info(IrcBot* bot, BotFunctArgs arguments) {
    stringstream ss;
    ss << "Hello! I'm IRCBot. You can get my source from https://github.com/C0DEHERO/IRCBot/" << endl;
    ss << "I'm written in C++ and this is what i can do!" << endl;
    bot->sendMessage(ss.str(), arguments.msgChannel);
}
void MyBot::cookie(IrcBot* bot, BotFunctArgs arguments) {
    string argNick = arguments.arg;
       if(argNick.empty())
	   argNick = arguments.msgNick;
       bot->sendAction("gives " + argNick + " a cookie", arguments.msgChannel);
}

void MyBot::roll(IrcBot* bot, BotFunctArgs arguments) {
    int faces = 1;
    if(arguments.arg.empty())
	faces = 6;
    else
        {
            try {
                faces = stoi(arguments.arg);
            } catch (std::invalid_argument) {
		bot->sendMessage("You need to input a valid number", arguments.msgChannel);
            }
	}
    bot->sendMessage(std::to_string(rand() % faces + 1), arguments.msgChannel);
}

void MyBot::time(IrcBot* bot, BotFunctArgs arguments) {
    bot->sendMessage(bot->timeNow(), arguments.msgChannel);
}

void MyBot::note(IrcBot* bot, BotFunctArgs arguments) {
    string message = arguments.arg;
    size_t found = message.find(" "); // second space
    string nickName = message.substr(0, found); // get the nick name, which is the first argument
    message = message.substr(found+1); // get the message, which is the second argument
    bot->notes[nickName].push_back("note from "+arguments.msgNick+": "+message);
    bot->sendMessage("saved note for "+nickName, arguments.msgChannel);
}

void MyBot::say(IrcBot* bot, BotFunctArgs arguments) {
    if (bot->isAdmin(arguments.msgNick)) {
        bot->sendMessage(arguments.arg, arguments.msgChannel);
    } else {
        bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::say_chan(IrcBot* bot, BotFunctArgs arguments) {
    if (bot->isAdmin(arguments.msgNick)) {
	vector<string> args;
	args = bot->splitString(arguments.arg, " ", 2);
        bot->sendMessage(args[1], args[0]);
    } else {
        bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::add_admin(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->isAdmin(arguments.msgNick)) {
	bot->addAdmin(arguments.arg);
	bot->sendMessage(arguments.arg+" was added as an admin", arguments.msgChannel);
    }
}

void MyBot::add_admins(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->isAdmin(arguments.msgNick)) {
	for(auto f : arguments.args) {
	    bot->addAdmin(f);
	    bot->sendMessage(f+" was added as an admin", arguments.msgChannel);
	}
    }
}

void MyBot::remove_admin(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->isAdmin(arguments.msgNick)) {
	bot->removeAdmin(arguments.arg);
	bot->sendMessage(arguments.arg+" was removed from the admin list", arguments.msgChannel);
    }
}

void MyBot::list_admins(IrcBot* bot, BotFunctArgs arguments) {
    stringstream ss;
    for(auto f : bot->admins) {
	ss << f << " ";
    }
    string adminNames= ss.str();
    adminNames.pop_back();
    bot->sendMessage("Admins: "+adminNames, arguments.msgChannel);
}

void MyBot::channel(IrcBot* bot, BotFunctArgs arguments) {
    bot->sendMessage(arguments.msgChannel, arguments.msgChannel);
}

void MyBot::channels(IrcBot* bot, BotFunctArgs arguments) {
    string message;
    for(auto f : bot->channels)
	{
	    message += " " + f;
	}
    bot->sendMessage(message.erase(0,1), arguments.msgChannel);
}

void MyBot::topic(IrcBot* bot, BotFunctArgs arguments) {
    if(arguments.msgNick[0] != '@') {
	bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that", arguments.msgChannel);
    } else {
	bot->sendData("TOPIC " + arguments.msgChannel + " :" + arguments.arg);
    }
}

void MyBot::names(IrcBot* bot, BotFunctArgs arguments) {
    string channelName = arguments.arg;
    if(channelName.empty() || channelName == "#")
	channelName = arguments.msgChannel;
    if(channelName[0] != '#') {
	bot->sendMessage("Channel name has to start with a #", arguments.msgChannel);
	return;
    }
    if(!bot->checkWhitelist(channelName, bot->channelWhitelist)) {
	bot->sendMessage("Channel name contains invalid characters.", arguments.msgChannel);
	return;
    }
    bot->sendData("NAMES " + channelName);
    bot->awaiting_names = arguments.msgChannel;
}

void MyBot::name_map(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->nicks.find(arguments.msgChannel) != bot->nicks.end()) {
	string names;
	names = "map: ";
	for(auto f : bot->nicks.find(arguments.msgChannel)->second) {
	    names += f+" ";
	}
	//bot->sendMessage(names, arguments.msgChannel);
	bot->sendMessage(names, bot->my_owner);
    }
    else {
	bot->sendMessage("sorry ._.", arguments.msgChannel);
    }
}

void MyBot::send_raw(IrcBot* bot, BotFunctArgs arguments) {
    if (bot->isAdmin(arguments.msgNick)) {
        bot->sendData(arguments.arg);
    } else {
        bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::activate(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->isAdmin(arguments.msgNick)) {
	if(bot->activateFunction(arguments.arg)) {
	    bot->sendMessage(arguments.arg+" was activated.", arguments.msgChannel);
	}
    } else {
	bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::deactivate(IrcBot* bot, BotFunctArgs arguments) {
    if(arguments.arg == "activate") {
	bot->sendMessage("The activate function cannot be deactivated.", arguments.msgChannel);
    } else if(bot->isAdmin(arguments.msgNick)) {
	if(bot->deactivateFunction(arguments.arg)) {
	    bot->sendMessage(arguments.arg+" was deactivated.", arguments.msgChannel);
	}
    } else {
	bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::join(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->isAdmin(arguments.msgNick)) {
	int result = bot->join(arguments.arg);
	switch(result) {
	case 0:
	    bot->sendMessage("Joining "+arguments.arg+".", arguments.msgChannel);
	    break;
	case 1:
	    bot->sendMessage("No channel specified.", arguments.msgChannel);
	    break;
	case 2:
	    bot->sendMessage("Channel name has to start with a #", arguments.msgChannel);
	    break;
	case 3:
	    bot->sendMessage("Channel name contains invalid characters.", arguments.msgChannel);
	    break;
	case 4:
	    bot->sendMessage("I'm already in that channel.", arguments.msgChannel);
	    break;
	}
    } else {
	bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::leave(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->isAdmin(arguments.msgNick) || bot->isMod(arguments.msgNick, arguments.msgChannel)) {
	int result = bot->leave(arguments.arg);
	switch(result) {
	case 0:
	    bot->sendMessage("Leaving "+arguments.arg+".", arguments.msgChannel);
	    break;
	case 1:
	    bot->sendMessage("No channel specified.", arguments.msgChannel);
	    break;
	case 2:
	    bot->sendMessage("Channel name has to start with a #", arguments.msgChannel);
	    break;
	case 3:
	    bot->sendMessage("Channel name contains invalid characters.", arguments.msgChannel);
	    break;
	case 4:
	    bot->sendMessage("I'm not in that channel.", arguments.msgChannel);
	    break;
	}
    } else {
	bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::quit(IrcBot* bot, BotFunctArgs arguments) {
    if(bot->isAdmin(arguments.msgNick)) {
	bot->sendMessage("Bye!", arguments.msgChannel);
	bot->sendData("QUIT :Bye!");
	bot->quit = true;
    } else {
	bot->sendMessage("Sorry " + arguments.msgNick + ", I'm afraid I can't let you do that.", arguments.msgChannel);
    }
}

void MyBot::registerFunctions() {
    registerFunction("help", MyBot::help);
    registerFunction("info", MyBot::info);
    registerFunction("cookie", MyBot::cookie);
    registerFunction("roll", MyBot::roll);
    registerFunction("time", MyBot::time);
    registerFunction("note", MyBot::note);
    registerFunction("addadmin", MyBot::add_admin);
    registerFunction("addadmins", MyBot::add_admins);
    registerFunction("say", MyBot::say);
    registerFunction("saychan", MyBot::say_chan);
    registerFunction("removeadmin", MyBot::remove_admin);
    registerFunction("listadmins", MyBot::list_admins);
    registerFunction("channel", MyBot::channel);
    registerFunction("channels", MyBot::channels);
    registerFunction("topic", MyBot::topic);
    registerFunction("names", MyBot::names);
    registerFunction("namemap", MyBot::name_map);
    registerFunction("sendraw", MyBot::send_raw);
    registerFunction("activate", MyBot::activate);
    registerFunction("deactivate", MyBot::deactivate);
    registerFunction("join", MyBot::join);
    registerFunction("leave", MyBot::leave);
    registerFunction("quit", MyBot::quit);
}

void MyBot::extraHandle(const string &buf, const string &msgChannel, const string &msgNick) {
    if (buf.find("huhu " + nick) != string::npos) {
	sendMessage("huhu " + msgNick, msgChannel);
    }
    if (!notes[msgNick].empty() && !msgChannel.empty() && msgNick != nick) {
        vector<string>::iterator it = notes[msgNick].begin();
        while(!notes[msgNick].empty()) {
            sendMessage(msgNick+": "+*it, msgChannel);
            it = notes[msgNick].erase(it);
        }
	return;
    }
}
