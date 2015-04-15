CXXFLAGS = -Wall -Werror -ansi -pedantic

all: rshell
rshell: 
	mkdir bin
	g++ $(CXXFLAGS) src/rshell -o bin/rshell
clean:
	rm -r bin
