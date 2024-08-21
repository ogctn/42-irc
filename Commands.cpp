#include "Server.hpp"

void Server::pass(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);

	if(tokens.size() == 2)
	{
		if(*(tokens_it + 1) != _passwd)
			sendCl(ERR_PASSWDMISMATCH(getHostname()), fd);
		else if (client_it->is_auth == true)
			sendCl(ERR_ALREADYREGISTERED(getHostname(), client_it->getNick()), fd);
		else
			client_it->is_auth = true;
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
    if (tokens.size() == 1)
    {
		sendReply("Client "+ std::to_string(findClient(fd)->getId())
					 + " left the server " + tokens[1] + "\r\n", fd);
        FD_CLR(fd, &_current);
        close(fd);
		eraseClient(fd);
    }
    else
		sendReply("Command form is: QUIT :<message>", fd);
}

void	Server::user(std::vector<std::string> &tokens, int fd)
{
	//user commandına channel içinde de değiştrime olucak mı veya değiştirilebilir bir şey mi
	//bir kere atanan bir şey mi
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
				ch_it->addClient(findClient(fd));
				sendToClisInCh(ch_it, RPL_JOIN(client_it->getNick(), client_it->getUname(), *(tokens_it + 1)), fd);
				sendCl(RPL_JOIN(client_it->getNick(), client_it->getUname(), *(tokens_it + 1)),fd);
				if (ch_it->getTopic().length() == 0)
					sendReply(RPL_NOTOPIC(client_it->getNick(), *(tokens_it + 1)),0);
				else
					sendReply(RPL_TOPIC(client_it->getNick(), *(tokens_it + 1), ch_it->getTopic()),fd);
				if (ch_it->clients_ch.size() > 0)
				{
					std::vector<Client>::iterator it = ch_it->clients_ch.begin();
					std::string users = it->getNick();
					it++;
					while (it != ch_it->clients_ch.end())
					{
						users += (" " + it->getNick());
						it++;
					}
					sendReply(RPL_USRS(client_it->getNick(), *(tokens_it + 1), users), fd);
					sendReply(RPL_EONL(client_it->getNick(), *(tokens_it + 1)), fd);
				}
				else
					sendReply(RPL_MODE(client_it->getNick(), client_it->getUname(), *(tokens_it + 1)), fd);
			}
			else
				sendReply("Already joined to this channel", fd);
		}
		else
		{
			Channel new_ch((*(tokens_it + 1)), findClient(fd));
			channels.push_back(new_ch);
			// sendReply(RPL_JOIN(client_it->getNick(), client_it->getUname(), *(tokens_it + 1)), fd);
			// sendReply(RPL_MODE(client_it->getNick(), client_it->getUname(), *(tokens_it + 1)), fd);
			// sendReply(RPL_NOTOPIC(client_it->getNick(), *(tokens_it + 1)), fd);

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
	if(tokens.size() == 3 && (*(tokens_it + 2))[0] == ':' && (*(tokens_it + 2)).length() >= 2)
	{
		*(tokens_it + 2) = (*(tokens_it + 2)).substr(1, (*(tokens_it + 2)).length() - 1);
		if ((*(tokens_it + 1))[0] == '#')
		{
			std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
			if (ch_it == channels.end())
				sendCl(ERR_NOSUCHNICK(_hostname, client_it->getNick()), fd);
			else if (findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
				sendReply("You are not in channel:" + ch_it->getName(), fd);
			else
				sendToClisInCh(ch_it, RPL_PRIV(client_it->getNick(), client_it->getUname(), ch_it->getName(), (*(tokens_it + 2))), fd);
		}
		else if (isValidNick(tokens_it))
			sendCl(ERR_NOSUCHNICK(_hostname, client_it->getNick()), fd);
		else
			sendReply(RPL_PRIVUS(client_it->getNick(), client_it->getUname(), (*(tokens_it + 1)), (*(tokens_it + 2))), findClientNick(*(tokens_it + 1))->getFd());
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
			sendCl(ERR_NOSUCHCHANNEL(_hostname, ch_it->getName()), fd);
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
			eraseClientFromCh(ch_it, clientkick_it->getFd());
			sendReply(KICK(client_it->getNick(), client_it->getUname(), ch_it->getName(), clientkick_it->getNick()), fd);
			sendToClisInCh(ch_it, KICK(client_it->getNick(), client_it->getUname(), ch_it->getName(), clientkick_it->getNick()), fd);
		}
	}
	else
		sendReply("Command form is: KICK <channel> <client>",fd);
}

