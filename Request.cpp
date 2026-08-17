#include "Request.hpp"

Request::Request()
{
	this->autoIndex = false;
	this->index = false;
	this->fd = 0;
	this->iscgi = false;
}

Request::~Request()
{

}

bool Request::parseFirstLine(std::stringstream &ss, Client &client)
{
	std::string line;
	std::getline(ss, line);

	std::stringstream f1(line);
	std::string word;

	while (f1 >> word)
		this->vec.push_back(word);
	if (this->vec.size() != 3
	|| (this->vec[0] != "GET" && this->vec[0] != "POST" && this->vec[0] != "DELETE") 
	|| (this->vec[2] != "HTTP/1.0" && this->vec[2] != "HTTP/1.1"))
	{
		this->res.setCode("400");
		this->res.setMsg("Bad Request");
		this->res.make_res(client.server, *this, client.writeBuffer);
		return false;
	}
	this->method = this->vec[0];
	this->path = this->vec[1];
	this->protocol = this->vec[2];

	size_t find = this->path.find("?");
	if (find != std::string::npos)
	{
		this->query = this->path.substr(this->path.find("?") + 1);
		this->path = this->path.substr(0, this->path.find("?"));
		this->map.insert(std::make_pair("QUERY_STRING", this->query));
	}

	size_t len = this->path.rfind('/', this->path.length());
	if (len == 0)
		len++;
	this->dir = this->path.substr(0, len);
	return true;
}

void Request::parseHedear(std::stringstream &ss)
{
	std::string line;
	while (std::getline(ss, line))
	{
		if (line[0] == '\r')
			break ;
		std::stringstream ss1(line);
		std::string key;
		std::string val;
		std::getline(ss1, key, ':');
		std::getline(ss1, val);
		this->map.insert(std::make_pair(key, val));
	}
}

void Request::parser(std::stringstream &ss, std::stringstream &out, int fdo, Client &client)
{
	if (!this->parseFirstLine(ss, client))
		return ;
	std::vector<LocationConfig>::iterator it1 = client.server.locations.begin();
	std::vector<LocationConfig>::iterator it0 = client.server.locations.end();
	while (it1 != client.server.locations.end())
	{
		if (it1->path[0] != '/')
			it1->path = "/" + it1->path;
		if (this->dir == it1->path)
		{
			it0 = it1;
			break ;
		}
		it1++;
	}
	if (it0 == client.server.locations.end())
	{
		this->res.setCode("404");
		this->res.setMsg("Not Found");
		this->res.make_res(client.server, *this, client.writeBuffer);
		return;
	}
	else if (!it0->redirect.empty())
	{
		std::stringstream code;
		code << it0->redirect_code;
		this->res.setCode(code.str());
		this->res.setMsg("Moved Permanently");
		this->res.setRedirect(it0->redirect);
	}
	else
	{
		this->parseHedear(ss);
		if (this->path.find(it0->path) != std::string::npos)
			this->path.replace(this->path.find(it0->path), it0->path.size(), it0->root + "/");
		if (this->method == "GET")
			get(it0);
		if (this->method == "DELETE")
			del(it0);
		if (this->method == "POST")
			post(out, fdo, it0);
	}
	this->res.make_res(it0, *this, client.writeBuffer);
	return ;
}

void Request::get(std::vector<LocationConfig>::iterator &it0)
{
	if (std::find(it0->methods.begin(), it0->methods.end(), "GET") == it0->methods.end())
	{
		this->res.setCode("403");
		this->res.setMsg("Forbidden");
		return ;
	}
	std::string inter;
	struct stat sb;
	this->rpath = this->path;
	stat(this->rpath.c_str(), &sb);
	switch (sb.st_mode & S_IFMT) {
		case S_IFDIR:
			if (this->rpath[this->rpath.size() - 1] != '/')
			{
				this->res.setCode("301");
				this->res.setMsg("Moved Permanently");
			}
			else if (it0->index_files.size() > 0)
			{
				for (std::vector<std::string>::iterator itf = it0->index_files.begin(); itf != it0->index_files.end(); itf++)
				{
					std::string file = this->path + "/" + *itf;
					int fdf = open(file.c_str(), O_RDONLY);
					if (fdf > 0)
					{
						close(fdf);
						this->path = "/" + *itf;
						this->rpath = file;
						this->res.setCode("200");
						this->res.setMsg("OK");
						return ;
					}
				}
				this->res.setCode("404");
				this->res.setMsg("Not Found");
			}
			else if (it0->autoindex)
			{
				this->autoIndex = true;
				DIR *dir;
				dir = opendir(this->rpath.c_str());
				struct dirent *ent;
				if (dir)
				{
					ent = readdir(dir);
					while (ent)
					{
						this->folders.push_back(ent->d_name);
						ent = readdir(dir);
					}
					closedir(dir);
				}
				this->res.setCode("200");
				this->res.setMsg("OK");
			}
			else
			{
				this->res.setCode("403");
				this->res.setMsg("Forbidden");
			}
			break;
		case S_IFREG:
			this->fd = open(this->rpath.c_str(), O_RDONLY);
			if (this->fd == -1)
			{
				this->res.setCode("403");
				this->res.setMsg("Forbidden");
				break;
			}
			close(this->fd);
			inter = getInterpreter(this->rpath, *it0);
			if (!inter.empty())
			{
				this->res.setCode("200");
				this->res.setMsg("OK");
				this->iscgi = true;
				this->cgistr = cgiHandler(this->rpath, buildEnv(*this), inter, "", *this);
			}
			else
			{
				this->res.setCode("200");
				this->res.setMsg("OK");
			}
			break;
		default:
			if (errno == 2)
			{
				this->res.setCode("404");
				this->res.setMsg("Not Found");
			}
			else
			{
				this->res.setCode("403");
				this->res.setMsg("Forbidden");
			}
	}
}

