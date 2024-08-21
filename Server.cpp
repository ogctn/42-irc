#include "Server.hpp"

Server::Server() {}

Server::~Server() {}


void	Server::arg_control(int ac, char **av)
{
	if(ac != 3)
			throw std::string(USAGE);
	std::string port = av[1];
	std::string pass = av[2];
	int port_num;
	if (pass.length() <= 0 || pass.find_first_of(" \r\n") != std::string::npos)
		throw std::string("error on PASS argument");
	if (port.length() <= 0 || port.find_first_of(" \r\n") != std::string::npos)
		throw std::string("error on PORT argument");

	for(size_t i = 0; i < port.length(); i++) {
		if (!std::isdigit(port[i]))
			throw std::string("error on PORT argument");
	}

	try {
		port_num = std::stoi(port);
	} catch(std::exception &e) {
		throw std::string(e.what());
	}

	if (port_num >= (1 << 16) || port_num < 1000)
		throw std::string("PORT argument must be in range of [1000 2^16]");

}

int	Server::runner(Server &server, std::string port, std::string pass)
{
	if( server.init(std::stoi(port), pass) )
		return(-1);

	return ( server.Start() );
}

int Server::init(int port, std::string pass)
{
	char name[1024];
	int opt = 1;

	_date = __DATE__;
	gethostname(name, sizeof(name));
	_hostname = name;
	_passwd = pass;
	_c_id = 0;
	memset(_buff, 0, sizeof(_buff));

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		return (perror("socket"), -1);
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0)
		return (perror("setsockopt"), -1);
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		return (perror("fcntl F_SETFL"), -1);
	_serverfd = fd;
	_maxfd = fd;

	FD_ZERO(&_current);
	FD_SET(_serverfd, &_current);

	memset(&_servaddr, 0, sizeof(_servaddr));
	_servaddr.sin_port = htons(port);
	_servaddr.sin_addr.s_addr = INADDR_ANY;
	_servaddr.sin_family = AF_INET;
	if ((bind(fd, (const sockaddr *)&_servaddr, sizeof(_servaddr))) == -1)
		return (-1);
	_len = sizeof(_servaddr);

	if (listen(_serverfd, QEUE) == -1)
		return (perror("listen"), -1);
	std::cout << "Server is listening on port: " << port << std::endl;

	return (0);
}

int Server::Start() {

	while (1) {

		_write_set = _read_set = _current;
		if (select(_maxfd + 1, &_read_set, &_write_set, 0, 0) < 0)
			return (-1);

		for (int fd = 0; fd <= _maxfd; ++fd) {

			if (FD_ISSET(fd, &_read_set)) {
				int clientfd, ret;
				if (fd == _serverfd) {
					if ((clientfd = accept(_serverfd, (struct sockaddr *)&_servaddr, &_len)) < 0)
						return (-1);

					++_c_id;
					Client client(clientfd, _c_id);
					_clients.push_back(client);
					std::cout << "Client " << _c_id << " joined the server" << std::endl;
					FD_SET(clientfd, &_current);
					if (_maxfd < clientfd)
						_maxfd = clientfd;
				}
				else {
					ret = recv(fd, _buff, sizeof(_buff), 0);
					if (ret <= 0) {
						if (ret == 0)
							std::cout << "Client " << findClient(fd)->getId() << " left the server" << std::endl;
						else
							std::cerr << "recv error\n";
						FD_CLR(fd, &_current);
						eraseClient(fd, 1);
						close(fd);
					}
					else if (ret == 1 && *_buff == '\n')
						continue;
					else {
						_buff[ret-1] = '\0';
						parse_cl(fd);
					}
				}
				memset(_buff, 0, sizeof(_buff));
			}
		}
	}
}


void read_from_buffer(std::vector<std::string> &lines, char *buff) {

	std::istringstream iss(buff);
	std::string line;

	while (std::getline(iss, line, '\n')) {
		if (line.length() == 0 || line == "\n")
			continue ;
		else if (line[line.length() - 1] == '\r')
			lines.push_back(line.substr(0, line.length() - 1));
		else
			lines.push_back(line);
	}
}

void	splitSpaces(std::vector<std::string> &tokens, std::string &str)
{
	std::string token;
	std::istringstream iss(str);

	while (std::getline(iss, token, ' '))
	{
		if (token.length() == 0 || token == "\n")
			continue ;
		tokens.push_back(token);
	}
}

