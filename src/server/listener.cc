#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <thread>

#include "listener.h"
#include "logging.h"
#include "stores.h"

namespace listener
{
int
Listener::setup ()
{
  Logging::debug ("starting the Listener ...");
  m_sock = socket (AF_INET, SOCK_STREAM, 0);
  if (m_sock == -1)
    {
      Logging::error ("socket(): {}", std::strerror (errno));
      return -1;
    }

  m_addr.sin_family = AF_INET;
  m_addr.sin_port = htons (m_port);
  if (inet_pton (AF_INET, m_ip, &m_addr.sin_addr) <= 0)
    {
      Logging::error ("inet_pton(): {}", std::strerror (errno));
      close (m_sock);
      return -1;
    };

  // bind
  if (bind (m_sock, (const struct sockaddr *)&m_addr, sizeof (m_addr)) == -1)
    {
      Logging::error ("bind(): {}", std::strerror (errno));
      close (m_sock);
      return -1;
    }

  // llisten
  if (listen (m_sock, 5) == -1)
    {
      Logging::error ("listen(): {}", std::strerror (errno));
      close (m_sock);
      return -1;
    }
  Logging::debug ("listening for connections on {}:{}", m_ip, m_port);
  return 0;
}

void
Listener::acceptClient ()
{
  int clientSock;
  struct sockaddr_in clientAddr;
  socklen_t clientAddrLength = sizeof (clientAddr);
  while (m_isRunning)
    {
      clientSock
          = accept (m_sock, (struct sockaddr *)&clientAddr, &clientAddrLength);
      if (clientSock == -1) // BUG: add & continue
        {
          Logging::error ("accept(): {}", std::strerror (errno));
          break;
        }

      stores::ClientManager::getInstance ().addClient (clientSock, clientAddr);
      Logging::info ("connection received from {}:{}",
                     inet_ntoa (clientAddr.sin_addr),
                     ntohs (clientAddr.sin_port));
    }
}

int
Listener::start ()
{
  if (setup () == 0)
    {
      m_isRunning = true;
      std::thread (&Listener::acceptClient, this)
          .detach (); // accept clients in new thread
      return 0;
    }
  return -1;
}

void
Listener::stop ()
{
  m_isRunning = false;
  if (m_sock != -1)
    {
      close (m_sock);
    }
  Logging::debug ("Listener stopped");
}

} // namespace listener
