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
 * if they exist. Will separate arguments based on spaces
 * or quoted strings. Spaces can be escaped with backslash.
 *
 * @param input The raw input from the user.
 */
void CommandParser::split_args(string input) {
  // clear args & command
  args.clear();
  command = "";

  std::stringstream ss(input);
  State state = WORD;
  string arg;
  while (ss.peek() != EOF) {
    char c = ss.get();

    switch (state) {
      case OUTSIDE:
        // move to next, dont add to word
        if (ss.peek() == '"') {
          state = QUOTE;
          args.push_back(arg);
          arg.clear();
        } else if (ss.peek() != ' ') {
          state = WORD;
          args.push_back(arg);
          arg.clear();
        }
        break;

      case WORD:
        // move to next & add unless a space
        if (c == '\\') {
          c = ss.get();
        }
        if (ss.peek() == ' ') {
          state = OUTSIDE;
        }
        arg.push_back(c);
        break;

      case QUOTE:
        // move to next & add unless end quote
        if (ss.peek() == '"') {
          state = OUTSIDE;
        }
        if (c != '"') {
          arg.push_back(c);
        }
        break;
    }
  }
  if (arg.size() != 0) {
    args.push_back(arg);
  }

  // remove the command from the args list
  if (!args.empty()) {
    command = args.front();
    args.erase(args.begin());
  }
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

