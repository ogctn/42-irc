#include "Client.hpp"

Client::Client(int c_fd, int c_id) : fd(c_fd), id(c_id) {
    is_operator = false;
}

Client::~Client()
{
}

void Client::setNick(std::string c_nick)
{
    this->nick = c_nick;
}

void Client::setUname(std::string c_uname)
{
    this->uname = c_uname;
}

void Client::setRname(std::string c_rname)
{
    this->rname = c_rname;
}

std::string Client::getNick(void) const
{
    return this->nick;
}

std::string Client::getUname(void) const
{
    return this->uname;
}

std::string Client::getRname(void) const
{
    return this->rname;
}

int Client::getId(void) const
{
    return this->id;
}

int Client::getFd(void) const
{
    return this->fd;
}
