#include "../headers/Server.hpp"
#include "../Request.hpp"

Server::Server(ConfigParser& servers) : _servers(servers)
{}

void Server::startServers()
{
    signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

    std::vector<ServerConfig> servers = _servers.getServers();
    std::vector<struct pollfd> pollfds;

    for (size_t i = 0; i < servers.size(); i++)
    {
        std::vector<Listen> listens = servers[i].getListen();

        for (size_t j = 0; j < listens.size(); j++)
        {
            int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (serverSocket < 0)
                throw std::runtime_error("socket failed");

            int opt = 1;
            if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
                throw std::runtime_error("setsockopt failed");

            int flags = fcntl(serverSocket, F_GETFL, 0);
            if (flags < 0 || fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) < 0)
                throw std::runtime_error("fcntl failed");

            if (listens[j].host == "localhost")
                listens[j].host = "127.0.0.1";

            sockaddr_in addr;
            memset(&addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(listens[j].port);
            addr.sin_addr.s_addr = inet_addr(listens[j].host.c_str());

            if (addr.sin_addr.s_addr == INADDR_NONE)
                throw std::runtime_error("invalid host");

            if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                throw std::runtime_error("bind failed");

            if (listen(serverSocket, SOMAXCONN) < 0)
                throw std::runtime_error("listen failed");

            struct pollfd pfd;
            pfd.fd = serverSocket;
            pfd.events = POLLIN;
            pfd.revents = 0;
            pollfds.push_back(pfd);

            server_fds.push_back(serverSocket);

            std::cout << "Server initialized on "
                      << listens[j].host << ":"
                      << listens[j].port
                      << " (FD: " << serverSocket << ")"
                      << std::endl;
        }
    }

     while (1)
    {
        int ret = poll(pollfds.data(), pollfds.size(), 1000);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("poll failed");
        }

        time_t now = time(NULL);

        for (size_t i = 0; i < pollfds.size(); ++i)
        {
            int fd = pollfds[i].fd;
            if (std::find(server_fds.begin(), server_fds.end(), fd) != server_fds.end())
            {
                if (pollfds[i].revents & POLLIN)
                {
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);

                    int client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0)
                        continue;

                    fcntl(client_fd, F_SETFL, O_NONBLOCK);

                    struct pollfd client_pfd;
                    client_pfd.fd = client_fd;
                    client_pfd.events = POLLIN;
                    client_pfd.revents = 0;
                    pollfds.push_back(client_pfd);

                    clients[client_fd] = Client(client_fd);

					getsockname(client_fd, (sockaddr*)&client_addr, &client_len);

					char local_ip[INET_ADDRSTRLEN];
					inet_ntop(AF_INET, &client_addr.sin_addr, local_ip, sizeof(local_ip));

					std::stringstream ss, s;
					ss << local_ip;
					s << ntohs(client_addr.sin_port);
					std::string host = ss.str() + ":" + s.str();
					for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); it++)
					{
						if (std::find(it->hosts.begin(), it->hosts.end(), host) != it->hosts.end())
							clients[client_fd].server = *it;
						else
						{
							host = "0.0.0.0:" + s.str();
							if (std::find(it->hosts.begin(), it->hosts.end(), host) != it->hosts.end())
								clients[client_fd].server = *it;
						}
					}
					std::cout << "Server: "
							<< host
							<< std::endl;
                }
            }
            else
            {
                Client &client = clients[fd];
				Request req;

				if ((pollfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)))
				{
					close(fd);
					clients.erase(fd);
					pollfds.erase(pollfds.begin() + i);
					i--;
					continue;
					
				}
                if ((pollfds[i].revents & POLLIN) && client.state == Client::READING)
                {
					std::stringstream ss;
					std::stringstream out;
					std::string str;
					char buffer[1024];
					std::memset(buffer, 0, sizeof(buffer));
					int bytes = recv(fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
					std::string s;
					while (bytes > 0)
					{
						str += buffer;
						size_t len = str.find("\r\n\r\n");
						if (len != std::string::npos)
						{
							ss << str.substr(0, len);
							s = buffer;
							for (int i = s.find("\r\n\r\n") + 4; i < bytes; i++)
								out << buffer[i];
							break ;
						}
						bytes = recv(fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
					}
					req.parser(ss, out, fd, client);
                    client.lastActivity = now;
					client.state = Client::WRITING;
					pollfds[i].events = POLLOUT;
                }

                if ((pollfds[i].revents & POLLOUT) && client.state == Client::WRITING)
                {

                    size_t remaining = client.writeBuffer.size() - client.bytesSent;
                    int sent = send(fd,
                                    client.writeBuffer.c_str() + client.bytesSent,
                                    remaining,
                                    0);

                    if (sent <= 0)
                    {
                        close(fd);
                        clients.erase(fd);
                        pollfds.erase(pollfds.begin() + i);
                        i--;
                        continue;
                    }

                    client.bytesSent += sent;
                    client.lastActivity = now;

                    if (client.bytesSent >= client.writeBuffer.size())
                    {
                        client.writeBuffer.clear();
                        client.bytesSent = 0;
                        client.state = Client::READING;
                        pollfds[i].events = POLLIN;
                    }
                }

                if (now - client.lastActivity > 30)
                {
                    close(fd);
                    clients.erase(fd);
                    pollfds.erase(pollfds.begin() + i);
                    i--;
                }
            }
        }
    }
}

Server::~Server()
{
    for (size_t i = 0; i < server_fds.size(); ++i)
        close(server_fds[i]);
}
