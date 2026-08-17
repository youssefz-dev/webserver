#pragma once

#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

class ConfigParser
{
	private:
		std::vector<ServerConfig> servers;
		std::string config_file;
		
		std::string trim(const std::string& str);
		std::vector<std::string> split(const std::string& str);
		
		ServerConfig parse_server(const std::vector<std::string>& lines, size_t start, size_t end);
		LocationConfig parse_location(ServerConfig& server, const std::vector<std::string>& lines, size_t start, size_t end);
		
		void parse_directive(const std::string& line, ServerConfig& config);
		void parse_location_directive(const std::string& line, LocationConfig& config);
		
		bool is_valid_method(const std::string& method);
		int parse_error_code(const std::string& code);
		size_t parse_size(const std::string& size_str);

	public:
		ConfigParser(const std::string& filename);
		~ConfigParser();
		bool parse();
		const std::vector<ServerConfig>& getServers() const;
};