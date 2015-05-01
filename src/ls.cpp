#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <cmath>
#include <unistd.h>

#include <vector>
#include <string>
#include <stack>
#include <algorithm>
#include <cstring>

using namespace std;

bool compDirent(dirent* d1, dirent* d2)
{
	char arr1[256] = {'a'};
	char arr2[256] = {'a'};
	char* d1Str = arr1;
	char* d2Str = arr2;


	strcpy(d1Str, d1->d_name);
	strcpy(d2Str,d2->d_name);
	if(d1Str[0] == '.')
	{
		strcpy(d1Str, (d1->d_name) + 1);
	}
	if(d2Str[0] == '.')
	{
		strcpy(d2Str, (d2->d_name) + 1);
	}
	for(unsigned int i = 0; i < strlen(d1Str); ++i)
	{
		d1Str[i] = toupper(d1Str[i]);
		d2Str[i] = toupper(d2Str[i]);
	}
	return strcmp(d1Str, d2Str) < 0;
}

// Prints names of files in vector
void printFileNames(vector<dirent*> &d, unsigned int maxLength, unsigned int totalLength, unsigned int winWidth)
{
	// If the total length of all file names + 2 spaces between are less than winlength
	// totalLength is found from c-string/sizeof so it includes '\0'
	// the extra size from '\0' contributes to 1 space for the purpose of boolean exp.
	sort(d.begin(), d.end(), compDirent);
	if(totalLength + d.size() <= winWidth)
	{
		for(auto ent : d)
		{
			cout << ent->d_name << "  ";
		}
	}
	else
	{
		// TODO
	}
	cout << endl;
}

// Prints in long form
void printLongForm(vector<dirent*> d)
{
}

// Prints recursively
void printRecursive(vector<dirent*> d)
{
}

int main(int argc, char** argv)
{
	bool aFlag = false;
	bool lFlag = false;
	bool RFlag = false;	
	vector<string> fileParam;

	unsigned int winWidth;
	winsize ws;

	if(-1 == (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)))
	{
		perror("Finding window width error");
		exit(1);
	}
	winWidth = ws.ws_col;
	
	// Loops through arguments looking for flags/files/directories
	for(int i = 1; i < argc; ++i)
	{
		// Is a flag if first char of c-string is '-'
		if(argv[i][0] == '-')
		{
			// Accounts for flags like -laR, -lR, etc.
			for(int j = 1; argv[i][j] != '\0'; ++j)
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

	sort(fileParam.begin(), fileParam.end());
	
	for(auto str : fileParam)
	{
		vector<dirent*> dirEntries;
		DIR* dirp;
		if(NULL == (dirp = opendir(str.c_str())))
		{
			perror("Error in opening directory");
			continue; // Continue to next parameter
		}

		dirent* tempDirEnt;
		errno = 0;
		// Following two lengths include '\0'
		unsigned int maxLength = 0;
		unsigned int totalLength = 0;
		while(NULL != (tempDirEnt = readdir(dirp)))
		{
			// If points to hidden file and no -a
			if(sizeof(tempDirEnt->d_name) > 0 && tempDirEnt->d_name[0] == '.' && !aFlag)
			{
				continue;
			}
			dirEntries.push_back(tempDirEnt);

			unsigned int tempSize = strlen(tempDirEnt->d_name); 
			if(tempSize > maxLength)
			{
				maxLength = tempSize;
			}
			totalLength += tempSize;
		}

		if(errno != 0)
		{
			perror("Error in reading directory");
			continue;
		}

		if(-1 == closedir(dirp))
		{
			perror("Error in closing directory");
			continue;
		}

		if(RFlag)
		{
		}
		else
		{
			if(lFlag)
			{
			}
			else
			{
				printFileNames(dirEntries, maxLength, totalLength, winWidth);
			}
		}
	}

	return 0;
}
