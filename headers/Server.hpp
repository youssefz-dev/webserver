#pragma once 

#include "ConfigParser.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <sys/poll.h>
#include <fcntl.h>
#include <algorithm>
#include <cstring>
#include  <signal.h>
#include "Client.hpp"

class Server
{
	private:
		std::map<int, Client> clients;
		std::vector<int> server_fds; 
		ConfigParser& _servers;
	public:
		Server(ConfigParser& servers);
		void startServers();
		~Server();
};