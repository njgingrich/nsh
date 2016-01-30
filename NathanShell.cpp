#include <iostream>
#include <pwd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <vector>

#include "CommandParser.h"
#include "NathanShell.h"
#include "Status.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;

/**
 * Constructs the shell, initializing information and setting
 * up the parser.
 */
NathanShell::NathanShell() {
  uname(&uname_data); // get system information
  cmd_counter = 1;
  getcwd(cur_dir, 256);
  parser = CommandParser();
}

/**
 * Check the built-in commands of the shell
 *
 * @return The status after checking all builtin commands
 */
Status NathanShell::check_builtins(string cmd) {
  int arg_count = parser.get_arg_count();
  vector<string> args = parser.get_args();

  if (cmd == "cd") {
    if (arg_count != 1) {
      return ARGS_ERR;
    }
    cd(args.front());

  } else if (cmd == "exit") {
    if (arg_count != 0) {
      return ARGS_ERR;
    }
    return EXIT;

  } else if (cmd == "jobs") {
    if (arg_count != 0) {
      return ARGS_ERR;
    }
    jobs();

  } else if (cmd == "pwd") {
    if (arg_count != 0) {
      return ARGS_ERR;
    }
    pwd();

  } else if (cmd == "terminate") {

  } else if (cmd == "uid") {
    if (arg_count != 0) {
      return ARGS_ERR;
    }
    print_uid();

  } else if (cmd == "user") {
    if (arg_count != 0) {
      return ARGS_ERR;
    }
    print_user();

  } else {
    // TODO: Implement process forking
    return CMD_NOT_FOUND;
  }

  return OKAY;
}

/**
 * Execute the command, first checking the built-in shell commands.
 *
 * @return OKAY if there were no errors, CMD_NOT_FOUND if the command
 * is unknown, or ARGS_ERR if the command has the wrong number of arguments.
 */
Status NathanShell::execute_command() {
  string cmd = parser.get_command();
  cmd_counter++;

  // check builtin commands
  if (cmd == "") {
    cmd_counter--;
    return OKAY;
  } else {
    return check_builtins(cmd);
  }

  return OKAY;
}

/**
 * Takes the input from the user and parses it.
 *
 * @param input The raw input string from the user.
 */
void NathanShell::parse_input(string input) {
  parser.split_args(input);
}

/**
 * Print the command and arguments from the user input.
 * Used in the error case when there was an error executing the command.
 *
 * @param status The status after executing the command.
 */
void NathanShell::print_args(Status status) {
  if (status == OKAY || status == EXIT) {
    return;

  } else {
    cout << "Command: " << parser.get_command() << endl;
    cout << "# of Arguments: " << parser.get_arg_count() << endl;
    vector<string> args = parser.get_args();

    for (unsigned int i = 0; i < args.size(); i++) {
      cout << "Argument #" << i+1 << ": " << args.at(i) << endl;
    }

    if (status == CMD_NOT_FOUND) {
      cout << "Command not found." << endl;
    } else if (status == ARGS_ERR) {
      cout << "Wrong number of arguments." << endl;
    }
  }
}

/**
 * Create the user prompt and give it to the parser to use.
 *
 * @return The string of user input.
 */
string NathanShell::prompt_user() {
  char prompt[64];
  sprintf(&prompt[0], "<%d %s %s> ", cmd_counter, uname_data.nodename, cur_dir);

  return parser.read_line(prompt);
}

/**
 * Change the directory to the one in the args parameter.
 * It will print an error if the directory doesn't exist.
 *
 * @param args The user arguments, 1 string
 */
void NathanShell::cd(string dir) {
  int status = chdir(dir.c_str());
  if (status == -1) {
    perror("An error occurred");
  } else {
    getcwd(cur_dir, 256); // update the prompt cwd
  }
}

/**
 * Print the list of processes currently executing in the background.
 * Each process will be outputted like <PID> <Command line>
 */
void NathanShell::jobs() {

}

/**
 * Print the uid of the user executing the shell.
 */
void NathanShell::print_uid() {
  cout << getuid() << endl;
}

/**
 * Print the username of the user executing the shell.
 */
void NathanShell::print_user() {
  struct passwd* pwd = getpwuid(getuid());
  cout << pwd->pw_name << endl;
}

/**
 * Print the current directory.
 */
void NathanShell::pwd() {
  cout << cur_dir << endl;
}

/**
 * The main program loop. Initializes the shell, then loops.
 */
int main() {
  NathanShell shell = NathanShell();
  Status status = OKAY;

  do {
    string input = shell.prompt_user();
    shell.parse_input(input);
    status = shell.execute_command();
    shell.print_args(status);

  } while (status != EXIT);
}

