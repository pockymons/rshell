CXXFLAGS = -std=c++11 -Wall -Werror -pedantic

all: rshell
rshell: 
	mkdir -p bin
	g++ $(CXXFLAGS) src/rshell.cpp -o bin/rshell
ls:
	mkdir -p bin
	g++ $(CXXFLAGS) src/ls.cpp -o bin/ls
clean:
	rm -rf bin
