#include "client.h"
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>

#define VERBOSE 1

using namespace std;

int write_all (int myfd, const void *mybuf, size_t mysize) {
	int mysent = 0;
	int myn = 0;
	for (mysent = 0; (unsigned) mysent < mysize; mysent += myn) {
		if ((myn = write (myfd, (char*)mybuf + mysent, mysize - mysent)) == -1)
			return -1;
	}
	return mysent;
}


client::client(char* myip, int myport, char* mydir, string myexecpath) {
	ip = string(myip);
	port = myport;
	dir = string(mydir);
	execpath = myexecpath;
	struct statvfs stmp;
	statvfs("/", &stmp);
	fsbs = stmp.f_bsize;
}

client::~client() {}

/*Main client function*/
int client::run() {
	int mysocket;
	struct sockaddr_in server;
	struct sockaddr *serverptr = (struct sockaddr*)&server;
	struct hostent *rem;

	if (dir[dir.length()-1] == '/') {
		dir.erase((dir.length() - 1), string::npos);
	}

	if (dir != "endnow") {
		if (dir[0] != '/' && dir[0] != '.')
			dir.insert(0, "./");

		relative_to_absolute(dir);

		makethedir(dir);
	}

	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Could not create socket!");
	}

	if ((rem = gethostbyname(ip.c_str())) == NULL) {
		perror("Could not get host by name!");
	}

	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	server.sin_port = htons(port);

	if (connect(mysocket, serverptr, sizeof(server)) < 0)
		perror("Could not connect to server socket!");

	if ((write_all(mysocket, (void*)dir.c_str(), (dir.size() + (unsigned)1))) < 0)
		perror("Error while writing on socket!");

	int ret = 0;
	if (dir != "endnow") {
		while((ret = readstr(mysocket)) == 1);
		if (ret == 0) {
			cout << "Something wicked happened!" << endl;
			return 1;
		}
		if (ret == 2) {
			cout << "Took the whole file list!" << endl;
		}
	}

	close(mysocket);
	return 0;
}

/*Function to create the path given so the received files can be saved*/
int client::makethedir(string mydir) {

	struct stat otbuf;

	if(lstat(mydir.c_str(), &otbuf) == -1) {
		if(makethedir(parentstr(mydir))) {
			if(mkdir(mydir.c_str(), 0777)) {
				perror("Could not create directory! ");
			}
			else {
				return 1;
			}
		}
		else {
			return 0;
		}
	}
	else {
			return 1;
	}
	return 0;
}

/*Function to make parent path string of given path string*/
string client::parentstr(string child)
{
	if (child.length() <= 2)
		return "/";
	return child.substr(0, child.find_last_of("/", string::npos));
}

/*Sunartisi pou metatrepsei sxetika monopatia se apoluta (px: ./ kai /home/george/..*/
string client::relative_to_absolute(string path)
{
	if (path[0] != '/')
	{
		if (path[0] == '.' && path[1] == '/')
		{
			path = execpath + path.erase(0,1);
		}
		else if (path[0] == '.' && path[1] == '.' && path[2] == '/')
		{
			path = parentstr(execpath) + path.erase(0,2);
		}
		else
		{
			path = execpath + path;
			path = relative_to_absolute(path);
		}
	}
	else
	{
		if (((path[1] == '.') && (path[2] == '/') )|| ((path[1] == '.') && (path[2] == '.') && path[3] == '/') )
		{
			path.erase(0,1);
			path = path.substr(path.find_first_of('/'), string::npos);
			path = relative_to_absolute(path);
		}
	}

	for(unsigned int i = 0; i < (path.length() - 1); i++)
	{
		if (path[i] == '.' && path[i+1] == '.' && ((i + 2 == path.length()) || (path[i+2] == '/')))
		{
			if(i + 2 == path.length())
			{
				path = parentstr(path);
				i = 0;
			}
			else
			{
				string temp = path;
				temp.erase(0,(i+2));
				path.erase((i-1),string::npos);
				path = parentstr(path);
				path = path + temp;
				i = 0;
			}
		}
	}


	return path;
}

