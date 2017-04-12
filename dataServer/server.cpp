#include "server.h"
#include <iostream>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <csignal>
#include "sighandlers.h"
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <cstring>
#include <dirent.h>
#include <cstdlib>
#include "lista.h"
#include <netdb.h>

#define CONQUEUE 32
#define PERMISS 0666
#define MAXCL 256
#define VERBOSE 1 /*More info on stdout*/
typedef struct thread_info thread_info;

using namespace std;

/*Thread and its socket struct*/
struct thread_info {
	pthread_t	thread_id;
	int			thread_num;		   /*Number or socket for the thread*/
};


/*Safe write function*/
int write_all (int myfd, void *mybuf, size_t mysize) {
	int mysent = 0;
	int myn = 0;
	for (mysent = 0; (unsigned) mysent < mysize; mysent += myn) {
		if ((myn = write (myfd, (char*)mybuf + mysent, mysize - mysent)) == -1)
			return -1;
	}
	return mysent;
}

/*Function that writes a directory, file or ending on the given socket*/
int passfileonsocket(int myfd, string mydir, int mytype) {

	unsigned long int fsbs;
	struct statvfs stmp;
	statvfs("/", &stmp);
	fsbs = stmp.f_bsize;
	struct stat info;
	string stype = "";
	char* buf = new char[fsbs]; /*or info.st_blksize*/
	memset (buf, 0, fsbs);
	if (mytype != 3) {
		if (lstat(mydir.c_str(), &info) == -1) {
			perror(" Failed to get original file status ");
			delete buf;
			return -1;
		}
	}
	if (mytype == 1)
	{
		stype = "dire\n" + mydir + "\n!@#$%^&!";
		if(write_all(myfd, (void*) stype.c_str(), stype.length()) != (int)stype.length())
			perror("Directory info was not written on socket correctly!");
		if (VERBOSE)
			cout << "\nI just sent the directory " << mydir << " so the client will create it!" << endl;
	}
	else if (mytype == 0)
	{
		int filefd = 0;
		int readc = 0;

		stype = "file\n" + mydir + "\n!@#$%^&!";
		if(write_all(myfd, (void*) stype.c_str(), stype.length()) != (int)stype.length())
			perror("File info was not written on socket correctly!");

		if ((filefd = open(mydir.c_str(), O_RDONLY)) == -1) {
			perror("File for copy could not be opened!");
			return -1;
		}
		if (VERBOSE)
			cout << "\nSendning file " << mydir << " on socket " << myfd << "." << endl;
		while ( (readc = read(filefd, buf, fsbs)) > 0) {
			if ( (write_all(myfd, buf, readc)) != readc)
				perror("I did not sent what i just read!");
			readc = 0;
			memset (buf, 0, fsbs);
		}
		if (VERBOSE)
			cout << "File " << mydir << " sent on socket " << myfd << ", sending end." << endl;
		memset(buf, 0 , fsbs);
		strcpy(buf, "!@#$%^&!");
		if ( (write_all(myfd, buf, 8)) != 8)
			perror("I did not sent ending correctly!");
		if (VERBOSE)
			cout << "File end for file " << mydir << " sent on socket " << myfd << "." << endl;
		close(filefd);
	}
	else if (mytype == 3)
	{
		if (VERBOSE)
			cout << "Sending end for the connection on socket: " << myfd << endl;
		stype = "end \n !@#$%^&!";
		write_all(myfd, (void*) stype.c_str(), stype.length());
		sleep(2);
	}
	else
	{
		cout << "Something didn't work on queue!" << endl;
	}

	/*{//Confirmation block
		string incoming = "";
		memset (buf, 0, fsbs);
		int err = 0;
		while (incoming.find("done!@#$%^&!") == (string::npos) && !err){
			if(read(myfd, buf, 2048) <= 0) {
				perror("Read error while waiting for client confirmation!");
				err = 1;
			}
			incoming.append(buf);
			memset(buf, 0, fsbs);
		}
		if (VERBOSE && !err)
			cout << "Job confirmed from client!" << endl;
	}*/

	delete buf;
	return 1;
}

