CXXFLAGS = -std=c++11 -Wall -Werror -pedantic

all: rshell
rshell: 
	mkdir bin
	g++ $(CXXFLAGS) src/rshell.cpp -o bin/rshell
clean:
	rm -rf bin
