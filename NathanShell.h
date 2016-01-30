#ifndef __NATHAN_SHELL__
#define __NATHAN_SHELL__

#include <iostream>
#include <string>

#include "CommandParser.h"
#include "Status.h"

class NathanShell {
  public:
    NathanShell();

    // Shell functions
    Status check_builtins(std::string cmd);
    Status execute_command();
    void parse_input(std::string input);
    void print_args(Status status);
    std::string prompt_user();

    // Builtins
    void cd(std::string dir);
    void jobs();
    void print_uid();
    void print_user();
    void pwd();

  private:
    int cmd_counter;
    char cur_dir[256];
    struct utsname uname_data;

    CommandParser parser;
};

#endif