/*Function of workers, run until exitflag terminates it*/
static void * worker_thread_run(void *arg)
{
	server *srv = (server*)arg;
	pthread_mutex_lock(&(srv->mutex));
	while(!(srv->exitflag)) {
		if (srv->filelist->empty()) {
			/*Ean to queue den exei tipota koimatai gia 1sec afou kanei unlock kai ksanakoitaei*/
			pthread_mutex_unlock(&(srv->mutex));
			sleep(1);
		}
		else
		{
			int onwork = 0;
			for (int i = 0; i < srv->pool; i++) {
				if (srv->filelist->first->clientsocketfd == srv->clientsonwork[i]) {
					onwork = 1;
					break;
				}
			}
			if (onwork) {
				pthread_mutex_unlock(&(srv->mutex));
				sleep(1);
			}
			else
			{
				string	temps = srv->filelist->first->file;
				int		tempi = srv->filelist->first->clientsocketfd;
				int		tempt = srv->filelist->first->isdir;
				if (!(srv->filelist->pop()))
					cout << "Something went wrong while poping from queue!" << endl;
				for (int i = 0; i < srv->pool; i++) {
					if (srv->clientsonwork[i] == 0) {
						srv->clientsonwork[i] = tempi;
						break;
					}
				}
				pthread_mutex_unlock(&(srv->mutex));
				if(!(passfileonsocket(tempi, temps, tempt)))
					cout << "Something went worng while passing file on socket" << endl;
				pthread_mutex_lock(&(srv->mutex));
				for (int i = 0; i < srv->pool; i++) {
					if (srv->clientsonwork[i] == tempi) {
						srv->clientsonwork[i] = 0;
						break;
					}
				}
				pthread_mutex_unlock(&(srv->mutex));
			}
		}
		pthread_mutex_lock(&(srv->mutex));
	}
	srv->workersrunning--;
	pthread_mutex_unlock(&(srv->mutex));
	pthread_exit(NULL);
	return (void *)NULL;
}

/*Sunartisi pou pernaei stin oura to entry*/
int cplfile(int mysocket, string mydir, server* srv, int type) {

	pthread_mutex_lock(&(srv->mutex));
	while(srv->filelist->full()) {
		pthread_mutex_unlock(&(srv->mutex));
		sleep(1);
		pthread_mutex_lock(&(srv->mutex));
	}
		srv->filelist->addtoend();
		srv->filelist->last->clientsocketfd = mysocket;
		srv->filelist->last->file = mydir;
		srv->filelist->last->isdir = type;
	pthread_mutex_unlock(&(srv->mutex));
	return 1;
}

/*Sunartisi pou analamvanei na valei anadromika olo to directory sto queue*/
int cpdir(int mysocket, string mydir, server* srv) {
	struct stat inbuf;
	if (lstat(mydir.c_str(), &inbuf) == -1) {
		cout << mydir << endl;
		perror(" Failed to get original file status ");
		return 0;
	}

	if (S_ISDIR(inbuf.st_mode)) {
		DIR* dirp;
		struct dirent* dp;
		if ((dirp = (opendir(mydir.c_str()))) == NULL)
		{
			perror("Den mporesa na anoikso kapoio directory");
			return 1;
		}
		rewinddir(dirp);
		cplfile(mysocket, mydir, srv, 1);
		while( (dp = (readdir(dirp))) != NULL )
		{
			if((strcmp(dp->d_name,".") != 0) && (strcmp(dp->d_name,"..") !=0))
			{
				string tempor = mydir;
				tempor.append("/");
				tempor.append(dp->d_name);
				cpdir(mysocket, tempor, srv);
			}
		}
		closedir(dirp);
	}
	else if (S_ISREG(inbuf.st_mode)) {
		cplfile(mysocket, mydir, srv, 0);
	}
	else {
		cout << "Weird file!!" << endl;
	}

	return 1;
}

