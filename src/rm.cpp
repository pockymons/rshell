#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

bool r_FLAG = false;

void rmFile(char* filename)
{
	if(-1 == unlink(filename))
	{
		perror("Could not delete");		
	}
}

void recursiveRM(char* filename)
{
	struct stat b;
	if(-1 == stat(filename, &b))
	{
		cout << filename << endl;
		perror("Stat");
		return;
	}

	if(S_ISDIR(b.st_mode))
	{
		DIR* dirp;

		if(NULL == (dirp = opendir(filename)))
		{
			perror("Open");
			return;
		}
		struct dirent* dir;
		errno = 0;

		while(NULL != (dir = readdir(dirp)))
		{
			string path = filename;

			if(strcmp(dir->d_name, ".") == 0 
				|| strcmp(dir->d_name, "..") == 0)
			{
				continue;
			}

			struct stat b1;


			path += dir->d_name;
			if(-1 ==  stat(const_cast<char*> (path.c_str()), &b1))
			{
				perror("Stat 1");
				continue;
			}

			if(S_ISDIR(b1.st_mode))
			{
				path += '/';
				recursiveRM(const_cast<char*> (path.c_str()));
				if(-1 == rmdir(const_cast<char*> (path.c_str())))
				{
					perror("Remove directory");
					exit(1);
				}
			}
			else
			{
				rmFile(const_cast<char*> (path.c_str()));
			}
		}
		if(errno != 0)
		{
			perror("Read");
			exit(1);
		}

		if(-1 == closedir(dirp))
		{
			perror("Close");
			exit(1);
		}	
	}
	else
	{
		rmFile(filename);
	}
}

int main(int argc, char** argv)
{
	int ch = 0;
	opterr = 0;

	while(( ch = getopt(argc, argv, "r")) != -1)
	{
		switch(ch)
		{
			case 'r':
				r_FLAG = true;
				break;

			case '?':
				cerr << "Flag not supported" << endl;
				break;
		}
	}

	if(r_FLAG)
	{
		for(int i = 1; i < argc; ++i)
		{
			if(argv[i][0] == '-')
			{
				continue;
			}
			string path = argv[i];
			path += '/';
			recursiveRM(const_cast<char*>(path.c_str()));
			if(-1 == rmdir(const_cast<char*>(path.c_str())))
			{
				perror("Remove directory");
				exit(1);
			}
		}
	}
	else
	{
		for(int i = 1; i < argc; ++i)
		{
			rmFile(argv[i]);
		}
	}
	return 0;
}
