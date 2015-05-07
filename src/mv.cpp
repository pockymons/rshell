#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>

using namespace std;

int main(int argc, char** argv)
{
	if(argc != 3)
	{
		cerr << "Needs to be 3 arguments" << endl;
		exit(1);
	}

	struct stat s;
	if(-1 == stat(argv[1], &s))
	{
		perror("Error 1");
		exit(1);
	}

	errno = 0;
	link(argv[1], argv[2]);
	if(errno != 0 && errno != EEXIST)
	{
		perror("Error 2");
		exit(1);
	}
	if(errno == 0)
	{
		if(-1 == unlink(argv[1]))
		{
			perror("Error 3");
			exit(1);
		}
		exit(0);
	}

	if(-1 == stat(argv[2], &s))
	{
		perror("Stat error");
		exit(1);
	}

	if(S_ISDIR(s.st_mode))
	{
		char* path = strcat(argv[2], "/");
		path = strcat(path, argv[1]);
		if(-1 == link(argv[1], path))
		{
			perror("Error 4");
			exit(1);
		}
		if(-1 == unlink(argv[1]))
		{
			perror("Error 5");
			exit(1);
		}
	}
	else
	{
		cerr << "Argument 2 is not a directory" << endl;
		exit(1);	
	}

	return 0;
}
