#include <iostream>
#include <string>
#include <boost/tokenizer.hpp> //Boost tokenizer
#include <unistd.h> // Fork()
#include <sys/types.h> // Wait()
#include <sys/wait.h> // Wait()
#include <sys/stat.h>
#include <vector>
#include <stdio.h> //Perror()
#include <errno.h> // Perror()
#include <algorithm>
#include <utility>
#include <fcntl.h>

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

// file is the file to be outputted to
// replaceFD is the file directory to copy to
// numBrackets == 1 if ">", numBrackets == 2 if ">>"
// used in child process; uses _exit() for that reason
void outputRedir(string file, int replaceFD, int numBrackets)
{
	int fdOpen;
	if(numBrackets == 1)
	{
		if(-1 == (fdOpen = open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)))
		{
			perror("Open");
			_exit(1);
		}
	}
	if(numBrackets == 2)
	{
		if(-1 == (fdOpen = open(file.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR)))
		{
			perror("Open at");
			_exit(1);
		}
	}
	
	if(-1 == dup2(fdOpen, replaceFD))
	{
		perror("Open");
		_exit(1);
	}
	
	if(-1 == close(fdOpen))
	{
		perror("Close");
		_exit(1);
	}
}

// file is the file to be outputted to
// replaceFD is the file directory to copy to
// numBrackets == 1 if "<", numBrackets == 3 if "<<<"
// Used in child process
void inputRedir(string file, int replaceFD, int numBrackets)
{
	int fdInput;
	if(numBrackets == 1)
	{
		if(-1 == (fdInput = open(file.c_str(), O_RDONLY)))
		{
			perror("Open");
			_exit(1);
		}
		if(-1 == dup2(fdInput, replaceFD))
		{
			perror("Open");
			_exit(1);
		}
		
		if(-1 == close(fdInput))
		{
			perror("Close");
			_exit(1);
		}
	}
	if(numBrackets == 3)
	{
		// In this case, file isn't actually a file; I just didn't rename it; in this case, file is the string to be redirected
		char buf[BUFSIZ];
		while(file.size() > 0)
		{

			for(unsigned int i = 0; i < BUFSIZ; ++i)
			{
				if(i < file.size())
				{
					buf[i] = file.at(i);
				}
				else
				{
					break;
				}
			}
			file.erase(0, BUFSIZ);
			if(-1 == write(replaceFD, buf, sizeof(buf)))
			{
				perror("Write");
			}
		}
	}
}

