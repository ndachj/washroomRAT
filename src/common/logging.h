#ifndef SRC_COMMON_LOGGING_H
#define SRC_COMMON_LOGGING_H

#include <chrono>
#include <fstream>
#include <iomanip> // std::put_time
#include <ios>     // std::ios
#include <iostream>
#include <iterator>
#include <map>
#include <mutex>
#include <regex>
#include <sstream> // std::ostringstream
#include <stdexcept>
#include <string>
#include <thread>

namespace ANSIColors
{
constexpr const char *RESET = "\033[0m";
constexpr const char *CYAN = "\033[36m";
constexpr const char *BLUE = "\033[34m";
constexpr const char *YELLOW = "\033[33m";
constexpr const char *RED = "\033[31m";
}

enum class LogLevel
{
  DEBUG = 0,
  INFO = 1,
  WARN = 2,
  ERROR = 3,
  FATAL = 4
};

// stores Logger configurations
struct LoggerConfig
{
  LogLevel logLevel = LogLevel::INFO;
  std::string logFormat = "[{level}] {message}";
  std::string dateFormat = "%H:%M:%S";
};

// format logs as in logFormat
class Formatter
{
private:
  std::string &m_logFormat;
  std::string &m_dateFormat;

private:
  // get current datetime
  std::string
  getCurrentTime ()
  {
    auto now = std::chrono::system_clock::now ();
    auto realTime = std::chrono::system_clock::to_time_t (now);
    std::ostringstream oss;
    oss << std::put_time (std::localtime (&realTime), m_dateFormat.c_str ());
    return oss.str ();
  }

  // get Thread Id
  std::string
  getThreadId ()
  {
    auto threadId = std::this_thread::get_id ();
    std::ostringstream oss;
    oss << threadId;
    return oss.str ();
  }

public:
  Formatter (LoggerConfig &config)
      : m_logFormat (config.logFormat), m_dateFormat (config.dateFormat)
  {
  }

  // 1. parseLog message
  template <typename... Args>
  std::string
  parseLog (const std::string &levelName, const std::string &msg)
  {
    // map of placeholder:value
    std::map<std::string, std::string> placeholders
        = { { "{timestap}", getCurrentTime () },
            { "{level}", levelName },
            { "{thread}", getThreadId () },
            { "{file}", "{}" },
            { "{line}", "{}" },
            { "{message}", msg } };

    // Replace placeholders in the format string
    std::string result = m_logFormat;
    for (const auto &[key, value] : placeholders)
      {
        size_t pos;
        while ((pos = result.find (key)) != std::string::npos)
          {
            result.replace (pos, key.length (), value);
          }
      }
    return result;
  }

  // 2. format message
  template <typename... Args>
  std::string
  formatMessage (const std::string &msg, Args... args)
  {
    std::ostringstream oss;
    formatMessageHelper (oss, msg, args...);
    return oss.str ();
  }

  // 3a. format helper with args
  template <typename T, typename... Args>
  void
  formatMessageHelper (std::ostringstream &oss, const std::string &message,
                       T value, Args... args)
  {
    size_t pos = message.find ("{}");
    if (pos != std::string::npos)
      {
        oss << message.substr (0, pos) << value;
        formatMessageHelper (oss, message.substr (pos + 2), args...);
      }
    else
      {
        oss << message; // Append remaining part of the message
      }
  }

  // 3b. format helper with no args
  void
  formatMessageHelper (std::ostringstream &oss, const std::string &message)
  {
    oss << message;
  }
};

// Logger
class Logger
{
private:
  std::ofstream m_logFile;
  std::mutex m_logMutex;
  LoggerConfig config;
  Formatter formatter;

  friend class Logging;

private:
  Logger (const std::string &filename)
      : m_logFile (filename, std::ios::app), formatter (config)
  {
  }

  Logger (const Logger &) = delete;
  Logger &operator= (const Logger &) = delete;

  ~Logger ()
  {
    if (m_logFile.is_open ())
      {
        m_logFile.close ();
      }
  }

  // log
  template <typename... Args>
  void
  log (LogLevel level, const std::string &msg, Args... args)
  {
    if (level >= config.logLevel)
      {
        std::lock_guard<std::mutex> lock (m_logMutex);

        std::string logLevelName = getLogLevelName (level);
        auto parsedMessage = formatter.parseLog (logLevelName, msg);
        auto formatedMessage
            = formatter.formatMessage (parsedMessage, args...);
        if (m_logFile.is_open ())
          {
            m_logFile << formatedMessage << std::endl;
          }
        else
          {
            std::cout << formatedMessage << std::endl;
          }
      }
  }

