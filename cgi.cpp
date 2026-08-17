#include "headers/Server.hpp"
#include "sys/wait.h"
#include "Request.hpp"
#include "cgi.hpp"
#define TIME_OUT 5

std::string getInterpreter(std::string path, LocationConfig &cigLocation)
{
	std::map<std::string, std::string> cgi = cigLocation.getCgiHandlers();
    if ((path.rfind(".py")  == path.size() - 3) && (cgi.find(".py") != cgi.end()))
		return "/usr/bin/python3";
    if ((path.rfind(".php") == path.size() - 4) && (cgi.find(".php") != cgi.end()))
		return "/usr/bin/php";
    return "";
}

char **buildEnv(Request &req)
{
    std::string scriptName = req.path;
    size_t qpos = scriptName.find('?');
    if (qpos != std::string::npos)
        scriptName = scriptName.substr(0, qpos);

    std::vector<std::string> env;
    env.push_back("REQUEST_METHOD="   + req.method);
    env.push_back("CONTENT_LENGTH="   + req.map["Content-Length"]);
    env.push_back("CONTENT_TYPE="     + req.map["Content-Type"]);
    env.push_back("QUERY_STRING="     + req.map["QUERY_STRING"]);
    env.push_back("SCRIPT_NAME="      + scriptName);
    env.push_back("SCRIPT_FILENAME=." + scriptName);
    env.push_back("SERVER_PROTOCOL="  + req.protocol);
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("REDIRECT_STATUS=200");
    env.push_back("REQUEST_URI="      + req.path);
    env.push_back("DOCUMENT_ROOT=.");

    char **envp = new char*[env.size() + 1];
    for (size_t i = 0; i < env.size(); i++)
    {
		envp[i] = new char[env[i].size() + 1];
		std::strcpy(envp[i], env[i].c_str());
	}
    envp[env.size()] = NULL;
    return envp;
}

void freeEnv(char **envp)
{
    for (int i = 0; envp[i]; i++)
        delete envp[i];
    delete[] envp;
}

static std::string cgiErrorResponse(int cgiErr, Request &req)
{
	std::stringstream ss;
    switch (cgiErr)
    {
        case CGI_TIMEOUT:
            req.res.setCode("504");
            req.res.setMsg("Gateway Timeout");
			return "";

        case CGI_EXECVE_FAIL:
            req.res.setCode("500");
            req.res.setMsg("Internal Server Error");
			return "";

        case CGI_FORK_FAIL:
            req.res.setCode("500");
            req.res.setMsg("Internal Server Error");
            return "";

        case CGI_PIPE_FAIL:
            req.res.setCode("500");
            req.res.setMsg("Internal Server Error");
            return "";

        case CGI_EXIT_ERROR:
            req.res.setCode("500");
            req.res.setMsg("Internal Server Error");
            return "";
        default:
            req.res.setCode("500");
            req.res.setMsg("Internal Server Error");
            return "";
    }
}

std::string cgiHandler(std::string full_path, char **env, std::string interpreter, std::string post_body, Request &req)
{
    int stdin_pipe[2];
    int stdout_pipe[2];

    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0)
        return (freeEnv(env), cgiErrorResponse(CGI_PIPE_FAIL, req));

    pid_t pid = fork();
    if (pid < 0)
    {
        close(stdin_pipe[0]);
		close(stdin_pipe[1]);
        close(stdout_pipe[0]);
		close(stdout_pipe[1]);
        return (freeEnv(env), cgiErrorResponse(CGI_FORK_FAIL, req));
    }

    if (pid == 0)
    {
        dup2(stdin_pipe[0],  STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);

        char *args[] = {
            (char *)interpreter.c_str(),
            (char *)full_path.c_str(),
            NULL
        };
        if (execve(interpreter.c_str(), args, env) < 0)
			return (freeEnv(env), cgiErrorResponse(CGI_EXECVE_FAIL, req));
    	exit(1);
    }

    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    if (!post_body.empty())
    {
        const char *data = post_body.c_str();
        int size = (int)post_body.size();
        int sent = 0;
        fcntl(stdin_pipe[1], F_SETFL, O_NONBLOCK);
        while (sent < size)
        {
            int ret = write(stdin_pipe[1], data + sent, size - sent);
            if (ret < 0)
				break;
			if (ret == 0)
				continue;
            sent += ret;
        }
    }
    close(stdin_pipe[1]);

    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
    time_t start_time = time(NULL);
    bool   timed_out  = false;
    std::string out;
    char buf[4096];
    int  status = 0;

    while (true)
    {
        int check = waitpid(pid, &status, WNOHANG);
        if (check == pid)
            break;

        int n = read(stdout_pipe[0], buf, sizeof(buf));
        if (n > 0)
            out.append(buf, n);

        if (time(NULL) - start_time >= TIME_OUT)
        {
            kill(pid, SIGKILL);
            waitpid(pid, NULL, 0);
            timed_out = true;
            break;
        }
        usleep(1000);
    }

    int n;
    while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0)
        out.append(buf, n);
    close(stdout_pipe[0]);

    if (timed_out)
        return (freeEnv(env), cgiErrorResponse(CGI_TIMEOUT, req));

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return (freeEnv(env), cgiErrorResponse(CGI_EXIT_ERROR, req));
    return (freeEnv(env), out);
}