#include <iostream>
#include <cstring>
#include <cstdlib>
#include "server.h"

#define MAXPOOL 100
#define MAXQUEUE 100

using namespace std;


int main(int argc, char *argv[])
{
	int argcount = 1;
	int presentargs = 0;
	int exitflag = 0;
	unsigned int port = 0;
	unsigned int pool = 0;
	unsigned int queu = 0;

	char p[3] = { '-', 'p', '\0'};
	char s[3] = { '-', 's', '\0'};
	char q[3] = { '-', 'q', '\0'};
	char portstr[100] = { '\0' };
	char thrdstr[100] = { '\0' };
	char quesstr[100] = { '\0' };

	/*Diadikasia anagrwrisis odigiwn xristi*/
	if (argc < 7)
		exitflag = 1;
	else
		while(argcount < argc && (presentargs < 3))
		{
			if (strcmp(argv[argcount], p)==0)
			{
				argcount++;
				strcpy(portstr, argv[argcount]);
				presentargs++;
			}
			else if (strcmp(argv[argcount], s)==0)
			{
				argcount++;
				strcpy(thrdstr, argv[argcount]);
				presentargs++;
			}
			else if (strcmp(argv[argcount], q)==0)
			{
				argcount++;
				strcpy(quesstr, argv[argcount]);
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
	pool = atoi(thrdstr);
	queu = atoi(quesstr);
	if (!port || !pool || !queu || port > 65535 || pool > MAXPOOL || queu > MAXQUEUE)
		exitflag = 1;

	if (exitflag)
	{
		cout << "Den dwthike swsti entoli! Den mporei kapoio orisma na einai mideniko!" << endl;
		return 1;
	}

	server myserver(port, pool, queu, string(argv[0]));
	int ret =  myserver.run();

	return ret;

}