  // FIX: a failed attempt to split args
  // template <typename... Args>
  // auto
  // Logger::SplitArgs (Args &&...args)
  // {
  //   bool hasFilePlaceholder = m_logFormat.find ("{file}") !=
  //   std::string::npos; bool hasLinePlaceholder = m_logFormat.find ("{file}")
  //   != std::string::npos; auto tupleArgs = std::make_tuple
  //   (std::forward<Args> (args)...);

  //   if (hasFilePlaceholder && hasLinePlaceholder)
  //     {
  //       std::string file;
  //       int line;

  //       auto remainingArgs = std::apply (
  //           [] (auto &&first, auto &&second, auto &&...rest) {
  //             file = first;
  //             line = second;
  //             return std::make_tuple (std::forward<decltype (rest)>
  //             (rest)...);
  //           },
  //           tupleArgs);

  //       return std::make_tuple (file, line, remainingArgs);
  //     }
  //   return std::make_tuple ("", 0,
  //                           std::make_tuple (std::forward<Args> (args)...));
  // }

  // get logFormat
  // @return logFormat
  std::string
  getLogFormat () const
  {
    return config.logFormat;
  }

  // get the current logLevel name
  // @param
  // @return human-readable logLevel name
  std::string
  getLogLevel () const
  {
    return getLogLevelName (config.logLevel);
  }

  // translate logLevel to string
  // @param level
  // @return human-readable logLevel name
  std::string
  getLogLevelName (LogLevel level) const
  {
    switch (level)
      {
      case LogLevel::DEBUG:
        return std::string (ANSIColors::CYAN) + "DEBUG"
               + std::string (ANSIColors::RESET);
      case LogLevel::INFO:
        return std::string (ANSIColors::BLUE) + "INFO"
               + std::string (ANSIColors::RESET);
      case LogLevel::WARN:
        return std::string (ANSIColors::YELLOW) + "WARNING"
               + std::string (ANSIColors::RESET);
      case LogLevel::ERROR:
        return std::string (ANSIColors::RED) + "ERROR"
               + std::string (ANSIColors::RESET);
      case LogLevel::FATAL:
        return std::string (ANSIColors::RED) + "FATAL"
               + std::string (ANSIColors::RESET);
      default:
        return "UNKNOWN";
      }
  }

  // configure the Logger
  void
  basicConfig (const LoggerConfig &configurations)
  {
    isValidConfig (configurations);
    std::lock_guard<std::mutex> lock (m_logMutex);
    config = configurations;
  }

  void
  isValidConfig (const LoggerConfig &cfg)
  {
    // checks logFormat
    if (cfg.logFormat.empty ())
      {
        throw std::invalid_argument ("logFormat can't be empty");
      }
    if (cfg.logFormat.find ("{message}") == std::string::npos)
      {
        log (LogLevel::DEBUG, "no message placeholder in logFormat");
      }

    // checks for invalid placeholders in logFormat
    std::regex validPlaceholders (
        "\\{(timestap|level|thread|file|line|message)\\}");
    auto it = std::sregex_iterator (cfg.logFormat.begin (),
                                    cfg.logFormat.end (), validPlaceholders);
    if (std::distance (it, std::sregex_iterator ()) == 0)
      {
        log (LogLevel::DEBUG, "invalid logFormat");
      }
  }

public:
  static Logger &
  getInstance (const std::string &filename = "")
  {
    static Logger instance (filename);
    return instance;
  }
};

class Logging
{
public:
  template <typename... Args>
  static void
  debug (const std::string &msg, Args... args)
  {

    Logger::getInstance ().log (LogLevel::DEBUG, msg, args...);
  }

  template <typename... Args>
  static void
  info (const std::string &msg, Args... args)
  {

    Logger::getInstance ().log (LogLevel::INFO, msg, args...);
  }

  template <typename... Args>
  static void
  warning (const std::string &msg, Args... args)
  {

    Logger::getInstance ().log (LogLevel::WARN, msg, args...);
  }

  template <typename... Args>
  static void
  error (const std::string &msg, Args... args)
  {

    Logger::getInstance ().log (LogLevel::ERROR, msg, args...);
  }

  template <typename... Args>
  static void
  fatal (const std::string &msg, Args... args)
  {

    Logger::getInstance ().log (LogLevel::FATAL, msg, args...);
  }

  static void
  basicConfig (const LoggerConfig &configurations)
  {
    Logger::getInstance ().basicConfig (configurations);
  }

  static std::string
  getLogFormat ()
  {
    return Logger::getInstance ().getLogFormat ();
  }

  std::string
  getLogLevel ()
  {
    return Logger::getInstance ().getLogLevel ();
  }
};

#endif // SRC_COMMON_LOGGING_H
