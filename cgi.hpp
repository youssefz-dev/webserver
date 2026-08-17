#pragma once 
#include "headers/ServerConfig.hpp"
#include "Request.hpp"

enum CgiError {
    CGI_OK          =  0,
    CGI_TIMEOUT     = -1,
    CGI_EXECVE_FAIL = -2,
    CGI_FORK_FAIL   = -3,
    CGI_PIPE_FAIL   = -4,
    CGI_EXIT_ERROR  = -5,
};
std::string getInterpreter(std::string path, LocationConfig &cigLocation);
char **buildEnv(Request &req);
void freeEnv(char **envp);
std::string cgiHandler(std::string full_path, char **env, std::string interpreter, std::string post_body, Request &req);