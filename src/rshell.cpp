#include <iostream>
#include <string>
#include <boost/tokenizer.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

using namespace std;
using namespace boost;

#define TOKENIZER tokenizer<char_seperator<char>> 

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

		TOKENIZER tok(commandLine);
		for(TOKENIZER::iterator iter = tok.begin(); iter != tok.end(); ++iter)
		{			
			string command;
			command = *iter;
			while(iter != tok.end() && *iter != "&&" *iter != "||" && *iter != ";")
			{
				++iter;
			}
		}
	}
	return 0;
}