void Server::parse_cl(int fd)
{
	std::vector<std::string> lines;
	read_from_buffer(lines, _buff);

	std::vector<Client>::iterator client_it = findClient(fd);
	std::vector<std::string>::iterator lines_it = lines.begin();
	while (lines_it != lines.end())
	{
		std::vector<std::string> tokens;
		splitSpaces(tokens, *lines_it);

		std::vector<std::string>::iterator tokens_it = tokens.begin();
		if (tokens_it == tokens.end())
		{
			++lines_it;
			continue;
		}
		if (*tokens_it == "PASS")
			pass(tokens, fd);
		else if (*tokens_it == "NICK")
			nick(tokens, fd);
		else if (*tokens_it == "USER")
			user(tokens, fd);
		else if (*tokens_it == "QUIT")
			quit(tokens,fd);
		else if (*tokens_it == "CAP")
			cap(tokens,fd);
		else if (*tokens_it == "PING")
			ping(tokens,fd);
        else if(!can_apply(tokens_it, client_it))
            sendCl(YELLOW(getHostname(), "PASS-NICK-USER before sending any other commands"), fd);
        else if (*tokens_it == "JOIN")
			join(tokens, fd);
		else if (*tokens_it == "MODE" || *tokens_it == "WHO")
			;
		else if (*tokens_it == "PRIVMSG")
			privmsg(tokens, fd);
		else if (*tokens_it == "KICK")
			kick(tokens, fd);
		else if (*tokens_it == "QUIT")
			quit(tokens, fd);
		else if (*tokens_it == "NOTICE")
			notice(tokens, fd);
		else if (*tokens_it == "PART")
			part(tokens, fd);
		else if (*tokens_it == "TOPIC")
			topic(tokens, fd);
		else
			sendCl(ERR_UNKNOWNCOMMAND(getHostname(), *tokens_it), fd);

		lines_it++;
	}
}

std::vector<Client>::iterator Server::findClient(int fd)
{
	std::vector<Client>::iterator it = _clients.begin();
	while (it != _clients.end())
    {
        if (it->getFd() == fd)
            return (it);
        it++;
    }
    return (_clients.end());
}

std::vector<Client>::iterator Server::findClientNick(std::string &str)
{
    std::vector<Client>::iterator it = _clients.begin();
	while (it != _clients.end())
	{
		if ((*it).getNick() == str)
			return it;
        it++;
	}
	return ( _clients.end() );
}

std::vector<Client>::iterator Server::findClientInCh(std::vector<Channel>::iterator it, int fd)
{
	std::vector<Client>::iterator client_it = it->clients_ch.begin();
	while (client_it != it->clients_ch.end())
	{
		if (client_it->getFd() == fd)
			return (client_it);
		client_it++;
	}
	return (client_it);
}

std::vector<Channel>::iterator Server::findChannel(std::string str)
{
   std::vector<Channel>::iterator channel_it = channels.begin();
   while (channel_it != channels.end())
   {
		if(channel_it->getName() == str)
			return(channel_it);
		channel_it++;
   }
	return (channel_it);
}

void Server::eraseClient(int fd, int flag)
{
    std::vector<Channel>::iterator channel_it = channels.begin();
	if(flag == 0)
		std::cout << "Client " << findClient(fd)->getId() << " left the server" << std::endl;
	while (channel_it != channels.end())
	{
		if(findClientInCh(channel_it,fd) != channel_it->clients_ch.end())
            channel_it->clients_ch.erase(findClientInCh(channel_it,fd));
		channel_it++;
	}
    _clients.erase(findClient(fd));

}

void Server::eraseClientFromCh(std::vector<Channel>::iterator it, int fd)
{
	if (it != channels.end())
	{
		if(findClientInCh(it,fd) != it->clients_ch.end())
            it->clients_ch.erase(findClientInCh(it,fd));
	}
}

void Server::sendToClisInCh(std::vector<Channel>::iterator it, std::string msg, int fd)
{
    std::vector<Client>::iterator client_it = it->clients_ch.begin();
    while (client_it != it->clients_ch.end())
    {
        if (client_it->getFd() != fd)
			sendCl(msg, client_it->getFd());//tekrar bakılacak
        client_it++;
    }
}

std::string Server::getHostname() const { return _hostname; }

void Server::sendReply(std::string msg, int fd)
{
    std::string reply = YELLOW(getHostname(), msg);
	send(fd, reply.c_str(), reply.length(), 0);
}

void Server::sendCl(std::string msg, int fd)
{
    send(fd, msg.c_str(), msg.length(), 0);
}

bool Server::isAlNumStr(std::string str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!std::isalnum(str[i]))
			return (false);
	}
	return (true);
}

void Server::handle_name(std::vector<std::string> &tokens)
{
	std::vector<std::string>::iterator it = tokens.begin();
	while (it != tokens.end())
	{
		if((*it)[0] == ':')
			break;
		it++;
	}
	if(it == tokens.end())
		return;
	while (it+1 != tokens.end())
	{
		(*it).append(" ");
		(*it).append((*(it + 1)));
		tokens.erase(it+1);
	}
}

int	Server::can_apply(std::vector<std::string>::iterator tokens_it, std::vector<Client>::iterator client_it)
{
	if((*tokens_it == "JOIN" || *tokens_it == "PRIVMSG" || *tokens_it == "KICK"
        || *tokens_it == "TOPIC" || *tokens_it == "NOTICE"
        || *tokens_it == "PART") && (!client_it->is_auth || !client_it->is_registered
        || client_it->getNick().length() == 0))
		return (0);
	else
		return (1);
}
