#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "Timer.h"

using namespace std;

void method1(char* arg1, char* arg2)
{	
	fstream fin;
	fstream fout;
	fin.open(arg1);
	fout.open(arg2, fstream::out);
	if(!fin.is_open())
	{
		cerr << "Failed to open file1" << endl;
		exit(1);
	}
	if(!fout.is_open())
	{
		cerr << "Failed to open file2" << endl;
		exit(1);
	}
	char ch;
	while(fin.get(ch))
	{
		fout.put(ch);
	}
	fin.close();
	fout.close();
}

void method2(char* arg1, char* arg2)
{
	int fdnew;
	int fdold;
	if(-1 == (fdnew = open(arg2, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR))) 
	{
		perror("There was an error with open(). ");
			exit(1);
	}   
	if(-1 == (fdold = open(arg1, O_RDONLY))) 
	{
		perror("There was an error with open(). ");
		exit(1); 
	}

	int size;
	char c[1];
	if(-1 == (size = read(fdold, c, sizeof(c))))
	{
		perror("Read error");
		exit(1);
	}
	while(size > 0)
	{
		if(-1 == write(fdnew, c, size))
		{
			perror("Write error");
			exit(1);
		}
		if(-1 == (size = read(fdold, c, sizeof(c))))
		{
			perror("Read error");
			exit(1);
		}
	}
	if(-1 == close(fdnew))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
	if(-1 == close(fdold))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
}

void method3(char* arg1, char* arg2)
{
	int fdnew;
	int fdold;
	if(-1 == (fdnew = open(arg2, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR))) 
	{
		perror("There was an error with open(). ");
			exit(1);
	}   
	if(-1 == (fdold = open(arg1, O_RDONLY))) 
	{
		perror("There was an error with open(). ");
		exit(1); 
	}

	int size;
	char c[BUFSIZ];
	if(-1 == (size = read(fdold, c, sizeof(c))))
	{
		perror("Read error");
		exit(1);
	}
	while(size > 0)
	{
		if(-1 == write(fdnew, c, size))
		{
			perror("Write error");
			exit(1);
		}
		if(-1 == (size = read(fdold, c, sizeof(c))))
		{
			perror("Read error");
			exit(1);
		}
	}
	if(-1 == close(fdnew))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
	if(-1 == close(fdold))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
}

void method2OVER(char* arg1, char* arg2)
{
	int fdnew;
	int fdold;
	if(-1 == (fdnew = open(arg2, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR))) 
	{
		perror("There was an error with open(). ");
			exit(1);
	}   
	if(-1 == (fdold = open(arg1, O_RDONLY))) 
	{
		perror("There was an error with open(). ");
		exit(1); 
	}

	int size;
	char c[1];
	if(-1 == (size = read(fdold, c, sizeof(c))))
	{
		perror("Read error");
		exit(1);
	}
	while(size > 0)
	{
		if(-1 == write(fdnew, c, size))
		{
			perror("Write error");
			exit(1);
		}
		if(-1 == (size = read(fdold, c, sizeof(c))))
		{
			perror("Read error");
			exit(1);
		}
	}
	if(-1 == close(fdnew))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
	if(-1 == close(fdold))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
}

void method3OVER(char* arg1, char* arg2)
{
	int fdnew;
	int fdold;
	if(-1 == (fdnew = open(arg2, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR))) 
	{
		perror("There was an error with open(). ");
			exit(1);
	}   
	if(-1 == (fdold = open(arg1, O_RDONLY))) 
	{
		perror("There was an error with open(). ");
		exit(1); 
	}

	int size;
	char c[BUFSIZ];
	if(-1 == (size = read(fdold, c, sizeof(c))))
	{
		perror("Read error");
		exit(1);
	}
	while(size > 0)
	{
		if(-1 == write(fdnew, c, size))
		{
			perror("Write error");
			exit(1);
		}
		if(-1 == (size = read(fdold, c, sizeof(c))))
		{
			perror("Read error");
			exit(1);
		}
	}
	if(-1 == close(fdnew))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
	if(-1 == close(fdold))
	{
		perror("There was an error with close(). ");
		exit(1);
	}
}


int main(int argc, char** argv)
{
	if(!(argc == 4 || argc == 3))
	{
		cerr << "Invalid number of arguments" << endl;
		exit(1);
	}

	struct stat s;
	if(stat(argv[2], &s) != -1)
	{
		cerr << "File exists already" << endl;
		exit(1);
	}
	
	
	if(argc == 3)
	{
		method3(argv[1], argv[2]);
	}
	else
	{
		if(argv[3][0] != '-')
		{
			cerr << "Unrecognized argument" << endl;
			exit(1);
		}
		if(argv[3][1] != '3')
		{
			cerr << "Unrecognized argument" << endl;
			exit(1);
		}
		Timer t;
		double uTime;
		double wTime;
		double sTime;
		t.start();
		method1(argv[1], argv[2]);
		t.elapsedUserTime(uTime);
		t.elapsedWallclockTime(wTime);
		t.elapsedSystemTime(sTime);
		cout << "Method 1: " << endl;
		cout << "Wall: " << wTime << endl;
		cout << "User: " << uTime << endl;
		cout << "System: " << sTime << endl;

		t.start();
		method2OVER(argv[1], argv[2]);
		t.elapsedUserTime(uTime);
		t.elapsedWallclockTime(wTime);
		t.elapsedSystemTime(sTime);
		cout << "Method 2: " << endl;
		cout << "Wall: " << wTime << endl;
		cout << "User: " << uTime << endl;
		cout << "System: " << sTime << endl;

		t.start();
		method3OVER(argv[1], argv[2]);
		t.elapsedUserTime(uTime);
		t.elapsedWallclockTime(wTime);
		t.elapsedSystemTime(sTime);
		cout << "Method 3: " << endl;
		cout << "Wall: " << wTime << endl;
		cout << "User: " << uTime << endl;
		cout << "System: " << sTime << endl;
	}
	
/*
	Timer t;
	double eTime;
	t.start();

	t.elapsedUserTime(eTime);
	cout << eTime << endl; */
}