int main()
{
	//Hostname/loginname won't change
	char* loginName = getlogin();
	if(!loginName)//Returns NULL if fails
	{
		perror("Login error");
		exit(1);
	}

	char hostName[64];
	int hostStatus = gethostname(hostName, sizeof(hostName));
	if(hostStatus == -1)// hostatus is -1 for error
	{
		perror("Hostname error");
		exit(1);
	}

	while(true) //Shell runs until the exit command
	{
		cout << loginName << "@" << hostName << " $ "; // Prints command prompt
		string commandLine;
		getline(cin, commandLine); 
		if(commandLine.size() == 0)
		{
			continue;
		}

		// Accounts for comments by removing parts that are comments
		if(commandLine.find(" #") != string::npos)
		{
			commandLine = commandLine.substr(0, commandLine.find(" #"));
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
		// # of subcommands == # of connectors - 1 (including 0, one-past-end)
		for(unsigned int i = 0; i < connectorLocs.size() - 1; ++i) 
		{
			int offset = 0; // Tells how much offset for connectors (&&, ||, ;)
			if(commandLine.at(connectorLocs.at(i)) == '&' || commandLine.at(connectorLocs.at(i)) == '|')
			{
				offset = 2;
			}
			else
			{
				if(commandLine.at(connectorLocs.at(i)) == ';')
				{
					offset = 1;
				}
			}

			//cout << commandLine.at(connectorLocs.at(i)) << endl; // DEBUGGING
			//cout << offset << endl; // DEBUGGING
			
			// For parsing line of commands; delimiter is whitespace, each token will be a command or an argument
			vector<string> strArgs;
			vector<pair<string, string>> ioFiles; // First string is which io redirect (<, >, >>), second is file
			// Use this instead of map, since map doesn't allow duplicates for a key

			char_separator<char> sep(" \t");
			// FOLLOWING LINE WILL BREAK IF USED DIRECTLY IN TOKENIZER
			string subcommand = commandLine.substr(connectorLocs.at(i) + offset, connectorLocs.at(i+1) - connectorLocs.at(i) - offset);
			//typedef tokenizer<char_separator<char>> tokenizer; // Used to use this
			//cout << "sub: " << subcommand << endl; // DEBUGGING
			tokenizer<char_separator<char>> tok(subcommand, sep);
			// First token is the command, other tokens are the arguments
			for(auto iter = tok.begin(); iter != tok.end(); ++iter)
			{
				//cout << "tok: " << *iter << endl; // DEBUGGING
				string tokenString = *iter;

				//Token has no spaces
				//Output redirection
				string::size_type oRedir1 = 0;
				string::size_type oRedir2 = 0;
				// find(">") will not be npos if oRedir2 = tokenString.find(">>")
				while((oRedir1 = tokenString.find(">")) != string::npos)
				{
					oRedir2 = tokenString.find(">>");
					unsigned int offset = 0;
					if(oRedir1 < oRedir2)
					{
						offset = 1;
					}
					else
					{
						offset = 2;
					}

					string::size_type nextRedir;
					// Will include ">>" and "<<<"
					nextRedir = min(tokenString.find(">", oRedir1 + offset), tokenString.find("<", oRedir1 + offset));
					if(nextRedir == string::npos)
					{
						// For substring purposes
						nextRedir = tokenString.size();
					}

					string redirFile = tokenString.substr(oRedir1 + offset, nextRedir - oRedir1 - offset);
				
					pair<string, string> tempPair("", redirFile);	
					if(offset == 1)
					{
						tempPair.first = ">";
					}
					else
					{
						tempPair.first = ">>";
					}
					ioFiles.push_back(tempPair);
					
					tokenString = tokenString.substr(0, oRedir1) + tokenString.substr(nextRedir, tokenString.size() - nextRedir);
				}
				
				//Input redirection
				string::size_type iRedir1 = 0;
				string::size_type iRedir3 = 0;
				while((iRedir1 = tokenString.find("<")) != string::npos)
				{
					iRedir3 = tokenString.find("<<<");
					unsigned int offset = 0;
					if(iRedir1 < iRedir3)
					{
						offset = 1;
					}
					else
					{
						offset = 3;
					}
						
					string::size_type nextRedir;
					// Will include ">>" and "<<<"
					nextRedir = min(tokenString.find("<", iRedir1 + offset), tokenString.find("<", iRedir1 + offset));
					if(nextRedir == string::npos)
					{
						// For substring purposes
						nextRedir = tokenString.size();
					}

					string redirFile = tokenString.substr(iRedir1 + offset, nextRedir - iRedir1 - offset);
				
					pair<string, string> tempPair("", redirFile);	
					if(offset == 1)
					{
						tempPair.first = "<";
					}
					else
					{
						tempPair.first = "<<<";
					}
					ioFiles.push_back(tempPair);
					
					tokenString = tokenString.substr(0, iRedir1) + tokenString.substr(nextRedir, tokenString.size() - nextRedir);
				}
				if(tokenString.size() > 0)
				{
					strArgs.push_back(tokenString);
				}
			}

			// Copy strArgs to vector of c-strings
			// NEED TO DO IT THIS WAY OR THERE'S ISSUES WITH POINTERS
			vector<char*> args;
			for(auto str : strArgs)
			{
				args.push_back(const_cast<char*> (str.c_str()));
			}
			args.push_back(NULL); // NULL terminating at the end of vector/array
			
			//Blank command or consecutive connectors
			if(args.size() == 1 && ioFiles.size() == 0)
			{
				continue;
			}
			
			char* exitCString = const_cast<char*> ("exit"); 
				
			//cout << cStringEqual(args.at(0), exitCString) << endl; // DEBUGGING
			if(args.size() > 1 && cStringEqual(args.at(0), exitCString)) // if command is exit, exit shell
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
					// Reminder: vector<pair<string, string>> ioFiles; First string is which io redirect (<, >, >>), second is file
					for(auto iter = ioFiles.begin(); iter != ioFiles.end(); ++iter)
					{
						if(iter->first == ">")
						{
							outputRedir(iter->second, 1, 1);
						}
						else if(iter->first == ">>")
						{
							outputRedir(iter->second, 1, 2);
						}
						else if(iter->first == "<")
						{
							inputRedir(iter->second, 0, 1);
						}
						else if(iter->first == "<<<")
						{
							inputRedir(iter->second, 0, 3);
						}
						else
						{
							// Shouldn't ever happen but just in case
							cerr << "IO redirection error" << endl;
							_exit(1);
						}
					}
					execvp(args.at(0), &(args[0]));
					// Following don't run if execvp succeeds
					perror("Command execution error");
					_exit(1);
				}
				else // Parent process
				{
					int status; // Status isn't used but might use in future?
					int waitVar = wait(&status);
					if(waitVar == -1) // If child process has error
					{
						perror("Child process error");
						// exits if next connector is && or one-past-end element
						// continues if next connector is ; or ||
						
					}
					else
					{
						int exitStatus = WEXITSTATUS(status); // Checks whether returns 0/1 when exiting
						if(exitStatus == 1) // If unsuccessful command
						{
							if(connectorLocs.at(i+1) < commandLine.size() && 
								commandLine.at(connectorLocs.at(i+1)) == '&')
							{
								//cout << commandLine.at(connectorLocs.at(i+1)) << endl; // DEBUGGING
								break;
							}
						}
						else
						{
							if(connectorLocs.at(i+1) < commandLine.size() && 
								commandLine.at(connectorLocs.at(i+1)) == '|')
							{
								//cout << commandLine.at(connectorLocs.at(i+1)) << endl; // DEBUGGING
								break;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
