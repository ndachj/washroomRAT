#ifndef SRC_SERVER_LISTENER_H
#define SRC_SERVER_LISTENER_H

#include <netinet/in.h>
#include <sys/socket.h>

#include <atomic>

namespace listener
{
class Listener
{
private:
  int m_sock = -1;
  struct sockaddr_in m_addr;
  const char *m_ip;
  unsigned short m_port;
  std::atomic<bool> m_isRunning = false;

private:
  int setup ();
  void acceptClient ();

public:
  Listener (const char *ip, unsigned short port) : m_ip (ip), m_port (port) {}
  int start ();
  void stop ();
};

} // namespace listener

#endif // !SRC_SERVER_LISTENER_H
