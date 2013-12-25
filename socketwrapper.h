#ifndef SOCKET_WRAPPER_H
#define SOCKET_WRAPPER_H

#include<iostream>	//cout
#include<stdio.h>	//printf
#include<string.h>	//strlen
#include<string>	//string
#include<sys/socket.h>	//socket
#include<arpa/inet.h>	//inet_addr
#include<netdb.h>	//hostent
#include <sys/fcntl.h>

using namespace std;

class Socket
{
private:
	int sock;
	//std::string address;
	//int port;
	struct sockaddr_in server;
	bool blocking, async;

public:
	Socket();
	bool conn(const string &, const int &);
	bool send_data(const string &data);
	string receive(const int&);
	bool set_flag(const int &flag);
};

#endif
