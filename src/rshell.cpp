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
#include <cstdio>
#include <cstring>
#include <cmath>
#include <stdlib.h>
#include <limits.h>
#include <stdlib.h>
#include <stack>

using namespace std;
using namespace boost;

stack<int> backgroundPIDStack;

// Could have used tuple; However, originally, pair was used, so making a triple struct was faster
template<typename T1, typename T2, typename T3>
struct triple
{
	public:
	T1 first;
	T2 second;
	T3 third;

	triple(T1 f, T2 s, T3 t) : first(f), second(s), third(t)
	{
	}
};

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
	
	// Man page recommends to close before dup2-ing
	if(-1 == close(replaceFD))
	{
		perror("Close replace fd");
		_exit(1);
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
		// Man page recommends closing first
		if(-1 == close(replaceFD))
		{
			perror("Close replace fd");
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
		int fd[2];
		if(-1 == pipe(fd))
		{
			perror("Pipe");
			_exit(1);
		}
		if(-1 == close(replaceFD))
		{
			perror("Close");
			_exit(1);
		}
		if(-1 == dup2(fd[0], replaceFD))
		{
			perror("Dup");
			_exit(1);
		}
		while(file.size() > 0)
		{
			int sizeofBuf = BUFSIZ;
			for(unsigned int i = 0; i < BUFSIZ; ++i)
			{
				if(i < file.size())
				{
					buf[i] = file.at(i);
				}
				else
				{
					sizeofBuf = i;
					break;
				}
			}
			file.erase(0, BUFSIZ);
			if(-1 == write(fd[1], buf, sizeofBuf))
			{
				perror("Write");
				_exit(1);
			}
		}
		if(-1 == write(fd[1], "\n", 1))
		{
			perror("Write");
			_exit(1);
		}
		if(-1 == close(fd[1]))
		{
			perror("Close fd");
			_exit(1);
		}
	}
}

void ioAndExec(vector<vector<char*>>& args, vector<vector<triple<string, string, int>>>& ioFiles, int argIndex)
{
	for(auto iter = ioFiles.at(argIndex).begin(); iter != ioFiles.at(argIndex).end(); ++iter)
	{
		if(iter->first == ">")
		{
			outputRedir(iter->second, iter->third, 1);
		}
		else if(iter->first == ">>")
		{
			outputRedir(iter->second, iter->third, 2);
		}
		else if(iter->first == "<")
		{
			inputRedir(iter->second, iter->third, 1);
		}
		else if(iter->first == "<<<")
		{
			inputRedir(iter->second, iter->third, 3);
		}
		else
		{
			// Shouldn't ever happen but just in case
			cerr << "IO redirection error" << endl;
			_exit(1);
		}
	}
	execvp(args.at(argIndex).at(0), &(args.at(argIndex).at(0)));
	// Following don't run if execvp succeeds
	perror("Command execution error");
	_exit(1);
}

