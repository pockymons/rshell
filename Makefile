CXXFLAGS = -std=c++11 -Wall -Werror -pedantic

all: rshell ls cp
rshell: 
	mkdir -p bin
	g++ $(CXXFLAGS) src/rshell.cpp -o bin/rshell
ls:
	mkdir -p bin
	g++ $(CXXFLAGS) src/ls.cpp -o bin/ls
cp:  
	mkdir -p bin
	g++ $(CXXFLAGS) src/cp.cpp -o bin/cp
clean:
	rm -rf bin
