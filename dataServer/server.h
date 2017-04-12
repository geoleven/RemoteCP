#ifndef SERVER_H_
#define SERVER_H_

#include "lista.h"
#include <string>

using namespace std;

/*class worker {
	private:

	public:
		worker();
		virtual ~worker();
		void run();
};*/

struct thread_info;

class server {
	private:

	public:
		server(int, int, int, string);
		virtual ~server();
		int run();
		int exitflag;
		int port;
		int pool;
		int queue;
		//size_t fsbs;
		//unsigned long int fsbs;
		int welcomes;
		lista *filelist;
		int workersrunning;
		pthread_mutex_t mutex;
		int *clientsonwork;
		int clientscomplete;
		//void *thrdinfptr;
		struct thread_info *thrdinfptr;
		string execpath;
		int thread_cleaner();
		pthread_t *tobecleaned;
		int tobecleanedcounter;
		int exitmsg;
		int *fdstobeclosed;
		int fds_cleaner();
};

#endif /* SERVER_H_ */
