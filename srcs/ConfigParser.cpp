#include "../headers/ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename) : config_file(filename)
{
}

ConfigParser::~ConfigParser()
{
}

std::string ConfigParser::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> ConfigParser::split(const std::string& str)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string word;

    while (ss >> word)
    {
        tokens.push_back(word);
    }
    return tokens;
}

bool check_server(const std::string& line)
{
    if (line.compare(0, 6, "server") != 0)
        return false;

    if (line.length() > 6 &&
        line[6] != ' ' &&
        line[6] != '\t' &&
        line[6] != '{')
        return false;

    return true;
}


bool check_location(const std::string& line)
{
    if (line.compare(0, 8, "location") != 0)
        return false;
    if (line.length() > 8 && line[8] != ' ' && line[8] != '\t')
        return false;
    return true;
}

bool ConfigParser::is_valid_method(const std::string& method)
{
    return (method == "GET" || method == "POST" || method == "DELETE");
}

int ConfigParser::parse_error_code(const std::string& code)
{
    for (size_t i = 0; i < code.length(); i++)
    {
        if (!isdigit(code[i]))
            throw std::runtime_error("Invalid error code: " + code);
    }
    int error_code = atoi(code.c_str());
    if (error_code < 100 || error_code > 599)
        throw std::runtime_error("Error code out of range: " + code);
    return error_code;
}

size_t ConfigParser::parse_size(const std::string& size_str)
{
    if (size_str.empty())
        throw std::runtime_error("Empty size value");
    
    std::string num_part;
    char multiplier = '\0';
    
    for (size_t i = 0; i < size_str.length(); i++)
    {
        if (isdigit(size_str[i]))
            num_part += size_str[i];
        else if (i == size_str.length() - 1)
        {
            multiplier = tolower(size_str[i]);
            break;
        }
        else
            throw std::runtime_error("Invalid size format: " + size_str);
    }
    
    if (num_part.empty())
        throw std::runtime_error("Invalid size format: " + size_str);
    
    size_t size = atol(num_part.c_str());
    
    if (multiplier == 'k')
        size *= 1024;
    else if (multiplier == 'm')
        size *= 1024 * 1024;
    else if (multiplier == 'g')
        size *= 1024 * 1024 * 1024;
    else if (multiplier != '\0')
        throw std::runtime_error("Invalid size multiplier: " + std::string(1, multiplier));
    
    return size;
}

void ConfigParser::parse_directive(const std::string& line, ServerConfig& config)
{
    std::vector<std::string> tokens = split(line);
    
    if (tokens.empty())
        return;
    
    std::string directive = tokens[0];
    
    bool directive_has_semicolon = false;
    if (!directive.empty() && directive[directive.length() - 1] == ';')
    {
        directive = directive.substr(0, directive.length() - 1);
        directive_has_semicolon = true;
    }
    
    if (directive == "listen")
    {
        if (directive_has_semicolon && tokens.size() == 1)
            throw std::runtime_error("listen directive requires a value");
        if (tokens.size() < 2)
            throw std::runtime_error("listen directive requires a value");

        std::string value = tokens[1];
        if (!value.empty() && value[value.length() - 1] == ';')
            value = value.substr(0, value.length() - 1);
        if (value.empty())
            throw std::runtime_error("listen directive requires a port or host:port");

        std::string host = "0.0.0.0";
        int port = -1;

        size_t colon_pos = value.find(':');
        if (colon_pos != std::string::npos)
        {
            host = value.substr(0, colon_pos);
            port = atoi(value.substr(colon_pos + 1).c_str());
        }
        else
        {
            port = atoi(value.c_str());
            if (port == 0)
            {
                throw std::runtime_error("listen directive: '" + value + "' is not a valid port or host:port");
            }
        }

        if (port <= 0 || port > 65535)
            throw std::runtime_error("listen directive: invalid port number: " + value);

        config.addListen(host, port);
		std::stringstream ss;
		ss << port;
		config.hosts.push_back(host+":"+ss.str());
    }
    else if (directive == "root")
    {
        if (tokens.size() < 2)
            throw std::runtime_error("root directive requires a value");
        std::string value = tokens[1];
        if (value[value.length() - 1] == ';')
            value = value.substr(0, value.length() - 1);
        config.setRoot(value);
    }
    else if (directive == "index")
    {
        if (tokens.size() < 2)
            throw std::runtime_error("index directive requires at least one value");
        for (size_t i = 1; i < tokens.size(); i++)
        {
            std::string value = tokens[i];
            if (value[value.length() - 1] == ';')
                value = value.substr(0, value.length() - 1);
            if (!value.empty())
                config.addIndexFile(value);
        }
    }
    else if (directive == "autoindex")
    {
        if (tokens.size() < 2)
            throw std::runtime_error("autoindex directive requires a value");
        
        std::string value = tokens[1];
        if (value[value.length() - 1] == ';')
            value = value.substr(0, value.length() - 1);
        
        if (value == "on")
            config.setAutoindex(true);
        else if (value == "off")
            config.setAutoindex(false);
        else
            throw std::runtime_error("autoindex must be 'on' or 'off'");
    }
    else if (directive == "error_page")
    {
        if (tokens.size() < 3)
            throw std::runtime_error("error_page directive requires error code(s) and path");
        
        std::string path = tokens[tokens.size() - 1];
        if (path[path.length() - 1] == ';')
            path = path.substr(0, path.length() - 1);
        
        for (size_t i = 1; i < tokens.size() - 1; i++)
        {
            int error_code = parse_error_code(tokens[i]);
            config.setErrorPage(error_code, path);
        }
    }
    else if (directive == "client_max_body_size")
    {
		if (tokens.size() < 2)
			throw std::runtime_error("client_max_body_size directive requires a value");
        std::string value = tokens[1];
        if (value[value.length() - 1] == ';')
            value = value.substr(0, value.length() - 1);
        
        size_t size = parse_size(value);
		std::stringstream ss;
		ss << size;
        config.setClientMaxBodySize(ss.str());
    }
    else if (directive == "allow_methods" || directive == "methods")
    {
        if (tokens.size() < 2)
            throw std::runtime_error("allow_methods directive requires at least one method");
        
        for (size_t i = 1; i < tokens.size(); i++)
        {
            std::string method = tokens[i];
            if (method[method.length() - 1] == ';')
                method = method.substr(0, method.length() - 1);
            
            if (!is_valid_method(method))
                throw std::runtime_error("Invalid HTTP method: " + method);
            
            config.addMethod(method);
        }
    }
}

