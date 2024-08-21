#include "Server.hpp"

void Server::pass(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);

	if(tokens.size() == 2)
	{
		if (client_it->is_auth == true)
			sendCl(ERR_ALREADYREGISTERED(getHostname(), client_it->getNick()), fd);
		else if(*(tokens_it + 1) != _passwd)
			sendCl(ERR_PASSWDMISMATCH(getHostname()), fd);
		else
		{
			sendReply("Password accepted", fd);
			client_it->is_auth = true;
		}
	}
	else
		sendCl(ERR_NEEDMOREPARAMS(client_it->getNick(), *(tokens.begin())), fd);
}

void Server::nick(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);

	if (tokens.size() == 2)
	{
		if (!isAlNumStr(*(tokens_it + 1)))
			sendCl(ERR_ERRONEUSNICKNAME(getHostname(), *(tokens_it + 1)), fd);
		else if (findClientNick(*(tokens_it + 1)) != _clients.end())
			sendCl(ERR_NICKNAMEINUSE(getHostname(), *(tokens_it + 1)), fd);
		else
		{
			std::vector<Channel>::iterator channel_it = channels.begin();
			std::vector<Client>::iterator client_it_ch;

			while (channel_it != channels.end())
			{
				if(findClientInCh(channel_it,fd) != channel_it->clients_ch.end())
				{
					client_it_ch = findClientInCh(channel_it,fd);
					client_it_ch->setNick(*(tokens_it + 1));
				}
				channel_it++;
			}
			sendReply("Nickname has been set as: " + *(tokens_it + 1), fd);
            client_it->setNick(*(tokens_it + 1));
		}
	}
	else
		sendReply("Command form is: NICK <nickname>", fd);
}

void Server::quit(std::vector<std::string> &tokens, int fd)
{
	handle_name(tokens);
    if (tokens.size() == 1)
    {
		sendReply("Client "+ std::to_string(findClient(fd)->getId())
					 + " left the server ", fd);
        FD_CLR(fd, &_current);
		eraseClient(fd, 0);
        close(fd);
    }
	else if (tokens.size() == 2 && tokens[1][0] == ':')
	{
		sendReply("Client "+ std::to_string(findClient(fd)->getId())
					 + " left the server " + tokens[1], fd);
		FD_CLR(fd, &_current);
		eraseClient(fd, 0);
		close(fd);
	}
    else
		sendReply("Command form is: QUIT :<message>", fd);
}

void	Server::user(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);

	handle_name(tokens);
	if (tokens.size() == 5 && (*(tokens_it + 4))[0] == ':' && (*(tokens_it + 4)).length() >= 2)
	{
		*(tokens_it + 4) = (*(tokens_it + 4)).substr(1, sizeof(*(tokens_it + 4)) - 1);
		if (client_it->getUname().empty() == false || client_it->getRname().empty() == false)
			sendCl(ERR_ALREADYREGISTERED(getHostname(), client_it->getNick()), fd);
		else
		{
			client_it->setUname(*(tokens_it + 1));
			client_it->setRname(*(tokens_it + 4));
			client_it->is_registered = true;

			sendCl(RPL_WELCOME(client_it->getNick(), client_it->getUname(), _hostname),fd);
			sendCl(RPL_YOURHOST(client_it->getNick(), _hostname),fd);
			sendCl(RPL_CREATED(client_it->getNick(), _hostname, _date),fd);
		}
	}
	else
		sendReply("Command form is: USER <username> <mode> <unused> :realname",fd);
}
std::string Prefix(std::vector<Client>::iterator users, std::string host)
{
    return ":" + users->getNick() + "!" + users->getUname() + "@" + host;
}
void Server::join(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);
	if(tokens.size() == 2 && (*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2)
	{
		std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
		if(ch_it != channels.end())
		{
			if(findClientInCh(ch_it,fd) == ch_it->clients_ch.end())
			{
				sendCl((Prefix(client_it, _hostname) + " JOIN " + (*(tokens_it + 1))  + "\r\n"), fd);
				std::vector<Client>::iterator cl_it = ch_it->clients_ch.begin();
				while (cl_it != ch_it->clients_ch.end())
    			{
					sendCl((Prefix(cl_it, _hostname) + " JOIN " + (*(tokens_it + 1)) + "\r\n"), fd);
    			    cl_it++;
    			}
				ch_it->addClient(findClient(fd));
				sendToClisInCh(ch_it, (Prefix(client_it, _hostname) + " JOIN " + (*(tokens_it + 1)) + "\r\n"), fd);
				if (ch_it->getTopic().length() == 0)
					sendCl(RPL_NOTOPIC(_hostname,client_it->getNick(), *(tokens_it + 1)),fd);
				else
					sendCl(RPL_TOPIC(_hostname, client_it->getNick(), *(tokens_it + 1), ch_it->getTopic()),fd);
			}
			else
				sendReply("Already joined to this channel", fd);
		}
		else
		{
			Channel new_ch((*(tokens_it + 1)), findClient(fd));
			channels.push_back(new_ch);
			sendCl((Prefix(findClient(fd), _hostname) + " JOIN " + (*(tokens_it + 1)) + "\r\n"), fd);
        	sendCl( Prefix(findClient(fd), _hostname) + " MODE " + (*(tokens_it + 1)) + " +o " + client_it->getNick() + "\r\n", fd);
		}
	}
	else
		sendReply("Command form is: JOIN #channel",fd);
}

