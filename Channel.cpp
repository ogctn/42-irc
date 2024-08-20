# include "Channel.hpp"

Channel::Channel(std::string name, std::vector<Client>::iterator it) 
{
    this->channel_name = name;
    addClient(it);
    it->is_operator = true;
}

std::string Channel::getTopic() const
{
    return (topic);
}

void Channel::addClient(std::vector<Client>::iterator it)
{
    clients_ch.push_back(*it);

}

std::string Channel::getName() const
{
    return (channel_name);
}

Channel::~Channel() {}
