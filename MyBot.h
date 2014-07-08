#ifndef MYBOT_H_
#define MYBOT_H_
#include "IRCBot.h"


class MyBot : public IrcBot
{
 public:
    using IrcBot::IrcBot;
    static void help(IrcBot* bot, BotFunctArgs arguments);
    static void info(IrcBot* bot, BotFunctArgs arguments);
    static void cookie(IrcBot* bot, BotFunctArgs arguments);
    static void roll(IrcBot* bot, BotFunctArgs arguments);
    static void time(IrcBot* bot, BotFunctArgs arguments);
    static void note(IrcBot* bot, BotFunctArgs arguments);
    static void say(IrcBot* bot, BotFunctArgs arguments);
    static void say_chan(IrcBot* bot, BotFunctArgs arguments);
    static void add_admin(IrcBot* bot, BotFunctArgs arguments);
    static void add_admins(IrcBot* bot, BotFunctArgs arguments);
    static void remove_admin(IrcBot* bot, BotFunctArgs arguments);
    static void list_admins(IrcBot* bot, BotFunctArgs arguments);
    static void channel(IrcBot* bot, BotFunctArgs arguments);
    static void channels(IrcBot* bot, BotFunctArgs arguments);
    static void topic(IrcBot* bot, BotFunctArgs arguments);
    static void names(IrcBot* bot, BotFunctArgs arguments);
    static void name_map(IrcBot* bot, BotFunctArgs arguments);
    static void send_raw(IrcBot* bot, BotFunctArgs arguments);
    static void activate(IrcBot* bot, BotFunctArgs arguments);
    static void deactivate(IrcBot* bot, BotFunctArgs arguments);
    static void join(IrcBot* bot, BotFunctArgs arguments);
    static void leave(IrcBot* bot, BotFunctArgs arguments);
    static void quit(IrcBot* bot, BotFunctArgs arguments);
    void registerFunctions();
    void extraHandle(const string &buf, const string &msgChannel, const string &msgNick);
};

#endif
