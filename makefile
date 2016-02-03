all:
	g++ -g -Wall -Wextra -Werror -std=c++0x NathanShell.cpp CommandParser.cpp Status.h -o nsh
