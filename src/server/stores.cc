#include <netinet/in.h>

#include <exception>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "logging.h"
#include "stores.h"

namespace stores
{
ClientManager &
ClientManager::getInstance ()
{
  static ClientManager clientManager;
  return clientManager;
}

/// add client to the clients list
/// @param sock socket file descriptor
/// @param addr sockaddr_in return by accept()
void
ClientManager::addClient (int sock, struct sockaddr_in &addr)
{
  Client client;
  client.sock = sock;
  client.addr = addr;
  client.id = m_clientId;
  m_clientsMap.insert ({ m_clientId, client });
  ++m_clientId;
}

/// remove client with the socket fd
/// @param id client unique id
void
ClientManager::removeClient (int id)
{
  try
    {
      m_clientsMap.erase (id);
    }
  catch (std::exception &e)
    {
      Logging::error ("failed to remove client {} {}", id, e.what ());
    }
}

/// give a client a fancy name
/// @param id client unique id
/// @param name new name
void
ClientManager::renameClient (int id, const std::string &name)
{
  try
    {
      auto client = m_clientsMap.find (id);
      client->second.alias = name;
    }
  catch (const std::out_of_range &)
    {
      Logging::debug ("client {} doesn't exists", id);
    }
}

/// get client details
/// @param id client unique id
ClientManager::Client &
ClientManager::getClient (int id)
{
  return m_clientsMap.find (id)->second;
}

/// get all clients details
std::vector<ClientManager::Client>
ClientManager::getClients () const
{
  std::vector<Client> clients;
  clients.reserve (m_clientsMap.size ());
  for (const auto &[sock, client] : m_clientsMap)
    {
      clients.push_back (client);
    }
  return clients;
}

} // namespace stores
