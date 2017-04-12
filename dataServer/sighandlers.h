#ifndef SIGHANDLERS_H_
#define SIGHANDLERS_H_

#include <csignal>

using namespace std;

volatile static sig_atomic_t globalsigpipeflag;
volatile static sig_atomic_t globalsigtstpflag;

void handler_sigpipe(int);
void handler_sigtstp(int);

#endif /* SIGHANDLERS_H_ */
