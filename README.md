#Rshell
This project is a shell that interprets and executes lines of command, including the commands, their arguments, and connectors. 
It simulates the functionality of the bash shell. This project is made for an assignment for the CS100 course, a course on open
source software construction, at University of California, Riverside. 
##Dependencies
rshell requires the Boost Libraries for C++ to be installed. 

The UCR lab machines, with cs100 settings, already have the Boost Libraries installed.

To install the libraries on Linux Mint, run `sudo apt-get install libboost-all-dev`

Otherwise, download Boost from [www.boost.org/users/download/](www.boost.org/users/download/)
##Install/Run 
To install, type the following into the terminal:
```
$ git clone https://github.com/pockymons/rshell.git
$ cd rshell
$ git checkout hw0
$ make
```

To run, type the following into the terminal, while you are in the rshell directory:
```
$ bin/rshell
```
##Edge Cases
*I chose to make comments only work if there is a space before the `#` symbol, just like bash does it.
*Connectors are ordered from left to right. The command before a connector is checked for each connector. 
##Limitations/Bugs
*Prompt only works for up to 64 characters.
*If there is whitespace only after a connector, it will just act as if it is then end, instead of outputting an error message.
*Pressing the up arrow key does not go to the previous command
*Press tab does not auto-complete a command/directory
*The `||` and `&&` connectors only work in even groupings. Otherwise, they are interpreted as commands or part of a command.
*Comments only work if there is a command preceding them.