/*Sunartisi twn threads pou trexoun gia kathe ksexwristo pelati. Analamvanei na dei ti zitaei o pelatis kai na antapokrithei*/
static void * client_thread_run(void *arg) {
	server *srv = (server*)arg;
	int myfd = 0;
	struct stat inbuf;
	char buf[2048];
	memset(buf, 0, 1024);
	string dir = "";

	int i;
	for(i = 0; i < MAXCL; i++)
	{
		if (pthread_equal(pthread_self(), srv->thrdinfptr[i].thread_id)) {
			myfd = srv->thrdinfptr[i].thread_num;
			break;
		}
	}

	if (VERBOSE)
		cout << "I am a client service thread and my socket's fd is: " << myfd << endl;

	if (read(myfd, buf, 2048) <= 0) {
		perror("Could not get path from client!\n");
		pthread_exit(NULL);
		return (void*)NULL;
	}

	dir.append(buf);
	if (dir == "endnow")
	{
		pthread_mutex_lock(&(srv->mutex));
		srv->exitmsg = 1;
		srv->exitflag = 1; /*An theloume na min teleiwsoun tin douleia tous oi workers*/
		pthread_mutex_unlock(&(srv->mutex));
	}
	else {
		if (lstat(dir.c_str(), &inbuf) == -1) {
			perror(" Failed to get original file status ");
		}
		else {
			if (S_ISREG(inbuf.st_mode)) {
				cplfile(myfd, dir, srv, 0);
			}
			else if(S_ISDIR(inbuf.st_mode)) {
				cpdir(myfd, dir, srv);
			}
			else {
				cout << "Weird file encountered!" << endl;
			}
		}
		cplfile(myfd, "end", srv, 3);
	}

	pthread_mutex_lock(&(srv->mutex));
	while(srv->tobecleaned[(srv->tobecleanedcounter)])  /*Hmm !?! modulo?*/
		(srv->tobecleanedcounter)++;
	srv->tobecleaned[(srv->tobecleanedcounter)] = srv->thrdinfptr[i].thread_id;
	srv->thrdinfptr[i].thread_num = 0;
	srv->thrdinfptr[i].thread_id = 0;
	int j = 0;
	while(srv->fdstobeclosed[j]) {
		j++;
	}
	sleep(5);
	srv->fdstobeclosed[j] = myfd;
	pthread_mutex_unlock(&(srv->mutex));

	pthread_exit(NULL);
	return (void*)NULL;
}


server::server(int myport, int mypool, int myqueue, string mypath) {
	port = myport;
	pool = mypool;
	queue = myqueue;
	welcomes = 0;
	filelist = new lista(queue);
	exitflag = 0;
	workersrunning = 0;
	clientsonwork = new int[pool];
	for (int i = 0; i < pool; i++)
		clientsonwork[i] = 0;
	pthread_mutex_init(&mutex, NULL);
	clientscomplete = 0;
	thrdinfptr = NULL;
	execpath = mypath;
    tobecleaned = new pthread_t[2*MAXCL];
    for (int i = 0; i < MAXCL *2; i++)
    	tobecleaned[i] = 0;
    tobecleanedcounter = 0;
    exitmsg = 0;
    fdstobeclosed = new int[MAXCL];
}

server::~server() {

	pthread_mutex_destroy(&mutex);
	close(welcomes);
	delete tobecleaned;
	delete filelist;
	delete clientsonwork;
	delete fdstobeclosed;
	cout << "Server is closing normally!" << endl;
}