// Each vector within the first vector has the commands between pipes; assume vector is size of 2 or more; checks this condition in main
void myPipe(vector<vector<char*>>& args, vector<vector<triple<string, string, int>>>& ioFiles)
{
	// First command of pipe; leftmost
	int fd1[2];
	if(-1 == pipe(fd1))
	{
		perror("Pipe");
		_exit(1);
	}
	int pid = fork();
	if(-1 == pid)
	{
		perror("Fork");
		_exit(1);
	}
	else
	{
		if(pid == 0)
		{
			if(-1 == close(1))
			{
				perror("Close stdout");
				_exit(1);
			}
			if(-1 == dup2(fd1[1], 1))
			{
				perror("Redirect stdout");
				_exit(1);
			}
			ioAndExec(args, ioFiles, 0);
		}
		else
		{
			// Do nothing 
			// I like this format
			// Only case where the process will continue in the function
		}
	}

	int fd2[2];
	if(args.size() > 2)
	{
		if(-1 == pipe(fd2))
		{
			perror("Pipe");
			_exit(1);
		}
	}
	pid = fork();
	for(unsigned int i = 1; i < args.size() - 1; ++i)
	{
		if(-1 == pid)
		{
			perror("Fork");
			// Wait for previous forks
			for(unsigned int j = 0; j < i - 1; ++j)
			{
				int status;
				if(-1 == wait(&status))
				{
					perror("Wait");
					_exit(1);
				}
			}
			_exit(1);
		}
		else
		{
			// If i is odd
			if(i % 2 == 1)
			{
				if(pid == 0)
				{
					if(-1 == close(0))
					{
						perror("Close stdin");
						_exit(1);
					}
					if(-1 == dup2(fd1[0], 0))
					{
						perror("Redirect stdin");
						_exit(1);
					}
					if(-1 == close(fd1[1]))
					{
						perror("Close fd1");
						_exit(1);
					}

					if(-1 == close(1))
					{
						perror("Close stdout");
						_exit(1);
					}
					if(-1 == dup2(fd2[1], 1))
					{
						perror("Redirect stdout");
						_exit(1);
					}

					ioAndExec(args, ioFiles, i);
				}
				else
				{
					if(-1 == close(fd1[0]))
					{
						perror("Close fd1");
						// Wait for previously forked processes to end
						for(unsigned int j = 0; j < i; ++j)
						{
							int status;
							if(-1 == wait(&status))
							{
								perror("Wait");
								_exit(1);
							}
						}
						_exit(1);
					}
					if(-1 == close(fd1[1]))
					{
						perror("Close fd1");
						// Wait for previously forked processes to end
						for(unsigned int j = 0; j < i; ++j)
						{
							int status;
							if(-1 == wait(&status))
							{
								perror("Wait");
								_exit(1);
							}
						}
						_exit(1);
					}
					if(i != args.size() - 1)
					{
						if(-1 == pipe(fd1))
						{
							perror("Pipe fd1");
							_exit(1);
						}
					}
					pid = fork();
					if(-1 == pid)
					{
						perror("Fork");
						_exit(1);
					}
					// Continues to next iteration after this
				}
			}
			else
			{
				if(pid == 0)
				{
					if(-1 == close(0))
					{
						perror("Close stdin");
						_exit(1);
					}
					if(-1 == dup2(fd2[0], 0))
					{
						perror("Redirect stdin");
						_exit(1);
					}
					if(-1 == close(fd2[1]))
					{
						perror("Close fd2");
						_exit(1);
					}

					if(-1 == close(1))
					{
						perror("Close stdout");
						_exit(1);
					}
					if(-1 == dup2(fd1[1], 1))
					{
						perror("Redirect stdout");
						_exit(1);
					}

					ioAndExec(args, ioFiles, i);
				}
				else
				{
					if(-1 == close(fd2[0]))
					{
						perror("Close fd1");
						// Wait for previously forked processes to end
						for(unsigned int j = 0; j < i; ++j)
						{
							int status;
							if(-1 == wait(&status))
							{
								perror("Wait");
								_exit(1);
							}
						}
						_exit(1);
					}
					if(-1 == close(fd2[1]))
					{
						perror("Close fd1");
						// Wait for previously forked processes to end
						for(unsigned int j = 0; j < i; ++j)
						{
							int status;
							if(-1 == wait(&status))
							{
								perror("Wait");
								_exit(1);
							}
						}
						_exit(1);
					}
					
					if(i != args.size() - 1)
					{
						if(-1 == pipe(fd2))
						{
							perror("Pipe fd2");
							_exit(1);
						}
					}
					pid = fork();
					// Continues to next iteration after this
				}
			}
		}
	}

	if(pid == -1)
	{
		perror("Fork");
		for(unsigned int j = 0; j < args.size() - 1; ++j)
		{
			int status;
			if(-1 == wait(&status))
			{
				perror("Wait");
				_exit(1);
			}
		}
		_exit(1);
	}
	else
	{
		if(args.size() % 2 == 1)
		{
			if(pid == 0)
			{
				if(-1 == close(0))
				{
					perror("Close stdin");
					_exit(1);
				}
				if(-1 == dup2(fd2[0], 0))
				{
					perror("Redirect stdin");
					_exit(1);
				}
				if(-1 == close(fd2[1]))
				{
					perror("Close fd2");
					_exit(1);
				}
				ioAndExec(args, ioFiles, args.size() - 1);
			}
			else
			{
				if(-1 == close(fd2[0]))
				{
					perror("Close fd2");
					for(unsigned int j = 0; j < args.size(); ++j)
					{
						int status;
						if(-1 == wait(&status))
						{
							perror("Wait");
							_exit(1);
						}
					}
					_exit(1);
				}
				if(-1 == close(fd2[1]))
				{
					perror("Close fd2");
					for(unsigned int j = 0; j < args.size(); ++j)
					{
						int status;
						if(-1 == wait(&status))
						{
							perror("Wait");
							_exit(1);
						}
					}
					_exit(1);
				}
			}
		}
		else
		{
			if(pid == 0)
			{
				if(-1 == close(0))
				{
					perror("Close stdin");
					_exit(1);
				}
				if(-1 == dup2(fd1[0], 0))
				{
					perror("Redirect stdin");
					_exit(1);
				}
				if(-1 == close(fd1[1]))
				{
					perror("Close fd1");
					_exit(1);
				}
				ioAndExec(args, ioFiles, args.size() - 1);
			}
			else
			{
				if(-1 == close(fd1[0]))
				{
					perror("Close fd1");
					for(unsigned int j = 0; j < args.size(); ++j)
					{
						int status;
						if(-1 == wait(&status))
						{
							perror("Wait");
							_exit(1);
						}
					}
					_exit(1);
				}
				if(-1 == close(fd1[1]))
				{
					perror("Close fd1");
					for(unsigned int j = 0; j < args.size(); ++j)
					{
						int status;
						if(-1 == wait(&status))
						{
							perror("Wait");
							_exit(1);
						}
					}
					_exit(1);
				}
			}
		}
	}

	// totalStatus will equal 1 if there is any error
	int totalStatus = 0;
	for(unsigned int j = 0; j < args.size(); ++j)
	{
		int status;
		if(-1 == wait(&status))
		{
			perror("Wait");
			_exit(1);
		}
		totalStatus = (totalStatus | WEXITSTATUS(status));
	}
	// Bash _exits entire command, if one between pipe fails
	if(totalStatus != 0)
	{
		_exit(1);
	}
	_exit(0);
}

