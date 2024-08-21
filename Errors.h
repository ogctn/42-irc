#ifndef __ERRORS_H
#define __ERRORS_H

#define RPL_WELCOME(nick, user, host) (":" + host + " 001 " + nick + " :Welcome to the Internet Relay Network " + nick + "!" + user + "@" + host + "\r\n")
#define RPL_YOURHOST(nick, host) (":" + host + " 002 " + nick + " :Your host is " + host + ", running version v1.0.0\r\n")
#define RPL_CREATED(nick, host, date) (":" + host + " 003 " + nick + " :This server was created " + date + "\r\n")

#define RPL_NOTOPIC(hostname, nickName, channel) (std::string(":") + hostname + " 331 " + nickName + " " + channel + " :No topic is set" + "\r\n")
#define RPL_TOPIC(hostname, nickName, channel, topic) (std::string(":") + hostname + " 332 " + nickName + " " + channel + " :" + topic + "\r\n")
#define RPL_JOIN(nick, user, channel) (":" + nick + "!" + user + "@*" + " JOIN " + channel)
#define RPL_PRIV(nick, user, channel, msg) (":" + nick + "!" + user + "@*" + " PRIVMSG " + channel + " :" + msg)
#define RPL_PRIVUS(nick, user, receiver, msg) (":" + nick + "!" + user + "@*" + " PRIVMSG " + receiver + " :" + msg)
#define RPL_MODE(nick, user, channel) (":" + nick + "!" + user + "@*" + " MODE " + channel + " +o " + nick)
#define RPL_USRS(nick, channel, users) (" 353 " + nick + " = " + channel + " :@" + users)
#define RPL_EONL(nick, channel) (" 366 " + nick + " " + channel + " :End of /NAMES list")
#define RPL_NICK(nick, user, new_nick) (":" + nick + "!" + user + "@* NICK " + new_nick)
#define KICK(nick, user, channel, kicked) (":" + nick + "!" + user + "@* KICK " + channel + " " + kicked)
#define NOTICE(nick, user, noticed, msg) (":" + nick + "!" + user + "@* NOTICE " + noticed + " " + msg)

#define NICKNAME_IN_USE(nickname) (" 433 " + nickname + " :Nickname is already in use. Please provide your password with 'USERPASS: <password> if you have this nickname already!'")

#define TOPICCHANGED(nickname, username, channelname, topic) (":" + nickname + "!" + username + "@* TOPIC " + channelname + " " + topic)
#define PART(hostname, nickname, username, channelname) (": "+ hostname + " " + nickname + " ! " + username + " @* PART " + channelname)
#define PARTWITHREASON(hostname, nickname, username, channelname, reason) (": " + hostname + " " + nickname + " ! " + username + " @* PART " + channelname + " " + reason)


#define ERR_USERNOTINCHANNEL(host, notyournick, channel) (": 441 " + host + " " + channel + " " + notyournick + " :They aren't on that channel\r\n")
#define ERR_CHANOPRIVSNEEDED(host, channel) (" 482 " + host + " " + channel + " :You're not channel operator\r\n")
#define ERR_NOTONCHANNEL(host, channel) (": 442 " + host + " " + channel + " :You're not on that channel\r\n")
#define ERR_NOSUCHCHANNEL(host, channel) (": 403 " + host + " " + channel + " :No such channel\r\n")
#define ERR_NOSUCHNICK(host, nick) (": 401 " + host + " " + nick + " :No such nick/channel\r\n")
#define ERR_ALREADYREGISTERED(host, nick) (" 462 " + host + " :" + nick +  " :You may not reregister\r\n")
#define ERR_PASSWDMISMATCH(host) (": 464 " + host + " :Password incorrect\r\n")
#define ERR_NEEDMOREPARAMS(host, command) (" 461 " + host + " " + command + " :Not enough parameters\r\n")
#define ERR_UNKNOWNCOMMAND(host, command) (": 421 " + host + " " + command + " :\r\n")
#define ERR_ERRONEUSNICKNAME(host, nick) (": 432 " + host + " :" + nick + " :Erroneus nickname\r\n")
#define YELLOW(host, msg) (": 377 " + host + " :" + msg + "\r\n")
#define ERR_NICKNAMEINUSE(host, nick) (": 377 " + host + " :" + nick + " :Nickname is already in use\r\n")


#endif