bool Server::isValidNick(std::vector<std::string>::iterator &tokens_it) {
	return (findClientNick(*(tokens_it + 1)) == _clients.end() ||
			findClientNick(*(tokens_it + 1))->is_registered == false ||
			findClientNick(*(tokens_it + 1))->is_auth == false );
}

void Server::privmsg(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);

	handle_name(tokens);
	std::string msg =*(tokens_it) + " " + (*(tokens_it + 1))+ " " + (*(tokens_it + 2));
	if(tokens.size() == 3 && (*(tokens_it + 2))[0] == ':' && (*(tokens_it + 2)).length() >= 2)
	{
		*(tokens_it + 2) = (*(tokens_it + 2)).substr(1, (*(tokens_it + 2)).length() - 1);
		if ((*(tokens_it + 1))[0] == '#')
		{
			std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
			if (ch_it == channels.end())
				sendCl(ERR_NOSUCHNICK(_hostname, client_it->getNick()), fd);
			else if (findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
				sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
			else
				sendToClisInCh(ch_it, Prefix(client_it, _hostname) + " " + msg + "\r\n", fd);
		}
		else if (isValidNick(tokens_it))
			sendCl(ERR_NOSUCHNICK(_hostname, client_it->getNick()), fd);
		else
			sendCl(Prefix(client_it, _hostname) + " " + msg + "\r\n", findClientNick(*(tokens_it + 1))->getFd());
	}
	else
		sendReply("Command form is: PRIVMSG <recipient> :<message>",fd);
}