//3 choices:
//cd <PATH> will change working directory to PATH
//cd will go to home directory
//cd - will go to previous working directory 
// Returns -1 if error, like a lot of syscalls; I don't use the return value, but just in case for later
int changeDirectory(char* path)
{
	if(path == NULL)
	{
		char* newCurrentDir;
		if(-1 == chdir(newCurrentDir = getenv("HOME")))
		{
			perror("Changing directory");
			return -1;
		}
		if(-1 == setenv("OLDPWD", getenv("PWD"), 1))
		{
			perror("Setting old pwd");
			return -1;
		}
		if(-1 == setenv("PWD", newCurrentDir, 1))
		{
			perror("Setting pwd");
			return -1;
		}
	}
	else if(strcmp(path, "-") == 0)
	{
		char* newCurrentDir = getenv("OLDPWD");
		if(-1 == chdir(newCurrentDir))
		{
			perror("Changing directory");
			return -1;
		}
		if(-1 == setenv("OLDPWD", getenv("PWD"), 1))
		{
			perror("Setting old pwd");
			return -1;
		}
		if(-1 == setenv("PWD", newCurrentDir, 1))
		{
			perror("Setting pwd");
			return -1;
		}
	}
	else // is PATH
	{
		string strPath = path;
		if(strPath.size() > 0 && strPath.at(0) == '~')
		{
			if(strPath.size() == 1 || strPath.at(1) == '/')
			{
				strPath = strPath.substr(1);
				string tempString = getenv("HOME");
				strPath = tempString + strPath;
			}
		}
		char fullPath[PATH_MAX];
		if(NULL == realpath(strPath.c_str(), fullPath))
		{
			perror("Finding full path"); 
			return -1;
		}
		if(-1 == chdir(fullPath))
		{
			perror("Changing directory");
			return -1;
		}
		if(-1 == setenv("OLDPWD", getenv("PWD"), 1))
		{
			perror("Setting old pwd");
			return -1;
		}
		if(-1 == setenv("PWD", fullPath, 1))
		{
			perror("Setting pwd");
			return -1;
		}
	}
	return 0;
}