/*Function that reads the files the server sends, saves them and ends the client*/
int client::readstr(int mysocket) {

	int exflag = 0;
	char *buffer = new char[fsbs];
	string incoming = "";
	memset (buffer, 0, fsbs);
	while (incoming.find("!@#$%^&!") == (string::npos)) {
		cout << "Going to read command on socket!" << endl;
		if (read(mysocket, buffer, 1/*fsbs*/) < 1)
			perror("Something did not went well with reading from socket!");
		incoming.append(buffer);
		cout << "Just read " << incoming << " on socket." << endl;
		memset (buffer, 0, fsbs);
	}
	if (VERBOSE)
		cout << "\nI got the command: " << incoming << endl;
	incoming.erase(incoming.find_last_of("\n"), string::npos);
	if (VERBOSE)
		cout << "\nConverted command/dir: " << incoming << " .(end of command)" << endl;
	if (incoming.find("dire") != (string::npos)) {
		incoming.erase((size_t)0, (size_t)5);
		if (VERBOSE)
			cout << "\nMaking directory " <<  incoming << " ." << endl;
		mkdir(incoming.c_str(), 0777);
	}
	else if (incoming.find("file") != (string::npos)) {
		incoming.erase((size_t)0, (size_t)5);
		int newfile = 0;
		if ( (newfile = creat(incoming.c_str(), 0777)) == -1) {
			perror("Could not create some file!");
			return 0;
		}
		if (VERBOSE) {
			cout << "Created file " << incoming << endl;
			cout << "Now initializing file transfer process!" << endl;
		}
		memset (buffer, 0, fsbs);
		int readc = 0;
		//int readp = 0;
		char *dbuf = new char[fsbs*2];
		memset (dbuf, 0, fsbs*2);
		char *pbuf = new char[fsbs];
		memset (pbuf, 0, fsbs);
		//char* ends = "!@#$%^&!";
		/*while (strstr(dbuf, "!@#$%^&!") == NULL) {
			cout << "parsing" << endl;
			readc = 0;
			memset (buffer, 0 , fsbs);
			memset (dbuf, 0, fsbs*2);
			if ((readc = read(mysocket, buffer, fsbs)) <1) {
				perror("Something did not went well with reading from socket!");
			}
			memcpy(dbuf, pbuf, fsbs);
			memcpy(dbuf+fsbs, buffer, fsbs);
			if (strstr(dbuf, "!@#$%^&!") != NULL) {
				if (write_all(newfile, dbuf, strlen(dbuf) - 8) == -1)
					perror("Could not write received data to file!");
				cout << "Last data on file were: " << dbuf << endl;
			}
			else {
				if (write_all(newfile, pbuf, readp) == -1)
					perror("Could not write received data to file!");
			}
			memcpy(pbuf, buffer, fsbs);
			readp = readc;
		}*/
		int en = 0;
		while(!en) {
			readc = 0;
			memset(buffer, 0, 1);
			if ((readc = read(mysocket, buffer, 1)) <1)
			{
				perror("Something did not went well with reading from socket!");
			}

			if (buffer[0] == '!')
			{
				if ((readc = read(mysocket, buffer, 1)) <1)
				{
					perror("Something did not went well with reading from socket!");
				}
				if (buffer[0] == '@')
				{
					if ((readc = read(mysocket, buffer, 1)) <1)
					{
							perror("Something did not went well with reading from socket!");
						}
						if (buffer[0] == '#')
						{
							if ((readc = read(mysocket, buffer, 1)) <1)
							{
								perror("Something did not went well with reading from socket!");
							}
							if (buffer[0] == '$')
							{
								if ((readc = read(mysocket, buffer, 1)) <1)
								{
									perror("Something did not went well with reading from socket!");
								}
								if (buffer[0] == '%')
								{
									if ((readc = read(mysocket, buffer, 1)) <1)
									{
										perror("Something did not went well with reading from socket!");
									}
									if (buffer[0] == '^')
									{
										if ((readc = read(mysocket, buffer, 1)) <1)
										{
											perror("Something did not went well with reading from socket!");
										}
										if (buffer[0] == '&')
										{
											if ((readc = read(mysocket, buffer, 1)) <1)
											{
												perror("Something did not went well with reading from socket!");
											}
											if (buffer[0] == '!')
											{
												en = 1;
												continue;
											}
											else
											{
												if (write_all(newfile, "!@#$%^&", 7) == -1)
												perror("Could not write received data to file!");
											}
										}
										else
										{
											if (write_all(newfile, "!@#$%^", 6) == -1)
												perror("Could not write received data to file!");
										}
									}
									else
									{
										if (write_all(newfile, "!@#$%", 5) == -1)
											perror("Could not write received data to file!");
									}
								}
								else
								{
									if (write_all(newfile, "!@#$", 4) == -1)
										perror("Could not write received data to file!");
								}
							}
							else
							{
								if (write_all(newfile, "!@#", 3) == -1)
									perror("Could not write received data to file!");
							}
						}
						else
						{
							if (write_all(newfile, "!@", 2) == -1)
								perror("Could not write received data to file!");
						}
					}
					else {
						if (write_all(newfile, "!", 1) == -1)
							perror("Could not write received data to file!");
					}
				}

			if (write_all(newfile, buffer, 1) == -1)
				perror("Could not write received data to file!");

		}
		if (VERBOSE)
			cout << "File " << incoming << " received." << endl;
		delete dbuf;
		delete pbuf;
		close(newfile);
	}
	else if (incoming.find("end") != (string::npos)) {
		exflag = 1;
	}
	else {
		cout << "Something weird was sent!!!" << endl;
		return 0;
	}

	/*{//Confirmation block
		if ((write_all(mysocket, (void*)"done!@#$%^&!", 12)) < 0)
			perror("Error while writing on socket!");
	}*/

	if (exflag)
		return 2;
	else
		return 1;
}
