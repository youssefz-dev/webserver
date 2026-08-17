#include "../headers/ConfigParser.hpp"
LocationConfig::LocationConfig() : ServerConfig(), upload_enable(false), redirect_code(0)
{
}

LocationConfig::LocationConfig(const std::string& p, ServerConfig& s) : ServerConfig(s), path(p), upload_enable(false), redirect_code(0)
{
}

LocationConfig::~LocationConfig()
{
}

const std::string& LocationConfig::getPath() const
{
    return path;
}

const std::string& LocationConfig::getRedirect() const
{
    return redirect;
}


bool LocationConfig::getUploadEnable() const
{
    return upload_enable;
}

int LocationConfig::getRedirectCode() const
{
    return redirect_code;
}

const std::map<std::string, std::string>& LocationConfig::getCgiHandlers() const
{
    return cgi_handlers;
}

void LocationConfig::setPath(const std::string& p)
{
    path = p;
}

void LocationConfig::setRedirect(const std::string& r)
{
    redirect = r;
}

void LocationConfig::setUploadEnable(bool enable)
{
    upload_enable = enable;
}

void LocationConfig::setRedirectCode(int code)
{
    redirect_code = code;
}

void LocationConfig::addCgiHandler(const std::string& extension, const std::string& interpreter)
{
    cgi_handlers[extension] = interpreter;
}