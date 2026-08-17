#include "../headers/Server.hpp"
#include "../Request.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
int main(int argc, char* argv[])
{
	
	if (argc > 2)
	{
		std::cerr << "./webserv [configuration file] or default " << std::endl;
		return 1;
	}
	std::string config_file;
	if (argc == 1)
		config_file = "default.conf";
	else
		config_file = argv[1];

	if (config_file.length() < 5 || config_file.substr(config_file.length() - 5) != ".conf")
	{
		std::cerr << "the file extention must be .conf";
		return 1;
	}
	try
	{
		ConfigParser parser(config_file);
		
		if (!parser.parse())
		{
			std::cerr << "Failed to parse configuration file" << std::endl;
			return 1;
		}
		Server s(parser);
		s.startServers();
		
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	
	return 0;
}