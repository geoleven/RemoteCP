#include <iostream>
#include <cstring>
#include <cstdlib>
#include "client.h"

using namespace std;


int main(int argc, char *argv[])
{
	int argcount = 1;
	int presentargs = 0;
	int exitflag = 0;

	char i[3] = { '-', 'i', '\0'};
	char p[3] = { '-', 'p', '\0'};
	char d[3] = { '-', 'd', '\0'};
	char portstr[100] = { '\0' };
	char sripstr[100] = { '\0' };
	char direstr[100] = { '\0' };
	unsigned int port = 0;

	/*Diadikasia anagrwrisis odigiwn xristi*/
	if (argc < 7)
		exitflag = 1;
	else
		while(argcount < argc && !(presentargs == 3))
		{
			if (strcmp(argv[argcount], p)==0)
			{
				argcount++;
				strcpy(portstr, argv[argcount]);
				presentargs++;
			}
			else if (strcmp(argv[argcount], i)==0)
			{
				argcount++;
				strcpy(sripstr, argv[argcount]);
				presentargs++;
			}
			else if (strcmp(argv[argcount], d)==0)
			{
				argcount++;
				strcpy(direstr, argv[argcount]);
				presentargs++;
			}
			else
			{
				exitflag = 1;
				break;
			}
			argcount++;
		}

	port = atoi(portstr);
	if (exitflag || port <= 0 || port > 65535)
	{
		cout << "Den swthike swsti entoli!" << endl;
		return 1;
	}

	client myclient(sripstr, port, direstr, string(argv[0]));
	myclient.run();
	return 0;

}
