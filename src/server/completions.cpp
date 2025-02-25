#include <readline/history.h>
#include <readline/readline.h>

#include <cstddef>
#include <string>
#include <vector>

namespace completions
{
class CommandHandler
{
private:
  std::vector<std::string> m_availableCommands
      = { "list", "select",  "alias", "shell", "dl",
          "up",   "sysinfo", "help",  "exit" };

public:
  static CommandHandler *
  getInstance ()
  {
    static CommandHandler instance;
    return &instance;
  }

  // generate all matching commands
  static char *
  commandGen (const char *text, int state)
  {
    static size_t index;
    static std::vector<std::string> *cmdList;
    if (!state)
      {
        index = 0;
        cmdList = &getInstance ()->m_availableCommands;
      }

    while (index < cmdList->size ())
      {
        const std::string &cmd = (*cmdList)[index++];
        if (cmd.find (text, 0) == 0)
          {
            return strdup (cmd.c_str ());
          }
      }
    return nullptr;
  }

  static char **
  completion (const char *text, int start, int end)
  {
    rl_on_new_line ();
    rl_refresh_line (0, 0);

    return rl_completion_matches (text, commandGen);
  }
};

} // namespace completions
