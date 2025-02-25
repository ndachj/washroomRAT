#ifndef SRC_SERVER_STORES_H
#define SRC_SERVER_STORES_H

#include <netinet/in.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace stores
{
// manage clients
class ClientManager
{
private:
  struct Client
  {
    int sock;
    struct sockaddr_in addr;
    std::string alias = "none";
    int id;
  };
  int m_clientId = 0;
  std::unordered_map<int, Client> m_clientsMap;

private:
  ClientManager () {}
  ClientManager (const ClientManager &) = delete;
  ClientManager &operator= (const ClientManager &) = delete;
  ~ClientManager () {}

public:
  static ClientManager &getInstance ();

  // add client to the clients list
  void addClient (int sock, struct sockaddr_in &addr);

  // remove client with the id
  void removeClient (int id);

  // give a client a fancy name
  // @param name new name
  void renameClient (int id, const std::string &name);

  // get a client details
  Client &getClient (int id);

  // get all clients details
  std::vector<Client> getClients () const;
};
} // namespace stores

#endif // SRC_SERVER_STORES_H
