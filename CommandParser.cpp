/**
 * The job of the CommandParser class is to split
 * the input into its command and possible arguments.
 *
 * The shell then can use the resulting data as it wants.
 */

#include <iostream>
#include <regex>
#include <string>
#include <sstream>
#include <vector>

#include "CommandParser.h"

using std::cout;
using std::cin;
using std::endl;
using std::getline;
using std::string;
using std::vector;

/**
 * Construct the parser, defaulting command to an empty string.
 */
CommandParser::CommandParser() {
  command = "";
}

/**
 * Read the line of input from the user.
 *
 * @param prompt The user prompt to print.
 * @return The raw input from the user.
 */
string CommandParser::read_line(char* prompt) {
  cout << string(prompt);
  string text = "";
  getline(cin, text);
  return text;
}

/**
 * Split the input into the command and its arguments,
 * if they exist.
 *
 * @param input The raw input from the user.
 */
void CommandParser::split_args(string input) {
  using std::regex;
  using std::smatch;
  using std::sregex_iterator;

  regex re("(\"[^\"]*\")|([^\\s]+(\\\\ )?[^\\s]+)");
  sregex_iterator next(input.begin(), input.end(), re);
  sregex_iterator end;
  while (next != end) {
    smatch match = *next;
    args.push_back(match.str());
    next++;
  }

  // remove the command from the args list
  if (!args.empty()) {
    command = args.front();
    args.erase(args.begin());
  }


/*
  std::stringstream ss(input);
  string item = "";

  // clear args & command
  args.clear();
  command = "";

  while (getline(ss, item, ' ')) {
    args.push_back(item);
  }

  */
}

/**
 * Returns the number of arguments sent with the command
 *
 * @return The amount of arguments.
 */
int CommandParser::get_arg_count() {
  return args.size();
}

/**
 * Get the arguments to the command.
 *
 * @return The vector<string> of arguments, which may be empty.
 */
vector<string> CommandParser::get_args() {
  return args;
}

/**
 * Get the command string.
 *
 * @return The command.
 */
string CommandParser::get_command() {
  return command;
}

