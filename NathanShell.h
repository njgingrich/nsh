#ifndef __NATHAN_SHELL__
#define __NATHAN_SHELL__

#include <iostream>
#include <map>
#include <string>

#include "CommandParser.h"
#include "Status.h"

#define MAX_PROCESSES 4

class NathanShell {
  public:
    NathanShell();

    // Shell functions
    void check_background();
    Status check_builtins(std::string cmd);
    Status execute_command();
    void parse_input(std::string input);
    void print_args(Status status);
    std::string prompt_user();
    int run_external(std::string cmd, std::vector<std::string> args);
    std::vector<char*> str_to_charptr(std::string cmd, std::vector<std::string> args);

    // Builtins
    void cd(std::string dir);
    void jobs();
    void print_uid();
    void print_user();
    void pwd();
    void terminate(int pid);

  private:
    int bg_processes;
    int cmd_counter;
    char cur_dir[256];
    struct utsname uname_data;
    std::map<int, std::string> job_list;

    CommandParser parser;
};

#endif