/*Kuriws sunartisi pou trexei ton server*/
int server::run() {
	globalsigpipeflag = 0;
	globalsigtstpflag = 0;
	struct sockaddr_in serverscad;
	struct sockaddr_in newclientad;
	socklen_t newclientlen;
	struct hostent *rem;
	int optionvalueret = 1;
	int newfd;
    thread_info *thread_pool;
    thread_info *thread_clients;
    pthread_attr_t attr;
    int clientcounter = 0;
	signal(SIGPIPE, handler_sigpipe);
	signal(SIGTSTP, handler_sigtstp);


	/*____________________________________________Create pool of workers________________________________________________________*/


    if ((thread_pool = (thread_info*)calloc(pool, sizeof(thread_info))) == NULL)
    	perror("Calloc thread info array error! ");

    if (pthread_attr_init(&attr) != 0)
    	perror("Thread attribute initialization not completed");

	for (int i = 0; i < pool; i++)
	{
		thread_pool[i].thread_num = i;
		pthread_mutex_lock(&mutex);
		workersrunning++;
		pthread_mutex_unlock(&mutex);
		if(pthread_create(&(thread_pool[i].thread_id), &attr, &worker_thread_run, (void*)this))
		{
			perror("Some worker thread was not created properly! ");
			pthread_mutex_lock(&mutex);
			workersrunning--;
			pthread_mutex_unlock(&mutex);
		}
	}

	/*_______________________________________________Server Socket Creation_____________________________________________________*/

	if ((welcomes = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		perror("Could not create socket: ");

	if (setsockopt(welcomes, SOL_SOCKET, SO_REUSEADDR, &optionvalueret, sizeof(int)) == -1)
		perror("Could not make the socket reusable: ");

	serverscad.sin_family = AF_INET;
	serverscad.sin_addr.s_addr = htonl(INADDR_ANY);
	serverscad.sin_port = htons(/*(short)*/port);

	if (bind(welcomes, (struct sockaddr*)&serverscad, sizeof(serverscad)) == -1)
		perror("Could not bind welcome socket to port: ");

	if (listen(welcomes, CONQUEUE) == -1)
		perror("Listen error: ");


    if ((thread_clients = (thread_info*)calloc(MAXCL, sizeof(thread_info))) == NULL)
    	perror("Calloc thread info array error! ");
    thrdinfptr = thread_clients;

    cout << "Server initialized well, server's parameters are:" << endl;
    cout << "       Port:              " << port << endl;
    cout << "       Pool size:         " << pool << endl;
    cout << "       Queue size:        " << queue << endl;
    cout << "       Listening socket:  " << welcomes << endl;

    /*_____________________________________________Server General Function______________________________________________________*/
	while(!globalsigtstpflag && !exitmsg) {
		while(MAXCL <= (clientcounter-clientscomplete))
			sleep(1);
		newclientlen = sizeof(newclientad);
		if ((newfd = accept(welcomes, (struct sockaddr*)&newclientad, &newclientlen)) <= 0)
			perror("Error on accepting connection!");
		if(!globalsigtstpflag) {
			clientcounter++;
			while(thread_clients[clientcounter%MAXCL].thread_num !=0)
				sleep(1);
			thread_clients[clientcounter%MAXCL].thread_num = newfd;
			if ((rem = gethostbyaddr((char*)&newclientad.sin_addr.s_addr,sizeof(newclientad.sin_addr.s_addr), newclientad.sin_family)) == NULL)
			{
				perror("Could not get host by address");
				clientcounter--;
			}
			else if(pthread_create(&(thread_clients[clientcounter%MAXCL].thread_id), &attr, &client_thread_run, (void*)this))
			{
				perror("Some client thread was not created properly! ");
				clientcounter--;
			}
		}
		cout << "Opened socket " << newfd << " for client!" << endl;
		//close(newfd);
		newfd = 0;

		thread_cleaner();
		/*____________________________________________Small waiting function____________________________________________________*/
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 300000;
		select(0, NULL, NULL, NULL, &tv);
		//sleep(1);
		/*______________________________________________________________________________________________________________________*/

	}

	if (VERBOSE)
		cout << "Exited server's ongoing while!" << endl;
	/*_______________________________________________Clean Up___________________________________________________________________*/
	double *nothing = new double;
	exitflag = 1;
	//sleep(2);
	int flaga = 1;
	int flagb = 0;
	while(flaga) {
		for (int i = 0; i < MAXCL; i++) {
			if (thread_clients[i].thread_id)
				flagb = 1;
		}
		if (!flagb)
			flaga = 0;
		else
			flagb = 0;
	}
	for (int i = 0; i < pool; i++)
	{
		pthread_join(thread_pool[i].thread_id, (void**)&nothing);
	}
	thread_cleaner();
	cout << "Threads were cleaned normally!" << endl;
	delete nothing;
	free(thread_pool);
	return 0;
}

/*Function to wait and gather used threads*/
int server::thread_cleaner() {

	int found = 0;
	void* nothing = new double;
	for (int i = 0; i < 2*MAXCL; i++)
	{
		pthread_mutex_lock(&mutex);
		if (tobecleaned[i]) {
			pthread_join(tobecleaned[i], &nothing);
			tobecleaned[i] = 0;
			found = 1;
		}
		if (fdstobeclosed[i%MAXCL]) {
			close(fdstobeclosed[i%MAXCL]);
			fdstobeclosed[i%MAXCL] = 0;
		}
		pthread_mutex_unlock(&mutex);
	}
	delete (double*)nothing;
	return found;
}

/*Function to wait and close used fds*/
int server::fds_cleaner() {
	for (int i = 0; i < MAXCL; i++)
	{
		if (fdstobeclosed[i]) {
			if(filelist->search_fd(fdstobeclosed[i]) == NULL) {
				close(fdstobeclosed[i]);
				fdstobeclosed[i] = 0;
			}
		}
	}
	return 1;
}
