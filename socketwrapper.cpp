#include "socketwrapper.h"

using namespace std;

Socket::Socket()
{
	sock = -1;
	//port = 0;
	//address = "";
	blocking = true;
	async = false;
}

bool Socket::conn(const string &address, const int &port)
{
	//create socket if it is not already created
	if(sock == -1)
	{
		//Create socket
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if(sock == -1)
		{
			perror("Could not create socket");
		}

		cout << "Socket created\n";
	}
	else	{	/* nothing */	}

	//setup address stucture
	if(inet_addr(address.c_str()) == -1)
	{
		struct hostent *he;
		struct in_addr **addr_list;

		//resolve the hostname, its not an ip address
		if((he = gethostbyname(address.c_str())) == NULL)
		{
			//gethostbyname failed
			herror("gethostbyname");
			cout<<"Failed to resolve hostname\n";

			return false;
		}

		//Cast the h_addr_list to in_addr, since h_addr_list also has the ip address in long format only
		addr_list = (struct in_addr **) he->h_addr_list;

		for(int i = 0; addr_list[i] != NULL; i++)
		{
			//strcpy(ip, inet_ntoa(*addr_list[i]));
			server.sin_addr = *addr_list[i];

			cout << address << " resolved to " << inet_ntoa(*addr_list[i]) << endl;

			break;
		}
	}
	//plain ip address
	else
	{
		server.sin_addr.s_addr = inet_addr( address.c_str());
	}

	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	//Connect to remote server
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}

	cout << "Connected\n";
	return true;
}

bool Socket::send_data(const string &data)
{
	//send some data
	if(send(sock, data.c_str(), strlen(data.c_str()), 0) < 0)
	{
		perror("Send failed : ");
		return false;
	}
	cout << "Data send\n";

	return true;
}

string Socket::receive(const int &size = 512)
{
	char buffer[size];
	string reply;
	int num;

	//Receive a reply from the server
	if( (num = recv(sock, buffer, sizeof(buffer), 0)) < 0)
	{
		puts("recv failed");
	}

	buffer[num] = '\0';

	reply = buffer;
	return reply;
}

bool Socket::set_flag(const int &flag)
{
	if(flag == O_NONBLOCK)
		blocking = false;
	else if(flag == O_ASYNC)
		async = true;
	return fcntl(sock, F_SETFL, flag);
}
