#ifndef __SERVER_H
# define __SERVER_H

# include <netinet/in.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <unistd.h>
# include <fstream>
# include <arpa/inet.h>
# include <vector>
# include <iterator>
# include <sstream>
# include <string>
# include <iostream>
# include <fcntl.h>

# include "Client.hpp"
# include "Channel.hpp"
# include "Errors.h"

# define USAGE "Usage: ./ircserv <PORT> <PASS>"
# define BUFFER_SIZE 1024
# define QEUE 100

class Client;
class Channel;

class Server
{
	private:
		int					_serverfd, _maxfd;
		int					_c_id;
		struct  sockaddr_in _servaddr;
		char				_buff[BUFFER_SIZE];
		fd_set				_read_set, _write_set, _current;
		socklen_t			_len;
		std::vector<Client>	_clients;
		std::vector<Channel> channels;
		std::string			_hostname, _passwd;
		std::string			_date;

	public:
		Server();
		~Server();

		int init(int port, std::string pass);
		int Start();

		static void	arg_control(int ac, char **av);
		static int runner(Server &server, std::string port, std::string pass);

		std::vector<Client>::iterator	findClient(int fd);
		std::vector<Client>::iterator	findClientInCh(std::vector<Channel>::iterator it, int fd);
		std::vector<Client>::iterator	findClientNick(std::string &str);
		std::vector<Channel>::iterator	findChannel(std::string str);

		std::string getHostname() const;

		void    sendToClisInCh(std::vector<Channel>::iterator it, std::string msg,int fd);
		void	sendCl(std::string msg, int fd);
		void    parse_cl(int fd);
		bool    isAlNumStr(std::string str);
		void    handle_name(std::vector<std::string> &tokens);
		void    eraseClient(int fd, int flag);
	    void    eraseClientFromCh(std::vector<Channel>::iterator it, int fd);
		bool	isValidNick(std::vector<std::string>::iterator &tokens_it);
		void	sendReply(std::string msg, int fd);
		int		can_apply(std::vector<std::string>::iterator tokens_it, std::vector<Client>::iterator client_it);

        void	pass(std::vector<std::string> &tokens, int fd);
		void	nick(std::vector<std::string> &tokens, int fd);
		void	user(std::vector<std::string> &tokens, int fd);
		void	privmsg(std::vector<std::string> &tokens, int fd);
		void	join(std::vector<std::string> &tokens, int fd);
		void	kick(std::vector<std::string> &tokens, int fd);
		void	quit(std::vector<std::string> &tokens, int fd);
		void	topic(std::vector<std::string> &tokens, int fd);
		void	notice(std::vector<std::string> &tokens, int fd);
		void	part(std::vector<std::string> &tokens, int fd);
		void	cap(std::vector<std::string> &tokens, int fd);
		void	ping(std::vector<std::string> &tokens, int fd);

};

#endif
