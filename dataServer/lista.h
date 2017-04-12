#ifndef LISTA_H_
#define LISTA_H_

#include <string>

using namespace std;

class nodel {
	private:

	public:
		nodel(nodel*, nodel*);
		virtual ~nodel();
		nodel* next;
		nodel* prev;
		string file;
		int clientsocketfd;
		int isdir;
};


class lista {
	private:

	public:
		lista(int);
		virtual ~lista();
		int counter;
		nodel* first;
		nodel* last;
		int max;
		int empty();
		int addtoend();
		int pop();			/*Returns 0 if nothing to pop*/
		int full();
		int destroy();
		nodel* search(string, int);
		int delete_node(string, int);
		nodel* search_fd(int);
};


#endif /* LISTA_H_ */
