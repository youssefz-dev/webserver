#pragma once
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include "Response.hpp"
#include "headers/ServerConfig.hpp"
#include "headers/Client.hpp"
#include "headers/Server.hpp"
#include "cgi.hpp"

class Request
{
	public:
		std::vector<std::string> vec;
		std::vector<std::string> folders;
		std::map<std::string, std::string> map;
		std::map<std::string, std::string> querys;
		std::string query;
		std::string body;
		std::string method;
		std::string path;
		std::string rpath;
		std::string protocol;
		std::string boundary;
		std::string dir;
		bool index;
		bool autoIndex;
		bool iscgi;
		int fd;
		std::string cgistr;
	public:
		Response res;
		Request();
		~Request();
		void get(std::vector<LocationConfig>::iterator &it0);
		void post(std::stringstream &out, int fdo, std::vector<LocationConfig>::iterator &it0);
		void del(std::vector<LocationConfig>::iterator &it0);
		void parser(std::stringstream &ss, std::stringstream &out, int fdo, Client &client);
		bool parseFirstLine(std::stringstream &ss, Client &client);
		void parseHedear(std::stringstream &ss);
		void fullBody(std::stringstream &out, int fdo);
		void getBoundary();
		void postAction(std::stringstream &out, bool upload);
};