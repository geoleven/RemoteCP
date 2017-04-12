#include "sighandlers.h"
#include <csignal>


void handler_sigpipe(int mysigno)
{
	globalsigpipeflag = 1;
	signal(SIGPIPE, handler_sigpipe);
}

void handler_sigtstp(int mysigno)
{
	globalsigtstpflag = 1;
	signal(SIGPIPE, handler_sigtstp);
}
