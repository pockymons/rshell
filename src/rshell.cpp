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
		string command;
		string commandLine;
		char* arguments[];
		getline(cin, commandLine);
		TOKENIZER tok(commandLine);
		for(TOKENIZER::iterator iter = tok.begin(); iter != tok.end(); ++iter)
		{ 
			command = *iter;
			while(iter != tok.end() && *iter != "&&" *iter != "||" && *iter != ";")
			{
			}
		}
	}
	return 0;
}
