#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <cmath>
#include <unistd.h>
#include <iomanip>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

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
	// 2 * d.size() accounts for spaces after each string
	sort(d.begin(), d.end(), compDirent);
	if(totalLength + 2 * d.size() <= winWidth)
	{
		for(auto ent : d)
		{
			cout << ent->d_name << "  ";
		}
	}
	else
	{
		// Prints in columns, if not enough space in one line
		unsigned int columnWidth = maxLength + 2;
		unsigned int numColumns = winWidth / columnWidth;
		// For if a file is longer than window width 
		if(numColumns == 0)
		{
			numColumns = 1;
		}
		unsigned int numRows = ceil(static_cast<double>(d.size()) / numColumns);

		unsigned int counter = 0;
		for(unsigned int i  = 0; i < numRows; ++i)
		{
			for(unsigned int j = 0; j < numColumns; ++j, ++counter)
			{
				// Mainly for last entry to exit out of loop
				if(counter >= d.size())
				{
					break;
				}
				// Needed when columnWidth is longer than window width
				if(columnWidth > winWidth)
				{
					cout << d.at(counter)->d_name;
				}
				else
				{
					cout << left << setw(columnWidth) << d.at(counter)->d_name;
				}
			}
			cout << endl;
		}
	}
	cout << endl;
}

// Used to find hard link integer's width
unsigned int integerWidth(unsigned int n)
{
	unsigned int num = n;
	unsigned int ans = 1;
	while(num > 9)
	{
		++ans;
		num /= 10;
	}
	return ans;
}
// Prints in long form
void printLongForm(vector<dirent*> d)
{
	unsigned int hardLinkWidth = 0;
	unsigned int uWidth = 0;
	unsigned int gWidth = 0;
	//unsigned int tWidth = 0;
	cout << d.size() << endl;

	for(auto ent : d)
	{
		struct stat s;
		cout << ent->d_name << endl;
		char* fileName = ent->d_name;
		if(-1 == stat(fileName, &s))
		{
			perror("Stat error");
			exit(1);
		}
		unsigned int tempHLWidth = integerWidth(s.st_nlink);
		if(hardLinkWidth < tempHLWidth)
		{
			hardLinkWidth = tempHLWidth;
		}
		cout << ent->d_name << endl;
		uid_t test = s.st_uid;
		cout << test << endl;
		cout << ent->d_name << endl;
		struct passwd* pw ;
		cout << ent->d_name << endl;
		if(NULL == (pw = getpwuid(test)))
		{
			perror("Getting user name");
			exit(1);
		}
		cout << test << endl;

		cout << ent->d_name << endl;
		unsigned int tempUWidth = strlen(pw->pw_name);
		cout << tempUWidth << endl;
		if(uWidth < tempUWidth)
		{
			uWidth = tempUWidth;
		}

		struct group* grp = getgrgid(s.st_gid);
		unsigned int tempGWidth = strlen(grp->gr_name);
		if(gWidth < tempGWidth)
		{
			gWidth = tempGWidth;
		}
	}

	for(auto ent : d)
	{
		struct stat s;
		cout << ent->d_name << endl;
		char* fileName = ent->d_name;
		if(-1 == stat(fileName, &s))
		{
			perror("Stat error");
			exit(1);
		}

		cout << "total " << (s.st_blocks / 2) << endl;
		cout << s.st_mode;
	}
}

// Prints recursively
void printRecursive(vector<dirent*> d, char* parent, bool longForm)
{
	for(auto ent : d)
	{
		struct stat s;
		char* fileName = ent->d_name;
		if(-1 == stat(fileName, &s))
		{
			perror("Stat error");
			exit(1);
		}
		if(!(S_ISDIR(s.st_mode)))
		{
		}
		else
		{
		}
	}
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
	
	for(unsigned int i = 0; i < fileParam.size(); ++i)
	{
		vector<dirent*> dirEntries;
		DIR* dirp;
		if(NULL == (dirp = opendir(fileParam.at(i).c_str())))
		{
			perror("Error in opening directory");
			continue; // Continue to next parameter
		}

		if(fileParam.size() > 1)
		{
			cout << fileParam.at(i) << ":" << endl;
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
			printRecursive(dirEntries, NULL, lFlag);
		}
		else
		{
			if(lFlag)
			{
				printLongForm(dirEntries);
			}
			else
			{
				printFileNames(dirEntries, maxLength, totalLength, winWidth);
			}
		}
		if(i != fileParam.size() - 1)
		{
			cout << endl;
		}
	}

	return 0;
}
