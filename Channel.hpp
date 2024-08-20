#ifndef __CHANNEL_HPP
# define __CHANNEL_HPP

# include "Client.hpp"
# include <vector>

class Client;

class Channel
{
    private:
    std::string channel_name;
    std::string topic;

    public:
    std::vector<Client> clients_ch;
    Channel(std::string name, std::vector<Client>::iterator it);
    ~Channel();

    std::string getName() const;
    std::string getTopic() const;
    void addClient(std::vector<Client>::iterator it);
    void setTopic(const std::string newTopic);
};

#endif
