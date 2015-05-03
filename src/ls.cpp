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
#include <ctime>

using namespace std;

bool compString(string s1, string s2)
{
	string tempS1 = s1;
	string tempS2 = s2;

	if(tempS1.at(0) == '.')
	{
		tempS1 = tempS1.substr(1);
	}

	if(tempS2.at(0) == '.')
	{
		tempS2 = tempS2.substr(1);
	}

	return strcasecmp(tempS1.c_str(), tempS2.c_str()) < 0;
}

// Prints names of files in vector
void printFileNames(vector<string> &d, unsigned int maxLength, unsigned int totalLength, unsigned int winWidth)
{
	// If the total length of all file names + 2 spaces between are less than winlength
	// 2 * d.size() accounts for spaces after each string
	sort(d.begin(), d.end(), compString);
	if(totalLength + 2 * d.size() <= winWidth)
	{
		for(auto str : d)
		{
			cout << str << "  ";
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

		for(unsigned int i  = 0; i < numRows; ++i)
		{

			unsigned int counter = i;
			for(unsigned int j = 0; j < numColumns; ++j, counter += numRows)
			{
				// Mainly for last entry to exit out of loop
				if(counter >= d.size())
				{
					break;
				}
				// Needed when columnWidth is longer than window width
				if(columnWidth > winWidth)
				{
					cout << d.at(counter);
				}
				else
				{
					cout << left << setw(columnWidth) << d.at(counter);
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
void printLongForm(vector<string> &d, string path)
{
	sort(d.begin(), d.end(), compString);

	unsigned int hardLinkWidth = 0;
	unsigned int uWidth = 0;
	unsigned int gWidth = 0;
	unsigned int sWidth = 0;
	unsigned int total = 0;
	//unsigned int tWidth = 0;

	for(auto str : d)
	{
		struct stat s;
		string fullPath = path;
		fullPath.append(str);
		if(-1 == stat(fullPath.c_str(), &s))
		{
			perror("Stat error 1");
			exit(1);
		}
		unsigned int tempHLWidth = integerWidth(s.st_nlink);
		if(hardLinkWidth < tempHLWidth)
		{
			hardLinkWidth = tempHLWidth;
		}
		unsigned int tempSzWidth = integerWidth(s.st_size);
		if(sWidth < tempSzWidth)
		{
			sWidth = tempSzWidth;
		}
		struct passwd* pw ;
		if(NULL == (pw = getpwuid(s.st_uid)))
		{
			perror("Getting user name");
			exit(1);
		}

		unsigned int tempUWidth = strlen(pw->pw_name);
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

		total += s.st_blocks;
	}
	total /= 2;
	cout << "total " << total << endl;

	for(unsigned int i = 0; i < d.size(); ++i)
	{
		string str = d.at(i);
		string fullPath = path;
		fullPath.append(str);
		struct stat s;
		if(-1 == stat(fullPath.c_str(), &s))
		{
			perror("Stat error 2");
			exit(1);
		}
		if(S_ISREG(s.st_mode))
		{
			cout << '-';
		}
		else if(S_ISDIR(s.st_mode))
		{
			cout << 'd';
		}
		else if(S_ISCHR(s.st_mode))
		{
			cout << 'c';
		}
		else if(S_ISBLK(s.st_mode))
		{
			cout << 'b';
		}
		else if(S_ISFIFO(s.st_mode))
		{
			cout << 'p';
		}
		else if(S_ISLNK(s.st_mode))
		{
			cout << 'l';
		}
		else if(S_ISSOCK(s.st_mode))
		{
			cout << 's';
		}
		else
		{
			cout << '?';
		}

		cout << ((s.st_mode & S_IRUSR) ? 'r' : '-');
		cout << ((s.st_mode & S_IWUSR) ? 'w' : '-');
		cout << ((s.st_mode & S_IXUSR) ? 'x' : '-');


		cout << ((s.st_mode & S_IRGRP) ? 'r' : '-');
		cout << ((s.st_mode & S_IWGRP) ? 'w' : '-');
		cout << ((s.st_mode & S_IXGRP) ? 'x' : '-');

		cout << ((s.st_mode & S_IROTH) ? 'r' : '-');
		cout << ((s.st_mode & S_IWOTH) ? 'w' : '-');
		cout << ((s.st_mode & S_IXOTH) ? 'x' : '-');

		cout << " " << left;

		cout << setw(hardLinkWidth + 1) << s.st_nlink;
		cout << setw(uWidth + 1) << getpwuid(s.st_uid)->pw_name;
		cout << setw(gWidth + 1) << getgrgid(s.st_gid)->gr_name;
		cout << right;
		cout << setw(sWidth) << s.st_size << " ";
		cout << left;

		struct tm* t = localtime(&(s.st_mtime));
		char formattedTime[256];  
		strftime(formattedTime, sizeof(formattedTime), "%b %d %R ", t); 
		// Removes '0' on date
		if(formattedTime[7] == '0')
		{
			formattedTime[7] = ' ';
		}
		if(formattedTime[10] == ' ')
		{
			formattedTime[10] = '0';
		}
		
		cout << formattedTime;

		cout << d.at(i);

		if(i != d.size() - 1)
		{
			cout << endl;
		}
	}
	cout << endl; 
}

// Prints recursively
void printRecursive(vector<string> &d, string path, bool longForm, bool aFlag)
{
	/*if(first)
	{
		cout << path.substr(0, path.size() -1) << ":" << endl;
	}*/
	if(longForm)
	{
		printLongForm(d, path);
	}
	else
	{
		unsigned int winWidth;
		winsize ws;
		if(-1 == (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)))
		{
			perror("Finding window width error");
			exit(1);
		}
		winWidth = ws.ws_col;

		unsigned int maxLength = 0;
		unsigned int totalLength = 0;
		for(auto str : d)
		{
			if(str.size() > maxLength)
			{
				maxLength = str.size();
			}
			totalLength += str.size();
		}

		printFileNames(d, maxLength, totalLength, winWidth);
	}
	cout << endl;
	for(unsigned int i = 0; i < d.size(); ++i)
	{
		string str = d.at(i);
		if(str.at(str.size() - 1) == '/')
		{
			str = str.substr(0, str.size() - 1);
		}

		if(!str.compare(".") || !str.compare(".."))
		{
			continue;
		}

		string fullPath = path;
		fullPath.append(str);
		/*if(fullPath.at(fullPath.size() - 1) != '/')
		{
			fullPath.append("/");
		}*/

		struct stat s;
		if(-1 == stat(fullPath.c_str(), &s))
		{
			perror("Stat error");
			exit(1);
		}

		if(!(S_ISDIR(s.st_mode)))
		{
		}
		else
		{
			vector<string> dirEntries;
			DIR* dirp;
			if(NULL == (dirp = opendir(fullPath.c_str())))
			{
				perror("Error in opening directory");
				continue; // Continue to next parameter
			}

			cout << fullPath << ":" << endl;
			dirent* tempDirEnt;
			errno = 0;
			//unsigned int maxLength = 0;
			//unsigned int totalLength = 0;
			while(NULL != (tempDirEnt = readdir(dirp)))
			{
				// If points to hidden file and no -a
				if(sizeof(tempDirEnt->d_name) > 0 && tempDirEnt->d_name[0] == '.' && !aFlag)
				{
					continue;
				}
				string temp = tempDirEnt->d_name;
				
				dirEntries.push_back(tempDirEnt->d_name);

				//unsigned int tempSize = strlen(tempDirEnt->d_name); 
				//if(tempSize > maxLength)
				//{
					//maxLength = tempSize;
				//}
				//totalLength += tempSize;
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

			fullPath.append("/");
			printRecursive(dirEntries, fullPath, longForm, aFlag);
		}
	}
}

int main(int argc, char** argv)
{
	bool aFlag = false;
	bool lFlag = false;
	bool RFlag = false;	
	vector<string> fileParam;
	vector<string> files;

	unsigned int winWidth;
	winsize ws;

	if(-1 == (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)))
	{
		perror("Finding window width error");
		exit(1);
	}
	winWidth = ws.ws_col;

	bool printF = false;	
	bool nonexistent = false;
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
			struct stat s;
			if(-1 == stat(argv[i], &s))
			{
				perror("Stat error");
				nonexistent = true;
				continue;
			}
			if(!(S_ISDIR(s.st_mode)))
			{
				/*cout << argv[i]; 
				if(i != argc - 1)
				{
					cout << " ";
				}*/
				files.push_back(argv[i]);
				printF = true;
				continue;
			}
			fileParam.push_back(argv[i]); 
		}
	}
	/*if(printF)
	{
		cout << endl;
	}*/
	sort(files.begin(), files.end());
	
	if(files.size() > 0 )
	{
		if(!lFlag)
		{
			for(auto f : files)
			{
				cout << f << " ";
			}
			cout << endl;
			if(fileParam.size() > 0)
			{
				cout << endl;
			}
		}
		else
		{
			printLongForm(files, "");
			if(fileParam.size() > 0)
			{
				cout << endl;
			}
		}
	}
	if(fileParam.size() == 0 && !(printF || nonexistent))
	{
		fileParam.push_back("."); // Current directory, if no others specified
	}

	sort(fileParam.begin(), fileParam.end());
	
	for(unsigned int i = 0; i < fileParam.size(); ++i)
	{
		vector<string> dirEntries;
		DIR* dirp;
		if(NULL == (dirp = opendir(fileParam.at(i).c_str())))
		{
			perror("Error in opening directory");
			continue; // Continue to next parameter
		}

		if(fileParam.size() > 1 || printF || RFlag)
		{
			cout << fileParam.at(i) << ":" << endl;
		}
		dirent* tempDirEnt;
		errno = 0;
		unsigned int maxLength = 0;
		unsigned int totalLength = 0;
		while(NULL != (tempDirEnt = readdir(dirp)))
		{
			// If points to hidden file and no -a
			if(sizeof(tempDirEnt->d_name) > 0 && tempDirEnt->d_name[0] == '.' && !aFlag)
			{
				continue;
			}
			dirEntries.push_back(tempDirEnt->d_name);

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
			string inputPath = fileParam.at(i);
			if(inputPath.at(inputPath.size() - 1) != '/')
			{
				inputPath.append("/");
			}
			printRecursive(dirEntries, inputPath, lFlag, aFlag);
		}
		else
		{
			if(lFlag)
			{
				string path = fileParam.at(i);
				if(path.at(path.size() - 1) != '/')
				{
					path.append("/");
				}
				printLongForm(dirEntries, path);
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
