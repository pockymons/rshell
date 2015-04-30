#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <vector>
#include <string>

using namespace std;

int main(int argc, char** argv)
{
	bool aFlag = false;
	bool lFlag = false;
	bool RFlag = false;	
	vector<string> fileParam;
	
	// Loops through arguments looking for flags/files/directories
	for(int i = 1; i < argc; ++i)
	{
		// Is a flag if first char of c-string is '-'
		if(argv[i][0] == '-')
		{
			// Accounts for flags like -laR, -lR, etc.
			for(int j = 1; argv[i][j] != '\n'; ++j)
			{
				if(argv[i][j] == 'a')
				{
					aFlag = true;
				}
				else if(argv[i][j] == 'l')
				{
					lFlag = true;
				}
				else if(argv[i][j] == 'R')
				{
					RFlag = true;
				}
				else
				{
					// If flag is neither 'l', 'a', nor 'R'
					cerr << "Error: Invalid flag" << endl;
					exit(1);
				}
			}
		}
		else
		{
			// Adds directories/files to vector
			fileParam.push_back(argv[i]); 
		}
	}
	if(fileParam.size() == 0)
	{
		fileParam.push_back("."); // Current directory, if no others specified
	}
	
	for(auto str : fileParam)
	{
		DIR* dirp;
		if(-1 == (dirp = opendir(str.c_str())))
		{
			perror("Error in opening directory");
			exit(1);
		}
	}

	return 0;
}
