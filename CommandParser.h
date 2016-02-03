#ifndef __COMMAND_PARSER__
#define __COMMAND_PARSER__

#include <string>
#include <vector>

class CommandParser {
  public:
    CommandParser();
    enum State {
      OUTSIDE,
      WORD,
      QUOTE,
      ESCAPE
    };
    std::string read_line(char* prompt);
    void split_args(std::string input);

    // getters
    int get_arg_count();
    std::vector<std::string> get_args();
    std::string get_command();

  private:
    std::vector<std::string> args;
    std::string command;
};

#endif

