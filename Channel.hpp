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

    //client ekle
    //client çıkar
    //konu değiştir
    //std::vector<Client>::iterator find_client_ch(int fd);
    std::string getName() const;
    std::string getTopic() const;
    void addClient(std::vector<Client>::iterator it);
    //std::string getTopic() const { return topic; }
    //void setTopic(const std::string& newTopic) { topic = newTopic; }
    //void addOperator(Client* client);
    //void removeOperator(Client* client);
    //bool isOperator(Client* client) const;
};

#endif
