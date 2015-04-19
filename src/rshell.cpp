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

// Chose to write function comparing c-strings rather than copy to string then compare
// Used for checking exit command
bool cStringEqual(char* c1, char* c2)
{
	int i;
	for(i = 0; c1[i] != '\0' && c2[i] != '\0'; ++i)
	{
		if(c1[i] != c2[i])
		{
			return false;
		}
	}
	if(c1[i] != '\0' || c2[i] != '\0')
	{
		return false;
	}
	return true;
}

int main()
{
	while(true) //Shell runs until the exit command
	{
		cout << "$ "; // Prints command prompt
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
		while(commandLine.find("&&", marker) != string::npos)//loc != string::npos) 
		{
			connectorLocs.push_back(commandLine.find("&&", marker));
			marker = commandLine.find("&&", marker) + 2;//loc + 2; // Starts searching after "&&"
		}
		marker = 0;
		while(commandLine.find("||", marker) != string::npos) 
		{
			connectorLocs.push_back(commandLine.find("||", marker)); 
			marker = commandLine.find("||", marker) + 2; // Starts searching after "||"
		}
		marker = 0;
		while(commandLine.find(";", marker) != string::npos)
		{
			connectorLocs.push_back(commandLine.find(";", marker));
			marker =  commandLine.find(";", marker)+ 1; // Starts searching after ";"
		}
		connectorLocs.push_back(0); // Will be sorted and put in beginning
		sort(connectorLocs.begin(), connectorLocs.end()); // Sorted to find each subcommand substring
		connectorLocs.push_back(commandLine.size()); // One past end index will act like connector
		
		// Runs through subcommands and runs each one
		// Works for connectors with nothing between them (tokenizer will have "" => syntax error, which is expected) 
		for(unsigned int i = 0; i < connectorLocs.size() - 1; ++i) // # of subcommands == # of connectors - 1 (including 0, one-past-end)
		{
			// For parsing line of commands; delimiter is whitespace, each token will be a command or an argument
			vector<char*> args;
			char_separator<char> delim(" ");
			tokenizer<char_separator<char>> tok(commandLine.substr(connectorLocs.at(i), connectorLocs.at(i+1) - connectorLocs.at(i)), delim);
			// First token is the command, other tokens are the arguments
			for(auto iter = tok.begin(); iter != tok.end(); ++iter)
			{
				args.push_back(const_cast<char*> (iter->c_str()));
			}
			
			char* exitCString = const_cast<char*> ("exit"); 
				
			//cout << cStringEqual(args.at(0), exitCString) << endl;
			if(cStringEqual(args.at(0), exitCString)) // if command is exit, exit shell
			{
				exit(0);
			}

			// Executes commands/takes care of errors
			int pid = fork();
			if(pid == -1) // If fork fails
			{
				perror("Fork error");
				exit(1);
			}
			else
			{
				if(pid == 0) // Child process
				{
					execvp(args.at(0), &args.at(0));
					// Following don't run if execvp succeeds
					perror("Command execution error");
					_exit(1);
				}
				else // Parent process
				{
					int status; // Status isn't used but might use in future?
					if(wait(&status) == -1) // If child process has error
					{
						perror("Child process error");
						// exits if next connector is && or one-past-end element
						// continues if next connector is ; or ||
						if(connectorLocs.at(i+1) == commandLine.size() || commandLine.at(connectorLocs.at(i+1)) == '&')
						{
							break;
						}
					}
				}
			}
		}
	}
	return 0;
}
