# include "Channel.hpp"

Channel::Channel(std::string name, std::vector<Client>::iterator it) 
{
    this->channel_name = name;
    addClient(it);
    std::vector<Client>::iterator client_it = clients_ch.begin();
    while (client_it != clients_ch.end())
    {
        if (client_it->getFd() == it->getFd())
           client_it->is_operator = true;
        client_it++;
    }
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

void Channel::setTopic(const std::string newTopic)
{
    this->topic = newTopic;
}

Channel::~Channel() {}
