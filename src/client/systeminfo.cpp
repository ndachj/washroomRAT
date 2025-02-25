#include <fstream>
#include <sstream>
#include <string>

#ifdef __linux__
#include <unistd.h>
#include <utmp.h>
#elif __WIN32
#include <VersionHelpers.h>
#include <sysinfoapi.h>
#include <windows.h>
#include <wtsapi32.h>
#endif // __linux__

class Sysinfo
{
public:
  std::string
  display ()
  {
    std::ostringstream outputStream;
    outputStream << "OS: " << getOSType () << "\nCPU: " << getCPUType ()
                 << "\nMEM: " << getMemUSage () << "\nUSER: " << getUserName ()
                 << "\nLogin users: " << geLoggedUsers ()
                 << "\nHostname: " << getComputerName ()
                 << "\nuptime: " << getUptime ();
    return outputStream.str ();
  }

private:
  // get the system uptime
  std::string
  getUptime ()
  {
#ifdef __linux__
    std::ifstream uptimeFile ("/proc/uptime");
    if (uptimeFile.is_open ())
      {
        double uptimeSeconds;
        uptimeFile >> uptimeSeconds;
        uptimeFile.close ();

        auto hours = static_cast<int> (uptimeSeconds / 3600);
        uptimeSeconds = static_cast<int> (uptimeSeconds) % 3600;
        auto mins = static_cast<int> (uptimeSeconds / 60);
        auto seconds = static_cast<int> (uptimeSeconds) % 3600;
        return std::to_string (hours) + "H:" + std::to_string (mins)
               + "M:" + std::to_string (seconds) + "S";
      }
    return "None"; // couldn't retrieve uptime
#elif __WIN32
    auto uptimeMillis = GetTickCount64 ();
    auto seconds = uptimeMillis / 1000;
    auto hours = seconds / 3600;
    seconds %= 3600;
    auto mins = seconds / 60;
    seconds %= 60;
    return std::to_string (hours) + ":" + std::to_string (mins) + ":"
           + std::to_string (seconds);
#else
    return "Unsupported Os";
#endif // __linux__
  }

  // get OS type and version
  std::string
  getOSType ()
  {
#ifdef __linux__
    std::ifstream osReleaseFile ("/etc/os-release");
    std::string line, osName, version;
    if (osReleaseFile.is_open ())
      {
        while (std::getline (osReleaseFile, line))
          {
            if (line.find ("PRETTY_NAME=") == 0)
              {
                osName = line.substr (13, line.length () - 14);
              }
            else if (line.find ("VERSION=") == 0)
              {
                version = line.substr (9, line.length () - 10);
              }
          }
        osReleaseFile.close ();
      }
    if (!osName.empty () and !version.empty ())
      {
        return osName + " " + version;
      }
    return "Unknown Linux";
#elif __WIN32
    if (IsWindows10OrGreater ())
      {
        return "Windows 10 or later";
      }
    else if (IsWindows8Point1OrGreater ())
      {
        return "Windows 8.1";
      }
    else if (IsWindows8OrGreater ())
      {
        return "Windows 8";
      }
    else if (IsWindows7SP1OrGreater ())
      {
        return "Windows 7 SP1";
      }
    else if (IsWindows7OrGreater ())
      {
        return "Windows 7";
      }
    return "Unknown Windows";
#else
    return "Unsupported Os";
#endif // __linux__
  }

  std::string
  getMemUSage ()
  {
#ifdef __linux__
    std::ifstream memInfoFile ("/proc/meminfo");
    std::string line;
    long totalMem = 0, freeMem = 0;
    if (memInfoFile.is_open ())
      {
        while (std::getline (memInfoFile, line))
          {
            if (line.find ("MemTotal:") == 0)
              {
                totalMem = std::stol (line.substr (10));
              }
            else if (line.find ("MemAvailable:") == 0)
              {
                freeMem = std::stol (line.substr (13));
                break;
              }
          }
        memInfoFile.close ();
      }
    return std::to_string (freeMem / 1024) + " MB avail/"
           + std::to_string (totalMem / 1024) + " MB total";
#elif __WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof (MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx (&memInfo))
      {
        return std::to_string (memInfo.ullTotalPhys / (1024 * 1024))
               + " MB total, "
               + std::to_string (memInfo.ullAvailPhys / (1024 * 1024))
               + " MB available.";
      }
    return "Unavailable meminfo";
#else
    return "Unsupported Os";
#endif // __linux__
  }

  std::string
  getCPUType ()
  {
#ifdef __linux__
    std::ifstream cpuInfoFile ("/proc/cpuinfo");
    std::string line, cpuModel;
    if (cpuInfoFile.is_open ())
      {
        while (std::getline (cpuInfoFile, line))
          {
            if (line.find ("model name") == 0)
              {
                cpuModel = line.substr (line.find (':') + 2);
                break;
              }
          }
        cpuInfoFile.close ();
      }
    return cpuModel.empty () ? "Unknown CPU" : cpuModel;
#elif __WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo (&sysInfo);
    return "Arch " + std::to_string (sysInfo.wProcessorArchitecture) + ", "
           + std::to_string (sysInfo.dwNumberOfProcessors) + " cores.";
#else
    return "Unsupported Os";
#endif // __linux__
  }

  std::string
  getComputerName ()
  {
#ifdef __linux__
    char buffer[256];
    if (gethostname (buffer, sizeof (buffer)) == 0)
      {
        return buffer;
      }
    return "Unknown hostname";
#elif __WIN32
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof (buffer);

    if (GetComputerNameExA (ComputerNamePhysicalDnsHostname, buffer, &size))
      {
        return buffer;
      }
    return "Unknown ComputerName";
#else
    return "Unsupported Os";
#endif // __linux__
  }

  std::string
  getUserName ()
  {
#ifdef __linux__
    const char *user = getenv ("USER");
    if (user)
      {
        return user;
      }
    return "Unknown USER";
#elif __WIN32
    char buffer[256];
    DWORD size = sizeof (buffer);

    if (GetUserNameA (buffer, &size))
      {
        return buffer;
      }
    return "Unknown USER";
#else
    return "Unsupported Os";
#endif // __linux__
  }

  std::string
  geLoggedUsers ()
  {
#ifdef __linux__
    struct utmp *entry;

    setutent (); // Open utmp file
    std::ostringstream userStream;
    while ((entry = getutent ()))
      {

        if (entry->ut_type == USER_PROCESS)
          {

            userStream << entry->ut_user << " (" << entry->ut_line << ")\n";
          }
      }
    endutent (); // Close utmp file
    return userStream.str ();
#elif __WIN32
    PWTS_SESSION_INFO sessions = nullptr;
    DWORD sessionCount = 0;

    if (WTSEnumerateSessions (WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessions,
                              &sessionCount))
      {
        std::ostringstream userStream;
        for (DWORD i = 0; i < sessionCount; ++i)
          {
            PWTS_SESSION_INFO session = &sessions[i];

            if (session->State == WTSActive)
              {
                LPTSTR username = nullptr;
                DWORD usernameLen = 0;

                if (WTSQuerySessionInformation (
                        WTS_CURRENT_SERVER_HANDLE, session->SessionId,
                        WTSUserName, &username, &usernameLen))
                  {
                    userStream << " - " << username << "\n";
                    WTSFreeMemory (username);
                  }
              }
          }
        WTSFreeMemory (sessions);
        return userStream.str ();
      }
    return "failed";
#else
    return "Unsupported Os";
#endif // __linux__
  }
};
