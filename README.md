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
##Edge Behavior
##Limitations/Bugs
