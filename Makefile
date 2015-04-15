CXXFLAGS = -Wall -Werror -ansi -pedantic

all: rshell
rshell: 
	mkdir bin
	g++ $(CXXFLAGS) src/rshell.cpp -o bin/rshell
clean:
	rm -rf bin