void nlhandler(int h)
{
	cout << endl;
}
/*
void ctrlZhandler(int h)
{
	cerr << 1 << endl;
	backgroundPIDStack.push(getpid());

	struct sigaction ctrlZinhandler = {0};
	ctrlZinhandler.sa_handler = SIG_DFL;
	if(-1 == sigaction(SIGTSTP, &ctrlZinhandler, NULL))
	{
		perror("sigaction SIGTSTP in handler");
		_exit(1);
	}
	if(0 != raise(SIGTSTP))
	{
		cerr << "Raise error" << endl;
		_exit(1);
	}
}
void fgbghandler(int h)
{
	int status = 0;
	if(-1 == waitpid(backgroundPIDStack.top(), &status, 0))
	{
		perror("Wait bg");
		exit(1);
	}
}

bool fgbg()
{
	if(backgroundPIDStack.empty())
	{
		cerr << "No process in background" << endl;
		return false;
	}
	if(-1 == kill(backgroundPIDStack.top(), SIGCONT))
	{
		perror("Continue bg process");
		exit(1);
	}
	return true;
	if(!fg)// if bg command
	{
		struct sigaction fgbgChildhandler = {0};
		fgbgChildhandler.sa_handler = fgbghandler;
		if(-1 == sigaction(SIGTSTP, &fgbgChildhandler, NULL))
		{
			perror("Child sigaction fgbg");
			_exit(1);
		}
	}
}
*/

