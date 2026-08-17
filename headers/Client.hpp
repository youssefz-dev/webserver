#pragma once
#include "ServerConfig.hpp"
#include <string>
#include <ctime>

class Client
{
public:
    int fd;

    // std::string readBuffer;
    std::string writeBuffer;
	ServerConfig server;

    size_t bytesSent;
    time_t lastActivity;

    enum State
    {
        READING,
        WRITING
    };

    State state;

    Client() : fd(-1), bytesSent(0), lastActivity(time(NULL)), state(READING) {}

    Client(int clientFd): fd(clientFd), bytesSent(0), lastActivity(time(NULL)), state(READING){}
};