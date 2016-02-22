#include <algorithm>
#include <ctime>
#include <dirent.h>
#include <grp.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <pwd.h>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "CommandParser.h"
#include "NathanShell.h"
#include "Status.h"

using std::cout;
using std::endl;
using std::list;
using std::map;
using std::pair;
using std::setw;
using std::string;
using std::vector;

/**
 * Constructs the shell, initializing information and setting
 * up the parser.
 */
NathanShell::NathanShell() {
  bg_processes = 0;
  uname(&uname_data); // get system information
  cmd_counter = 1;
  getcwd(cur_dir, 256);
  parser = CommandParser();
}

/**
 * Check the status of background processes currently running.
 * Any processes that have ended will be printed with the reason
 * for their exit.
 */
void NathanShell::check_background() {
  int pid;
  while ((pid = waitpid((pid_t)(-1), 0, WNOHANG)) > 0) {
    if (job_list.count(pid) > 0) {
      cout << pid << " exited." << endl;
    } else {
      cout << pid << " terminated." << endl;
    }
  }
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

  } else if (cmd == "dir") {
    if (arg_count > 3) {
      return ARGS_ERR;
    }
    dir(args);

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
    if (arg_count != 1) {
      return ARGS_ERR;
    }
    terminate(atoi(args.front().c_str()));

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
    int status = run_external(cmd, args);
    if (status == -1) {
      return CMD_NOT_FOUND;
    }
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
 * Run an external program, specified by 'cmd'.
 *
 * @param cmd The name of the program to run.
 * @param args The arguments to pass to the external program.
 * @return The status from execvp.
 */
int NathanShell::run_external(string cmd, vector<string> args) {
  int pid = fork();
  int status = 0;
  bool background = false;
  if (args.empty()) {
    background = false;
  } else if (args.back() == "&") {
    background = true;
    args.pop_back(); // remove '&' from args
  }

  if (pid == 0) { // new process
    vector<char*> argv = str_to_charptr(cmd, args);
    status = execvp(cmd.c_str(), &argv[0]);
    if (status == -1) {
      perror("An error occurred");
    }

  } else {
    if (background) {
      cout << pid << " " << cmd << endl;
      job_list[pid] = pair<int, string>(0, cmd); // add to jobs list
      return status;
    } else {
      wait(&status);
    }
  }
  return status;
}

/**
 * Convert a vector<string> to vector<char*> to use in execvp().
 * With convention, the first element in the vector<char*> is the
 * name of the command used.
 *
 * @param args The arguments from the user input to convert.
 * @param cmd The command inputted from the user.
 * @return A vector<char*> of the arguments.
 */
vector<char*> NathanShell::str_to_charptr(string cmd, vector<string> args) {
    vector<char*> argv;
    argv.push_back(&cmd[0]);
    for (vector<string>::iterator it = args.begin(); it != args.end(); ++it) {
      argv.push_back(&(*it)[0]);
    }
    argv.push_back((char *) NULL); // needs to be null-terminated
    return argv;
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
 * List the files in the current working directory.
 *
 * @param args The arguments from the user's input.
 */
void NathanShell::dir(vector<string> args) {
  // Check for the optional flags
  bool a_flag = false; // access time vs mod time
  bool b_flag = false; // block size vs actual size
  string dir_name = ".";
  auto a_exists = std::find(args.begin(), args.end(), "-a");
  auto b_exists = std::find(args.begin(), args.end(), "-b");
  if (a_exists != args.end()) {
    a_flag = true;
    //args.erase(a_exists);
  }
  if (b_exists != args.end()) {
    b_flag = true;
    //args.erase(b_exists);
  }
  if (!args.empty()) {
    dir_name = args.back();
  }

  chdir(dir_name.c_str());
  list<string> entries = get_entries(dir_name);

  struct stat dir_stat;
  if (stat(dir_name.c_str(), &dir_stat) < 0) {
    perror("An error occurred");
    chdir(cur_dir);
    return;
  }

  for (auto it = entries.begin(); it != entries.end(); ++it) {
    struct stat info;
    if (stat((*it).c_str(), &info) < 0) {
      perror("An error occurred");
      chdir(cur_dir);
      return;
    }

    string perms = get_permissions(info.st_mode);
    string owner = get_owner(info.st_uid);
    string group = get_group(info.st_gid);
    string time  = a_flag ? get_time(info.st_atime) : get_time(info.st_mtime);
    int filesize = b_flag ? info.st_blocks : info.st_size;
    /*
     * output:
     * ?rwxrwxrwx owner group size mod_date filename
     */
    cout << perms << " " << setw(8) << owner << "\t";
    cout << setw(8) << group << "\t";
    cout << filesize << "\t" << time << "\t" << *it << endl;
  }

  chdir(cur_dir);
}

/**
 * Print the list of processes currently executing in the background.
 * Each process will be outputted like <PID> <Command line>
 */
void NathanShell::jobs() {
  for (map<int, pair<int, string>>::iterator it = job_list.begin(); it != job_list.end(); ++it) {
    cout << it->first << " " << it->second.second << endl;
  }
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
 * Terminate a currently-running background process.
 *
 * @param pid The id of the process to terminate.
 */
void NathanShell::terminate(int pid) {
  int status = kill(pid, SIGKILL);
  if (status < 0) {
    perror("An error occurred");
  } else {
    // set the int value to 1
    map<int, pair<int, string>>::iterator job = job_list.find(pid);
    if (job != job_list.end()) {
      job_list[pid] = pair<int, string>(1, (job->second).second);
    }
  }
}

/**
 * Get the entries in a directory for the given directory,
 * or the file if the parameter is a file.
 *
 * @param dir_name The name of the directory, which could be a file.
 * @return The list of entries, as std::strings.
 */
list<string> NathanShell::get_entries(string dir_name) {
  DIR *dir;
  list<string> entries;
  struct dirent *dp;
  if ((dir = opendir(dir_name.c_str())) == NULL) {
    // not a directory, but if it's a file return it as the entry
    entries.push_back(dir_name);
    return entries;
  }
  while ((dp = readdir(dir)) != NULL) {
    entries.push_back(dp->d_name);
  }
  entries.sort([](const string &a, const string &b) {
    // ignore periods (hidden files) by removing them
    string lhs(a);
    string rhs(b);
    lhs.erase(std::remove(lhs.begin(), lhs.end(), '.'), lhs.end());
    rhs.erase(std::remove(rhs.begin(), rhs.end(), '.'), rhs.end());
    unsigned int i = 0;
    while (i < lhs.length() && i < rhs.length()) {
      if (tolower(lhs.at(i)) < tolower(rhs.at(i))) {
        return true;
      } else if (tolower(lhs.at(i)) > tolower(rhs.at(i))) {
        return false;
      } else {
        i++;
      }
    }
    return a.length() < b.length();
  });
  closedir(dir);
  return entries;
}

/**
 * Get the group name from the group id for a file.
 *
 * @param gid The group id to search.
 * @return The std::string of the group name.
 */
string NathanShell::get_group(gid_t gid) {
  struct group* grp = getgrgid(gid);
  return string(grp->gr_name);
}

/**
 * Get the owner's username for a file.
 *
 * @param uid The uid of the user.
 * @return The std::string of the username.
 */
string NathanShell::get_owner(uid_t uid) {
  struct passwd* pwd = getpwuid(uid);
  return string(pwd->pw_name);
}

/**
 * Construct the unix permissions from a mode_t from a file.
 *
 * @param mode The st_mode from a stat() syscall.
 * @return The string of permissions, represented by ?rwxrwxrwx.
 */
string NathanShell::get_permissions(mode_t mode) {
  std::stringstream ss;
  // Get filetype
  if (S_ISBLK(mode)) {
    ss << "b";
  } else if (S_ISCHR(mode)) {
    ss << "c";
  } else if (S_ISDIR(mode)) {
    ss << "d";
  } else if (S_ISFIFO(mode)) {
    ss << "f";
  } else if (S_ISREG(mode)) {
    ss << "-";
  } else if (S_ISLNK(mode)) {
    ss << "l";
  } else {
    ss << "-";
  }

  // user-group-other permissions
  ss << ((mode & S_IRUSR) ? "r" : "-");
  ss << ((mode & S_IWUSR) ? "w" : "-");
  ss << ((mode & S_IXUSR) ? "x" : "-");
  ss << ((mode & S_IRGRP) ? "r" : "-");
  ss << ((mode & S_IWGRP) ? "w" : "-");
  ss << ((mode & S_IXGRP) ? "x" : "-");
  ss << ((mode & S_IROTH) ? "r" : "-");
  ss << ((mode & S_IWOTH) ? "w" : "-");
  ss << ((mode & S_IXOTH) ? "x" : "-");
  return ss.str();
}

/**
 * Construct the formatted string for the given time to use
 * in the dir command.
 *
 * @param time The time in milliseconds (time_t).
 * @return The formatted string of output: Mon DD HH:MM,
 * or Mon DD YYYY if the file is older than 6 months.
 */
string NathanShell::get_time(time_t time) {
  std::tm* t = std::localtime(&time);
  std::time_t now = std::time(NULL);
  double seconds = std::difftime(now, time);
  char new_time[100];

  if (seconds < 15768000) { // 60s * 60m * 24h * 365d/2 = 6mo
    std::strftime(new_time, sizeof(new_time), "%b %d\t%H:%M", t);
  } else {
    std::strftime(new_time, sizeof(new_time), "%b %d\t%Y", t);
  }
  return string(new_time);
}

/**
 * Handle the SIGCHILD calls.
 *
 * @param sig The signal.
 */
void handle_sigchld(int sig) {
  if (sig) {} // avoid 'handle 'unused' warning as error'
}

/**
 * Handle the SIGINT calls - ignore ctrl-C ending the program.
 *
 * @param sig The signal.
 */
void handle_sigint(int sig) {
  if (sig) {} // ignore ctrl+C
}

/**
 * The main program loop. Initializes the shell, then loops.
 */
int main() {
  NathanShell shell = NathanShell();
  Status status = OKAY;

  // handle bg processes exiting
  struct sigaction sa;
  sa.sa_handler = &handle_sigchld;
  if (sigaction(SIGCHLD, &sa, 0) == -1) {
      perror(0);
  }
  signal(SIGINT, &handle_sigint);

  do {
    shell.check_background();
    string input = shell.prompt_user();
    shell.parse_input(input);
    status = shell.execute_command();
    shell.print_args(status);

  } while (status != EXIT);
}