void Server::topic(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);
	std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
	handle_name(tokens);
	if(tokens.size() == 3 && (*(tokens_it + 2))[0] == ':' && (*(tokens_it + 2)).length() >= 2
	&& (*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2)
	{
		*(tokens_it + 2) = (*(tokens_it + 2)).substr(1, (*(tokens_it + 2)).length() - 1);
		if(ch_it == channels.end())
			sendCl(ERR_NOSUCHCHANNEL(_hostname, ch_it->getName()), fd);
		else if(findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
			sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
		else if(findClientInCh(ch_it, fd)->is_operator == false)
			sendCl(ERR_CHANOPRIVSNEEDED(_hostname, ch_it->getName()), fd);
		else
			ch_it->setTopic(*(tokens_it + 2));
			sendToClisInCh(ch_it, TOPICCHANGED(client_it->getNick(), client_it->getUname(), tokens[1], tokens[2]), fd);
			sendReply(TOPICCHANGED(client_it->getNick(), client_it->getUname(), ch_it->getName(), ch_it->getTopic()), fd);
			sendReply(RPL_TOPIC(client_it->getNick(), ch_it->getName(), ch_it->getTopic()), fd);
	}
	else
		sendReply("Command form is: TOPIC <channel> <topic>",fd);

}

void Server::notice(std::vector<std::string> &tokens, int fd)
{
	//notice komutundan emin değilim sadece channela mı yapılıyor yoksa usera mı
	//cemal sadece usera yollananı yapmış
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);
	handle_name(tokens);

	if(tokens.size() == 3 && (*(tokens_it + 2))[0] == ':' && (*(tokens_it + 2)).length() >= 2)
	{
		*(tokens_it + 2) = (*(tokens_it + 2)).substr(1, (*(tokens_it + 2)).length() - 1);
		if ((*(tokens_it + 1))[0] == '#')
		{
			std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
			if (ch_it == channels.end())
				sendCl(ERR_NOSUCHCHANNEL(_hostname, ch_it->getName()), fd);
			else if (findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
				sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
			else
				sendToClisInCh(ch_it, NOTICE(client_it->getNick(), client_it->getUname(), *(tokens_it +1), *(tokens_it + 2)), fd); //burası düzenlenebilir
		}
		else if (isValidNick(tokens_it))
			sendCl(ERR_NOSUCHNICK(_hostname, client_it->getNick()), fd);
		else
			sendReply(NOTICE(client_it->getNick(), client_it->getUname(), *(tokens_it +1), *(tokens_it + 2)),
							 findClientNick(*(tokens_it + 1))->getFd());
	}
	else
		sendReply("Command form is: NOTICE <recipient> :<message>",fd);
}

void Server::part(std::vector<std::string> &tokens, int fd)
{
	std::vector<std::string>::iterator tokens_it = tokens.begin();
	std::vector<Client>::iterator client_it = findClient(fd);
	std::vector<Channel>::iterator ch_it = findChannel(*(tokens_it + 1));
	if((*(tokens_it + 1))[0] == '#' && (*(tokens_it + 1)).length() >= 2 && ((tokens.size() >= 3 && tokens[2][0] == ':') || tokens.size() == 2))
	{
		if (tokens.size() >= 3)
            handle_name(tokens);
		if(ch_it == channels.end())
			sendCl(ERR_NOSUCHCHANNEL(_hostname, ch_it->getName()), fd);
		else if(findClientInCh(ch_it, fd) == ch_it->clients_ch.end())
			sendCl(ERR_NOTONCHANNEL(_hostname, ch_it->getName()), fd);
		else
		{
			eraseClientFromCh(ch_it, fd);
			if(tokens.size() >= 3)
			{
				sendToClisInCh(ch_it, PARTWITHREASON(client_it->getNick(), client_it->getUname(), tokens[1], tokens[2]), fd);
				sendReply(PARTWITHREASON(client_it->getNick(), client_it->getUname(), tokens[1], tokens[2]), fd);

			}
			else
			{
				sendToClisInCh(ch_it , PART(client_it->getNick(), client_it->getUname(), tokens[1]), fd);
				sendReply(PART(client_it->getNick(), client_it->getUname(), tokens[1]), fd);
			}
		}
	}
	else
		sendReply("Command form is: PART <channel> :[<message>]",fd);
}
void Server::ping(std::vector<std::string> &tokens, int fd)
{
	handle_name(tokens);
	sendCl("PONG " + tokens[1] + "\r\n", fd);
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