void ConfigParser::parse_location_directive(const std::string& line, LocationConfig& config)
{
    std::vector<std::string> tokens = split(line);
    
    if (tokens.empty())
        return;
    
    std::string directive = tokens[0];
    bool directive_has_semicolon = false;
    if (!directive.empty() && directive[directive.length() - 1] == ';')
    {
        directive = directive.substr(0, directive.length() - 1);
        directive_has_semicolon = true;
    }
    if (directive == "return" || directive == "redirect")
    {
		if (directive_has_semicolon && tokens.size() == 1)
            throw std::runtime_error("redirect directive requires a value");
        if (tokens.size() < 2)
            throw std::runtime_error("redirect directive requires a value");
        if (tokens.size() >= 2)
        {
            std::string first_val = tokens[1];
            if (first_val[first_val.length() - 1] == ';')
                first_val = first_val.substr(0, first_val.length() - 1);
            bool is_code = true;
            for (size_t i = 0; i < first_val.length(); i++)
            {
                if (!isdigit(first_val[i]))
                {
                    is_code = false;
                    break;
                }
            }
            
            if (is_code)
            {
                int code = atoi(first_val.c_str());
                if (code >= 300 && code < 400)
                    config.setRedirectCode(code);
                else
                    throw std::runtime_error("invalid return code : "+first_val);
                std::string value = tokens[2];
                if (value[value.length() - 1] == ';')
                    value = value.substr(0, value.length() - 1);
                config.setRedirect(value);
                return;
            }
            throw std::runtime_error("invalid return code : "+first_val);
        }
    }
    else if (directive == "upload_enable")
    {
        if (tokens.size() < 2)
            throw std::runtime_error("upload_enable directive requires a value");
        
        std::string value = tokens[1];
        if (value[value.length() - 1] == ';')
            value = value.substr(0, value.length() - 1);
        
        if (value == "on")
            config.setUploadEnable(true);
        else if (value == "off")
            config.setUploadEnable(false);
        else
            throw std::runtime_error("upload_enable must be 'on' or 'off'");
    }
    else if (directive == "cgi_pass")
    {
        if (tokens.size() < 3)
            throw std::runtime_error("cgi_pass directive requires extension and interpreter path");
        
        std::string extension = tokens[1];
        std::string interpreter = tokens[2];
        if (interpreter[interpreter.length() - 1] == ';')
            interpreter = interpreter.substr(0, interpreter.length() - 1);
        
        config.addCgiHandler(extension, interpreter);
    }
    else
    {
        parse_directive(line, config);
    }
}

