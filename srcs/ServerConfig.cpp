#include "../headers/ConfigParser.hpp"

ServerConfig::ServerConfig() 
    : autoindex(false)
{
}

ServerConfig::~ServerConfig()
{
}

void ServerConfig::addListen(const std::string& host, int port)
{
    all_listen.push_back(Listen(host, port));
}
const std::vector<Listen> &ServerConfig::getListen() const
{
	return (all_listen);
}

const std::string& ServerConfig::getRoot() const
{
    return root;
}

const std::vector<std::string>& ServerConfig::getIndexFiles() const
{
    return index_files;
}

bool ServerConfig::getAutoindex() const
{
    return autoindex;
}

const std::vector<std::string>& ServerConfig::getMethods() const
{
    return methods;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
    return error_pages;
}

std::string ServerConfig::getClientMaxBodySize() const
{
    return client_max_body_size;
}

void ServerConfig::setRoot(const std::string& r)
{
    root = r;
}

void ServerConfig::addIndexFile(const std::string& i)
{
    index_files.push_back(i);
}

void ServerConfig::setAutoindex(bool a)
{
    autoindex = a;
}

void ServerConfig::addMethod(const std::string& m)
{
    methods.push_back(m);
}

void ServerConfig::setErrorPage(int code, const std::string& path)
{
    error_pages[code] = path;
}

void ServerConfig::setClientMaxBodySize(std::string size)
{
    client_max_body_size = size;
}

void ServerConfig::addLocation(const LocationConfig& loc)
{
    locations.push_back(loc);
}

std::vector<LocationConfig>& ServerConfig::getLocations()
{
    return locations;
}
