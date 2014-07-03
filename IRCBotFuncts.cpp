#include <string>
#include <sstream>
#include <stdexcept>
#include "IRCBotFuncts.h"

using namespace std;

string IrcBotFuncts:: help(BotFunctArgs& args) {
    stringstream ss;
    ss << args.trigger+"cookie <username> gives cookies to <username>" << endl;
    ss << args.trigger+"channels lists the channels I'm in" << endl;
    //args.bot->sendMessage(ss.str(), args.msgChannel);
    return ss.str();
}

string IrcBotFuncts::roll(BotFunctArgs& args) {
    int faces = 1;
    //string arg = args.bot->getArgument(args.buf, "roll");
    if(args.arg.empty())
	faces = 6;
    else
        {
            try {
                faces = stoi(args.arg);
            } catch (std::invalid_argument) {
                //args.bot->sendMessage("You need to input a valid number", args.msgChannel);
                return "You need to input a valid number";
            }
	}
    //args.bot->sendMessage(std::to_string(rand() % faces + 1), args.msgChannel);
    return std::to_string(rand() % faces + 1);
}

string IrcBotFuncts::cookie(BotFunctArgs& args) {
    string argNick = getArgument(buf, "cookie");
    if(argNick.empty())
	argNick = msgNick;
    
    sendAction("gives " + argNick + " a cookie", msgChannel);
    return;
}
