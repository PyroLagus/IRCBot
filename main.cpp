#include <sstream>
#include <getopt.h>
#include "IRCBot.h"

void help(const string&);

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		cout << "Missing arguments!" << endl;
		help(argv[0]);
		return 1;
	}

	string server, channel_string, nick, port_string, owner;
	int port;
	
	int c;

	while ((c = getopt (argc, argv, "s:p:c:n:o:")) != -1)
	{
		switch(c)
		{
			case 's':
				server = optarg;
				break;
			case 'p':
				port_string = optarg;
				break;
			case 'c':
				channel_string = optarg;
				break;
			case 'n':
				nick = optarg;
				break;
			case 'o':
				owner = optarg;
				break;
		}
	}
	
	bool missing = false;

	if(server.empty())
	{
		cout << "Missing server argument (-s)" << endl;
		missing = true;
	}
	if(channel_string.empty())
	{
		cout << "Missing channel argument (-c)" << endl;
		missing = true;
	}
	if(nick.empty())
	{
		cout << "Missing nick argument (-n)" << endl;
		missing = true;
	}
	if(owner.empty())
	{
		cout << "Missing owner argument (-o)" << endl;
		missing = true;
	}
	if(missing)
	{
		help(argv[0]);
		return 1;
	}

	if(port_string.empty())
		port_string = "6667";
	port = stoi(port_string);

	cout << "----------information----------" << endl;
	cout << "server=" + server <<endl;
        cout << "port=" + port_string << endl;
        cout << "channel=" + channel_string << endl;
        cout << "nick=" + nick << endl;
	cout << "owner=" + owner << endl;
	cout << "-------------------------------" << endl;

	list<string> channels;
	stringstream ss(channel_string);

	string channel;

	while(ss >> channel)
	{
		channels.push_back(channel);

		if (ss.peek() == ',')
			ss.ignore();
	}
	
	IrcBot bot = IrcBot(server, port, channels, nick, "test", owner);
	bot.start();

  return 0;

}

void help(const string &program_name)
{
cout << "Use like this:" << endl;
cout << string(program_name) + " -s irc.myserver.org [-p 6667] -c #mychannel -n botname" << endl;
cout << endl;
cout << "-s\tserver address" << endl;
cout << "-p\tport (standard: 6667)" << endl;
cout << "-c\tchannel/s (example -c #mychannel or -c #mychannel,#yourchannel -o you)" << endl;
cout << "-n\tname" << endl;
cout << "-o\towner" << endl;
cout << endl;
cout << "arguments can be given in any order" << endl;
}
