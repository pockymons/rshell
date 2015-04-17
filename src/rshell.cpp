#include <iostream>
#include <string>
#include <boost/tokenizer.hpp> //Boost tokenizer
#include <unistd.h> // Fork()
#include <sys/types.h> // Wait()
#include <sys/wait.h> // Wait()
#include <vector>
#include <stdio.h> //Perror()
#include <errno.h> // Perror()
#include <algorithm>

using namespace std;
using namespace boost;

int main()
{
	while(true) //Shell runs until the exit command
	{
		cout << "$"; // Prints command prompt
		string commandLine;
		getline(cin, commandLine);

		// Accounts for comments by removing parts that are comments
		// TODO: Account for escape character + comment (\#)
		if(commandLine.find(" #") != string::npos)
		{
			commandLine = commandLine.substr(commandLine.find(" #"));
		}

		// Finds locations of connectors; a && b, && has a location of 3
		vector<unsigned int> connectorLocs;
		unsigned int marker = 0; // Marks location to start find() from
		unsigned int loc = commandLine.find("&&", marker);
		while(loc != string::npos) 
		{
			connectorLocs.push_back(loc);
			marker = loc + 2; // Starts searching after "&&"
			loc = commandLine.find("&&", marker);
		}
		marker = 0;
		loc = commandLine.find("||", marker);
		while(loc != string::npos) 
		{
			connectorLocs.push_back(loc); 
			marker = loc + 2; // Starts searching after "||"
			loc = commandLine.find("||", marker);
		}
		marker = 0;
		loc = commandLine.find(";", marker);
		while(loc != string::npos)
		{
			connectorLocs.push_back(loc);
			marker = loc + 1; // Starts searching after ";"
			loc = commandLine.find(";", marker);
		}
		connectorLocs.push_back(0); // Will be sorted and put in beginning
		sort(connectorLocs.begin(), connectorLocs.end()); // Sorted to find each subcommand substring
		connectorLocs.push_back(commandLine.size()); // One past end index will act like connector
		
		// Runs through subcommands and runs each one
		// Works for connectors with nothing between them (tokenizer will have "" => syntax error, which is expected) 
		for(unsigned int i = 0; i < connectorLocs.size() - 1; ++i) // # of subcommands == # of connectors - 1 (including 0, one-past-end)
		{
			string command;
			vector<char*> args;
			char_separator<char> delim(" ");
			tokenizer<char_separator<char>> tok(commandLine, delim);
			auto iter = tok.begin();
			command = *iter;
			++iter;
			for(; iter != tok.end(); ++iter)
			{
				char* c = const_cast<char*> (iter->c_str());
				args.push_back(c);
			}
		}
	}
	return 0;
}
