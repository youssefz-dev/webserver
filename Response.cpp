#include "Response.hpp"
#include "Request.hpp"

Response::Response()
{
	this->server = "webserv";
	this->cLength = "0";
}


Response::~Response()
{

}

void Response::autoIndexFun(std::string &data, Request& req)
{
	data = "<html>\n";
	data += "<body>\n";
	data += "<h1>index</h1>\n";
	for (std::vector<std::string>::iterator it = req.folders.begin(); it != req.folders.end(); it++)
		data += "<p><a href='/" + *it + "'>" + *it + "</a></p>\n";
	data += "</body>\n";
	data += "</html>\n";
	this->cType = "text/html";
	std::stringstream ss;
	ss << data.length();
	this->cLength = ss.str();
}

void Response::readFile(std::string &data, Request& req)
{
	std::ifstream inp(req.rpath.c_str(), std::ios::binary);
	std::string file((std::istreambuf_iterator<char>(inp)),std::istreambuf_iterator<char>());
	data = file;
	this->cType = this->getType(req.path);
	std::stringstream ss;
	ss << data.length();
	this->cLength = ss.str();
}

void Response::make_res(ServerConfig &server, Request& req, std::string &buff)
{
	std::string data1 = "";
	buff = req.protocol + " " + this->code + " " + this->msg + "\r\n";
	if (!this->location.empty())
		buff += "Location: " + this->location + "\r\n";
	buff += "Server: " + this->server + "\r\n";
	if (req.method == "GET" && this->code == "200")
	{
		if (req.autoIndex)
			this->autoIndexFun(data1, req);
		else if (req.iscgi)
		{
			data1 = req.cgistr;
			std::stringstream ss;
			ss << data1.length();
			this->cLength = ss.str();
			this->cType = "text/html";
		}
		else
			this->readFile(data1, req);
	}
	else if (this->code != "200")
	{
		this->cType = "text/html";
		std::map<int, std::string>::iterator itm = server.error_pages.find(std::atoi(this->code.c_str()));
		if (itm != server.error_pages.end())
		{
			std::ifstream inp(itm->second.c_str(), std::ios::binary);
			if (!inp)
				data1 = this->htmlPage();
			else
			{
				std::string file((std::istreambuf_iterator<char>(inp)),std::istreambuf_iterator<char>());
				data1 = file;
			}
		}
		else
			data1 = this->htmlPage();
		std::stringstream ss;
		ss << data1.length();
		this->cLength = ss.str();
	}
	if (req.method == "POST")
		this->cLength = "0";
	if (req.method == "POST" && req.iscgi)
	{
		data1 = req.cgistr;
		std::stringstream ss;
		ss << data1.length();
		this->cLength = ss.str();
		this->cType = "text/html";
	}
	if (req.method == "GET")
		buff += "Content-Type: " + this->cType + "\r\n";
	buff += "Content-Length: " + this->cLength + "\r\n";
	this->Cookie(req, buff);
	buff += "Connection: close\r\n";
	buff +=  "\r\n";
	if (!data1.empty())
		buff +=  data1;
}

void Response::make_res(std::vector<LocationConfig>::iterator &it0, Request& req, std::string &buff)
{
	std::string data1 = "";
	buff = req.protocol + " " + this->code + " " + this->msg + "\r\n";
	if (!this->location.empty())
		buff += "Location: " + this->location + "\r\n";
	buff += "Server: " + this->server + "\r\n";
	if (req.method == "GET" && this->code == "200")
	{
		if (req.autoIndex)
			this->autoIndexFun(data1, req);
		else if (req.iscgi)
		{
			data1 = req.cgistr;
			std::stringstream ss;
			ss << data1.length();
			this->cLength = ss.str();
			this->cType = "text/html";
		}
		else
			this->readFile(data1, req);
	}
	else if (this->code != "200")
	{
		this->cType = "text/html";
		std::map<int, std::string>::iterator itm = it0->error_pages.find(std::atoi(this->code.c_str()));
		if (itm != it0->error_pages.end())
		{
			std::ifstream inp(itm->second.c_str(), std::ios::binary);
			if (!inp)
				data1 = this->htmlPage();
			else
			{
				std::string file((std::istreambuf_iterator<char>(inp)),std::istreambuf_iterator<char>());
				data1 = file;
			}
		}
		else
			data1 = this->htmlPage();
		std::stringstream ss;
		ss << data1.length();
		this->cLength = ss.str();
	}
	if (req.method == "POST")
		this->cLength = "0";
	if (req.method == "POST" && req.iscgi)
	{
		data1 = req.cgistr;
		std::stringstream ss;
		ss << data1.length();
		this->cLength = ss.str();
		this->cType = "text/html";
	}
	if (req.method == "GET")
		buff += "Content-Type: " + this->cType + "\r\n";
	buff += "Content-Length: " + this->cLength + "\r\n";
	this->Cookie(req, buff);
	buff += "Connection: close\r\n";
	buff +=  "\r\n";
	if (!data1.empty())
		buff +=  data1;
}

void Response::Cookie(Request& req, std::string &buff)
{
	if (req.map.find("Cookie") != req.map.end())
		if (req.map.find("Cookie")->second.find("login=1"))
			buff += "Set-Cookie: sid=1\r\n";
}

std::string Response::htmlPage()
{
	std::string html;
	html = "<html>\n";
	html += "<head><title>" + this->code + " " + this->msg + "</title></head>\n";
	html += "<body>\n";
	html += "<center><h1>" + this->code + " " + this->msg + "</h1></center>\n";
	html += "<hr><center>" + this->server + "</center>\n";
	html += "</body>\n";
	html += "</html>\n";
	return html;
}

void Response::setCode(std::string str)
{
	this->code = str;
}

void Response::setMsg(std::string str)
{
	this->msg = str;
}

void Response::setBody(std::string str)
{
	this->body = str;
}

std::string Response::getType(std::string& str)
{
	if (str.find(".html") != std::string::npos)
		return "text/html";
	if (str.find(".pdf") != std::string::npos)
		return "application/pdf";
	if (str.find(".mp4") != std::string::npos)
		return "video/mp4";
	return "text/plain";
}

void Response::setRedirect(std::string &loc)
{
	this->location = loc;
}