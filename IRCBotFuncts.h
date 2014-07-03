#include <string>

struct BotFunctArgs
{
    //IrcBot* bot;
    std::string buf;
    std::string msgChannel;
    std::string msgNick;
    std::string trigger;
    std::string arg;
};

class IrcBotFuncts
{
 public:
    static std::string help(BotFunctArgs& args);
    static std::string roll(BotFunctArgs& args);
    static std::string cookie(BotFunctArgs& args);
};
