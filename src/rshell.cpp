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
		char* arguments[];
		getline(cin, commandLine);

		// Accounts for comments by removing parts that are comments
		// TODO: Account for escape character + comment (\#)
		if(commandLine.find('#') != -1)
		{
			commandLine = commandLine.substr(commandLine.find('#'));
		}

		// Finds locations of connectors; a && b, && has a location of 3
		vector<int> connectorLocs;
		unsigned int marker = 0; // Marks location to start find() from
		while((int loc = commandLine.find(" && ", marker)) != -1) 
		{
			connectorLocs.push_back(loc + 1); // Add 1 so that the location is &, not space
			marker = loc + 4; // Starts searching after " && "
		}
		marker = 0;
		while((int loc = commandLine.find(" || ", marker)) != -1) 
		{
			connectorLocs.push_back(loc + 1); // Add 1 so that the location is &, not space
			marker = loc + 4; // Starts searching after " && "
		}
		marker = 0;
		while((int loc = commandLine.find(" ; ", marker)) != -1) 
		{
			connectorLocs.push_back(loc + 1); // Add 1 so that the location is &, not space
			marker = loc + 3; // Starts searching after " && "
		}
		sort(connectorLocs.begin(), connectorLocs.end());

		char_seperator<char> delim(" ");
		tokenizer<char_seperator<char>> tok(commandLine, delim);
		for(auto iter : tok) // = tok.begin(); iter != tok.end(); ++iter)
		{			
			string command;
			command = *iter;
		}
	}
	return 0;
}