LocationConfig ConfigParser::parse_location(ServerConfig& server, const std::vector<std::string>& lines, size_t start, size_t end)
{
    if (start >= lines.size())
        throw std::runtime_error("Invalid location block");
    
    std::vector<std::string> tokens = split(lines[start]);
    
    if (tokens.size() < 2)
        throw std::runtime_error("location directive requires a path");
    
    std::string path = tokens[1];
    if (path == "{" && tokens.size() > 2)
    {
        throw std::runtime_error("location directive requires a path before '{'");
    }
    if (!path.empty() && path[path.length() - 1] == '{')
        path = path.substr(0, path.length() - 1);
    
    path = trim(path);
    
    if (path.empty())
        throw std::runtime_error("location directive requires a path");
    
    LocationConfig location(path, server);
    
    int brace_count = 0;
    bool found_open = false;
    
    for (size_t i = start; i <= end && i < lines.size(); i++)
    {
        std::string line = lines[i];
        
        if (line.find('{') != std::string::npos)
        {
            brace_count++;
            found_open = true;
            if (i == start)
                continue;
        }
        
        if (line.find('}') != std::string::npos)
        {
            brace_count--;
            if (brace_count == 0)
                break;
            continue;
        }
        
        if (found_open && brace_count > 0)
        {
            parse_location_directive(line, location);
        }
    }
    
    return location;
}

ServerConfig ConfigParser::parse_server(const std::vector<std::string>& lines, size_t start, size_t end)
{
    ServerConfig server;
    
    int brace_count = 0;
    bool in_location = false;
    size_t location_start = 0;
    
    for (size_t i = start; i <= end && i < lines.size(); i++)
    {
        std::string line = lines[i];
		if (check_server(line))
			throw std::runtime_error("server inside server");
        
        if (check_location(line))
        {
            in_location = true;
            location_start = i;
            if (line.find('{') != std::string::npos)
                brace_count = 1;
            else
                brace_count = 0;
            continue;
        }
        
        if (in_location)
        {
            if (line.find('{') != std::string::npos)
                brace_count++;
            
            if (line.find('}') != std::string::npos)
            {
                brace_count--;
                if (brace_count == 0)
                {
                    server.addLocation(parse_location(server, lines, location_start, i));
                    in_location = false;
                }
            }
            continue;
        }
        
        if (line.find('{') != std::string::npos || line.find('}') != std::string::npos)
            continue;
        
        parse_directive(line, server);
    }
	if (server.getListen().empty())
		server.addListen("0.0.0.0", 80);
    
    return server;
}

bool ConfigParser::parse()
{
	std::ifstream in(config_file.c_str());
    if (!in.is_open())
		throw std::runtime_error("Config file not found: " + config_file);
	
	std::vector<std::string> all_data;
    std::string line;
    int line_number = 0;

    while (std::getline(in, line))
    {
        line_number++;
        std::string cleaned = trim(line);
        
        if (cleaned.empty() || cleaned[0] == '#')
            continue;
        
        size_t hash_pos = cleaned.find('#');
        if (hash_pos != std::string::npos)
        {
            cleaned = trim(cleaned.substr(0, hash_pos));
            if (cleaned.empty()) 
                continue;
        }
        
        char last_char = cleaned[cleaned.length() - 1];
        if (last_char != ';' && last_char != '{' && last_char != '}')
		{
			if (cleaned == "server" || cleaned.rfind("location", 0) == 0)
				;
			else
			{
				std::cerr << "Error on line " << line_number
						<< ": Missing semicolon or invalid syntax." << std::endl;
				return false;
			}
		}
        all_data.push_back(cleaned);
    }

    in.close();
    
	if (all_data.empty())
        throw std::runtime_error("Config file is empty");

    if (!check_server(all_data[0]))
        throw std::runtime_error("Config error: First directive must be 'server {'");
    
    int open_braces = 0;
    for (size_t i = 0; i < all_data.size(); i++)
    {
        if (all_data[i].find('{') != std::string::npos)
            open_braces++;
        if (all_data[i].find('}') != std::string::npos)
            open_braces--;
    }
    
    if (open_braces != 0)
		throw std::runtime_error("Syntax Error: Unclosed curly braces");

	size_t server_index = 0;
    for (size_t i = 0; i < all_data.size(); i++)
    {
        if (check_server(all_data[i]))
        {
            server_index++;
            size_t j = i;
            int server_braces = 0;
            bool found_open = false;

            for (; j < all_data.size(); j++)
            {
                if (all_data[j].find('{') != std::string::npos)
                {
                    server_braces++;
                    found_open = true;
                }
                if (all_data[j].find('}') != std::string::npos)
                {
                    server_braces--;
                    if (server_braces == 0)
                        break;
                }
            }
            if (!found_open)
                throw std::runtime_error("server block : missing opening '{'");
            if (server_braces != 0)
                throw std::runtime_error("server block : missing closing '}'");

            try
            {
                ServerConfig server = parse_server(all_data, i + 1, j - 1);
                servers.push_back(server);
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error(e.what());
            }

            i = j;
        }
    }
	return true;
}

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
    return servers;
}