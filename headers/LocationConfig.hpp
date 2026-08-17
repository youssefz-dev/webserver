#pragma once

#include "ServerConfig.hpp"
class LocationConfig : public ServerConfig
{
public:
    std::string path;
    std::string redirect;
    bool upload_enable;
    int redirect_code;
    std::map<std::string, std::string> cgi_handlers;

public:
    LocationConfig();
    LocationConfig(const std::string& p, ServerConfig& s);
    ~LocationConfig();

    const std::string& getPath() const;
    const std::string& getRedirect() const;
    bool getUploadEnable() const;
    int getRedirectCode() const;
    const std::map<std::string, std::string>& getCgiHandlers() const;
    void setPath(const std::string& p);
    void setRedirect(const std::string& r);
    void setUploadEnable(bool enable);
    void setRedirectCode(int code);
    void addCgiHandler(const std::string& extension, const std::string& interpreter);
};