void Request::fullBody(std::stringstream &out, int fdo)
{
	char buffer[1024];
	std::memset(buffer, 0, sizeof(buffer));
	int bytes = recv(fdo, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
	while (bytes > 0)
	{
		for (int i = 0; i < bytes; i++)
			out << buffer[i];
		std::memset(buffer, 0, sizeof(buffer));
		bytes = recv(fdo, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
	}
}

void Request::getBoundary()
{
	std::stringstream ct(this->map["Content-Type"]);
	ct >> this->boundary;
	ct >>	this->boundary;
	this->boundary.erase(0, boundary.find("=") + 1);
	this->boundary = "--" + this->boundary;
}

void  Request::postAction(std::stringstream &out, bool upload)
{
	std::string line;
	std::string filename;
	while (std::getline(out, line))
	{
		if (line.find(this->boundary + "--\r") != std::string::npos || line.empty())
			break ;
		else if (line.find("filename") != std::string::npos)
		{
			filename = line.substr(line.find("filename") + 10);
			filename.erase(filename.size() - 2);
		}
		else if (line.find("name") != std::string::npos && line.find("filename") == std::string::npos)
		{
			std::string key;
			key = line.substr(line.find("name") + 6);
			key.erase(key.size() - 2);
			std::getline(out, line);
			std::getline(out, line);
			if (line != "\r")
				line.erase(line.size() - 1);
			else
				line = "";
			std::string val = line;
			this->querys.insert(std::make_pair(key, val));
		}
		else if (line == "\r" && !filename.empty() && upload)
		{
			std::ofstream output(this->rpath + "/" + filename, std::ios::binary);
			std::stringstream out1;
			while (std::getline(out, line))
			{
				if (line.find(this->boundary + "\r") != std::string::npos || line.find(this->boundary + "--\r") != std::string::npos)
					break ;
				out1 << line << std::endl;
			}
			std::string s = out1.str();
			s.erase(s.size() - 2);
			output << s;
			filename = "";
		}
	}
}

void Request::post(std::stringstream &out, int fdo, std::vector<LocationConfig>::iterator &it0)
{
	if (std::find(it0->methods.begin(), it0->methods.end(), "POST") == it0->methods.end())
	{
		this->res.setCode("403");
		this->res.setMsg("Forbidden");
		return ;
	}

	std::map<std::string, std::string>::iterator itm = this->map.find("Content-Length");
	if (itm == this->map.end())
	{
		this->res.setCode("403");
		this->res.setMsg("Forbidden");
		return ;
	}

	if (std::atof(itm->second.c_str()) > std::atof(it0->client_max_body_size.c_str()))
	{
		this->res.setCode("413");
		this->res.setMsg("Request Entity Too Large");
		return ;
	}

	std::string inter;
	struct stat sb;
	this->rpath = this->path;
	stat(this->rpath.c_str(), &sb);
	switch (sb.st_mode & S_IFMT) {
		case S_IFDIR:
			if (this->rpath[this->rpath.size() - 1] != '/')
			{
				this->res.setCode("301");
				this->res.setMsg("Moved Permanently");
			}
			else
			{
				DIR *dir;
				dir = opendir(this->rpath.c_str());
				if (dir)
				{
					this->fullBody(out, fdo);
					this->getBoundary();
					this->postAction(out, it0->upload_enable);
					this->res.setCode("200");
					this->res.setMsg("OK");
					closedir(dir);
				}
			}
		case S_IFREG:
			this->fd = open(this->rpath.c_str(), O_RDONLY);
			if (this->fd == -1)
			{
				this->res.setCode("404");
				this->res.setMsg("Not Found");
				break;
			}
			close(this->fd);
			this->fullBody(out, fdo);
			this->getBoundary();
			this->postAction(out, false);
			inter = getInterpreter(this->rpath, *it0);
			if (!inter.empty())
			{
				this->res.setCode("200");
				this->res.setMsg("OK");
				this->iscgi = true;
				std::string b;
				for (std::map<std::string, std::string>::iterator it = this->querys.begin(); it != this->querys.end(); it++)
				{
					b += it->first;
					b += "=";
					b += it->second;
					b += "&";
				}
				this->cgistr = cgiHandler(this->rpath, buildEnv(*this), inter, b, *this);
			}
			break;
		default:
			this->res.setCode("403");
			this->res.setMsg("Forbidden");
	}
	std::stringstream ss;
	ss << out.str().size();
	this->res.cLength = ss.str();
	this->res.cType = "text/plain";
}

void Request::del(std::vector<LocationConfig>::iterator &it0)
{
	if (std::find(it0->methods.begin(), it0->methods.end(), "DELETE") == it0->methods.end())
	{
		this->res.setCode("403");
		this->res.setMsg("Forbidden");
		return ;
	}
	struct stat sb;
	this->rpath = this->path;
	stat(this->rpath.c_str(), &sb);
	switch (sb.st_mode & S_IFMT) {
		case S_IFREG:
			this->fd = open(this->rpath.c_str(), O_RDONLY);
			if (this->fd == -1)
			{
				this->res.setCode("404");
				this->res.setMsg("Not Found");
				break;
			}
			close(this->fd);
			std::remove(this->rpath.c_str());
			this->res.setCode("200");
			this->res.setMsg("OK");
			break;
		default:
			this->res.setCode("403");
			this->res.setMsg("Forbidden");
	}
}