int main()
{
	// Signal handling
	struct sigaction ctrlCParentHandler = {0};
	ctrlCParentHandler.sa_handler = nlhandler;
	
	if(-1 == sigaction(SIGINT, &ctrlCParentHandler, NULL))
	{
		perror("Parent sigaction SIGINT");
		exit(1);
	}
/*
	struct sigaction ctrlZParentHandler = {0};
	ctrlZParentHandler.sa_handler = nlhandler;
	
	if(-1 == sigaction(SIGTSTP, &ctrlZParentHandler, NULL))
	{
		perror("Parent sigaction SIGTSTP");
		exit(1);
	}*/

	while(true) //Shell runs until the exit command
	{
		char* loginName = getlogin();
		if(!loginName)//Returns NULL if fails
		{
			perror("Login error");
			exit(1);
		}

		char hostName[128];
		int hostStatus = gethostname(hostName, sizeof(hostName));
		if(hostStatus == -1)// hostatus is -1 for error
		{
			perror("Hostname error");
			exit(1);
		}
		
		string currentWorkingDir = getenv("PWD");
		string tempHomeDir = getenv("HOME");
		if(tempHomeDir == currentWorkingDir.substr(0, tempHomeDir.size()))
		{
			currentWorkingDir = currentWorkingDir.substr(tempHomeDir.size());
			// Probably won't happen, just in case
			if(currentWorkingDir.size() >0 && currentWorkingDir.at(0) != '/')
			{
				currentWorkingDir = "/" + currentWorkingDir;
			}
			currentWorkingDir = "~" + currentWorkingDir;
		}
		cout << loginName << "@" << hostName << ":" << currentWorkingDir << " $ "; // Prints command prompt
		string commandLine;
		cin.clear();
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
			vector<vector<string>> strArgs;
			vector<vector<triple<string, string, int>>> ioFiles; // First string is which io redirect (<, >, >>), second is file
			// Use this instead of map, since map doesn't allow duplicates for a key

			// FOLLOWING LINE WILL BREAK IF USED DIRECTLY IN TOKENIZER
			string subcommand = commandLine.substr(connectorLocs.at(i) + offset, connectorLocs.at(i+1) - connectorLocs.at(i) - offset);
			//typedef tokenizer<char_separator<char>> tokenizer; // Used to use this
			//cout << "sub: " << subcommand << endl; // DEBUGGING
			char_separator<char> pipeSep("|");
			tokenizer<char_separator<char>> tokPipe(subcommand, pipeSep);
			int pipeSectionCounter = 0;
			for(auto iterPipe = tokPipe.begin(); iterPipe != tokPipe.end(); ++iterPipe)
			{
				strArgs.push_back(vector<string>());
				ioFiles.push_back(vector<triple<string, string, int>>());

				string subSubcommand = *iterPipe;
				char_separator<char> sep(" \t");
				tokenizer<char_separator<char>> tok(subSubcommand, sep);

				// For redirection cases with spaces
				string addBegin = "";
				// First token is the command, other tokens are the arguments
				for(auto iter = tok.begin(); iter != tok.end(); ++iter)
				{
					//cout << "tok: " << *iter << endl; // DEBUGGING
					string tokenString = *iter;

					if(addBegin != "")
					{
						tokenString = addBegin + tokenString;
						addBegin = "";
					}
					
					int oFrom = 1; // 1 for stdout
					bool oFromChanged = false;
					int firstOut = tokenString.find(">");
					int o = 0;
					for(o = 0; o < firstOut; ++o)
					{
						int fdnum = 0;
						// if is digit
						if(tokenString.at(o) > 47 && tokenString.at(o) < 58)
						{
							int tempNum = tokenString.at(o);
							tempNum -= 48;
							fdnum += pow(10, o) * tempNum;
						}
						else
						{
							o = 0;
							break;
						}
						if(o == firstOut - 1)
						{
							oFromChanged = true;
							oFrom = fdnum;
						}
					}
					
					// For redirection cases with spaces; eg. >> file
					int takeAwayLast = 0;
					if(tokenString.size() > 0 && tokenString.at(tokenString.size() - 1) == '>')
					{
						++takeAwayLast;
						if(tokenString.size() > 1 && tokenString.at(tokenString.size() - 2) == '>')
						{
							++takeAwayLast;
						}
						if(oFromChanged && (unsigned int)(o + takeAwayLast) == tokenString.size())
						{
							takeAwayLast += o;
						}
					}
					if(tokenString.size() > 0 && tokenString.at(tokenString.size() - 1) == '<')
					{
						++takeAwayLast;
						if(tokenString.size() > 1 && tokenString.at(tokenString.size() - 2) == '<' && tokenString.at(tokenString.size() - 3) == '<')
						{
							// <<<
							++takeAwayLast;
							++takeAwayLast;
						}
					}
					addBegin = tokenString.substr(tokenString.size() - takeAwayLast, takeAwayLast);
					tokenString = tokenString.substr(0, tokenString.size() - takeAwayLast);


					if(oFromChanged && (unsigned int)(o + takeAwayLast) == tokenString.size())
					{
						takeAwayLast += o;
					}

					//Token has no spaces
					//Output redirection
					
					if(oFromChanged)
					{
						tokenString.erase(0, firstOut);
					}

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
					
						triple<string, string, int> tempTrip("", redirFile, oFrom);	
						if(offset == 1)
						{
							tempTrip.first = ">";
						}
						else
						{
							tempTrip.first = ">>";
						}
						ioFiles.at(pipeSectionCounter).push_back(tempTrip);
						
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
					
						triple<string, string, int > tempTrip("", redirFile, 0);	
						if(offset == 1)
						{
							tempTrip.first = "<";
						}
						else
						{
							tempTrip.first = "<<<";
						}
						ioFiles.at(pipeSectionCounter).push_back(tempTrip);
						
						tokenString = tokenString.substr(0, iRedir1) + tokenString.substr(nextRedir, tokenString.size() - nextRedir);
					}
					if(tokenString.size() > 0)
					{
						strArgs.at(pipeSectionCounter).push_back(tokenString);
					}
				}
				++pipeSectionCounter;
			}

			// Copy strArgs to vector of c-strings
			// NEED TO DO IT THIS WAY OR THERE'S ISSUES WITH POINTERS
			vector<vector<char*>> args;
			for(unsigned int j = 0; j < strArgs.size(); ++j)
			{
				args.push_back(vector<char*>());
				for(auto str : strArgs.at(j))
				{
					args.at(j).push_back(const_cast<char*> (str.c_str()));
				}
				args.at(j).push_back(NULL); // NULL terminating at the end of vector/array
			
			
				//Blank command or consecutive connectors
				if(args.at(j).size() == 1 && ioFiles.at(j).size() == 0)
				{
					cerr << "Syntax error" << endl;
					exit(1);	
				}

				if(args.at(j).size() > 1 && strcmp(args.at(j).at(0), "exit") == 0) // if command is exit, exit shell
				{
					exit(0);
				}
			}
			
			// Doesn't matter if it is args or ioFiles; both should be the same size
			// If args size is 1, there are no pipes
			if(args.size() == 1)
			{
				// Checks for cd
				if(args.at(0).size() > 1 && strcmp(args.at(0).at(0), "cd") == 0)
				{
					char* cdPath = NULL;
					//If there is an argument
					if(args.at(0).size() > 1)
					{
						cdPath = args.at(0).at(1);
					}
					changeDirectory(cdPath);
					continue;
				}
				/*
				if(args.at(0).size() == 2 && strcmp(args.at(0).at(0), "fg") == 0)
				{
					if(!fgbg())
					{
						continue;
					}
					int fgStatus;
					if(-1 == wait(&fgStatus))
					{
						perror("Wait fg");
						exit(1);
					}
					backgroundPIDStack.pop();
					continue;
				}
				if(args.at(0).size() == 2 && strcmp(args.at(0).at(0), "bg") == 0)
				{
					fgbg();
					continue;
				}*/

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
						struct sigaction childCtrlCHandler = {0};
						childCtrlCHandler.sa_handler = SIG_DFL;
						if(-1 == sigaction(SIGINT, &childCtrlCHandler, NULL))
						{
							perror("Child sigaction SIGINT");
							_exit(1);
						}

						// Reminder: vector<triple<string, string>> ioFiles; First string is which io redirect (<, >, >>), second is file
						ioAndExec(args, ioFiles, 0);
					}
					else // Parent process
					{
						int status; 
						int waitVar;
						do
						{
							waitVar = waitpid(pid, &status, WUNTRACED);
						}
						while(waitVar == -1 && errno == EINTR);
						if(waitVar == -1) // If child process has error
						{
							perror("Child process error");
							exit(1);
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
			else
			{
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
						struct sigaction childCtrlCHandler = {0};
						childCtrlCHandler.sa_handler = SIG_DFL;
						if(-1 == sigaction(SIGINT, &childCtrlCHandler, NULL))
						{
							perror("Child sigaction SIGINT");
							_exit(1);
						}
						myPipe(args, ioFiles);
					}
					else
					{
						int status; 
						int waitVar;
						do
						{
							waitVar = wait(&status);
						}
						while(waitVar == -1 && errno == EINTR);
						if(waitVar == -1) // If child process has error
						{
							perror("Child process error");
							exit(1);
							// exits if next connector is && or one-past-end element
							// continues if next connector is ; or ||
							
						}
						else
						{
							int exitStatus = WEXITSTATUS(status); // Checks whether returns 0/1 when exiting
							if(exitStatus == 1) // If unsuccessful command
							{
								// Bash exits entire command if one part of pipe fails
								break;
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
	}
	return 0;
}