void Server::kick(std::vector<std::string> &tokens, int fd)
{

	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);
	std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
	if(tokens.size() == 3 && (*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2)
	{
		std::vector<Client>::iterator clientkick_it = findClientNick((*(tokens_it + 2)));
		if(ch_it == channels.end())
			sendCl(ERR_NOSUCHCHANNEL(_hostname, (*(tokens_it + 1))), fd);
		else if(findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
			sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
		else if(findClientInCh(ch_it, fd)->is_operator == false)
			sendCl(ERR_CHANOPRIVSNEEDED(_hostname, ch_it->getName()), fd);
		else if(clientkick_it == _clients.end())
			sendCl(ERR_NOSUCHNICK(_hostname, client_it->getNick()), fd);
		else if(findClientInCh(ch_it, clientkick_it->getFd()) == ch_it->clients_ch.end())
			sendCl(ERR_USERNOTINCHANNEL(_hostname, client_it->getNick(), ch_it->getName()), fd);
		else
		{
			std::string msg = *(tokens_it) + " " + (*(tokens_it + 1)) + " " + (*(tokens_it + 2));
			sendToClisInCh(ch_it, Prefix(client_it, _hostname) + " " + msg + "\r\n", fd);
			sendCl(Prefix(client_it, _hostname) + " " + msg + "\r\n", fd);
			eraseClientFromCh(ch_it, clientkick_it->getFd());
		}
	}
	else
		sendReply("Command form is: KICK <channel> <client>",fd);
}

void Server::topic(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
	handle_name(tokens);
	if(tokens.size() == 3 && (*(tokens_it + 2))[0] == ':' && (*(tokens_it + 2)).length() >= 2
	&& (*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2)
	{
		*(tokens_it + 2) = (*(tokens_it + 2)).substr(1, (*(tokens_it + 2)).length() - 1);
		if(ch_it == channels.end())
			sendCl(ERR_NOSUCHCHANNEL(_hostname, *(tokens_it + 1)), fd);
		else if(findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
			sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
		else if(findClientInCh(ch_it, fd)->is_operator == false)
			sendCl(ERR_CHANOPRIVSNEEDED(_hostname, ch_it->getName()), fd);
		else
		{
			ch_it->setTopic(*(tokens_it + 2));
    		std::vector<Client>::iterator cl_it = ch_it->clients_ch.begin();
    		while (cl_it != ch_it->clients_ch.end())
    		{
				sendCl(RPL_TOPIC(_hostname,cl_it->getNick(), ch_it->getName(), ch_it->getTopic()), cl_it->getFd());
    		    cl_it++;
    		}
		}
	}
	else
		sendReply("Command form is: TOPIC <channel> :<topic>",fd);

}

void Server::notice(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
	handle_name(tokens);

	if(tokens.size() == 3 && (*(tokens_it + 2))[0] == ':' && (*(tokens_it + 2)).length() >= 2)
	{
		if((*(tokens_it + 1))[0] == '#')
		{
			*(tokens_it + 2) = (*(tokens_it + 2)).substr(1, (*(tokens_it + 2)).length() - 1);
			if (ch_it == channels.end())
				sendCl(ERR_NOSUCHCHANNEL(_hostname, *(tokens_it + 1)), fd);
			else if (findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
				sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
			else
			{

				sendToClisInCh(ch_it, ":ADMIN!ADMIN@ADMIN NOTICE "
				+ *(tokens_it + 1) + " " + *(tokens_it + 2) + "\r\n", fd);
				sendCl(":ADMIN!ADMIN@ADMIN NOTICE "
				+ *(tokens_it + 1) + " " + *(tokens_it + 2) + "\r\n", fd);
			}
		}
	}
	else
		sendReply("Command form is: NOTICE <recipient> :<message>",fd);
}

void Server::part(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
	if((*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2 && ((tokens.size() >= 3 && tokens[2][0] == ':') || tokens.size() == 2))
	{
		if (tokens.size() >= 3)
			handle_name(tokens);
		if(ch_it == channels.end())
			sendCl(ERR_NOSUCHCHANNEL(_hostname, *(tokens_it + 1)), fd);
		else if(findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
			sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
		else
		{
			if(tokens.size() >= 3)
			{
				sendCl((Prefix(findClient(fd), _hostname) + " PART " + ch_it->getName() + "\r\n"), fd);
				sendToClisInCh(ch_it, (Prefix(findClient(fd), _hostname) + " PART " + ch_it->getName() + " " + tokens[2] + "\r\n"), fd);
			}
			else
			{
				sendCl((Prefix(findClient(fd), _hostname) + " PART " + ch_it->getName() + "\r\n"), fd);
				sendToClisInCh(ch_it, (Prefix(findClient(fd), _hostname) + " PART " + ch_it->getName() + "\r\n"), fd);
			}
			eraseClientFromCh(ch_it, fd);
		}
	}
	else
		sendReply("Command form is: PART <channel> :[<message>]",fd);
}

void Server::ping(std::vector<std::string> &tokens, int fd)
{
	handle_name(tokens);
	if(tokens.size() == 2 && tokens[1][0] != ':')
		sendReply("PONG :" + tokens[1] + "\r\n", fd);
	else
		sendReply("PONG\r\n", fd);
}

void Server::cap(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
    if(*(tokens_it + 1) == "LS"){
        std::string capLsCommand = "CAP * LS :multi-prefix sasl\r\n";
		sendReply(capLsCommand, fd);
    }
    else if (*(tokens_it + 1) == "REQ") {
        std::string reqParameter = *(tokens_it + 2);
        if (reqParameter == ":multi-prefix") {
            std::string capAckCommand = "CAP * ACK :multi-prefix\r\n";
            sendReply(capAckCommand, fd);
        }
    }
}
