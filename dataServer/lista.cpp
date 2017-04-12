#include "lista.h"
#include <iostream>

using namespace std;

nodel::nodel(nodel* myprev, nodel* mynext) {
	next = mynext;
	prev = myprev;
	file = "";
	clientsocketfd = 0;
	isdir = 0;

}

nodel::~nodel() {


}

lista::lista(int mymax) {
	first = NULL;
	last = NULL;
	counter = 0;
	max = mymax;

}

lista::~lista() {
	if (!destroy())
		cout << "H lista den diagrafike kanonika!" << endl;
}

int lista::empty(){
	if (first == NULL && last == NULL)
		return 1;
	return 0;
}

/*Prosthetei last node stin oura*/
int lista::addtoend()
{
	if (empty())
	{
		first = new nodel(NULL, NULL);
		last = first;
	}
	else
	{
		last->prev = new nodel(NULL, last);
		//last->prev->next = last;
		last = last->prev;
	}
	counter++;
	return 0;
}

/*Delete first node of queue*/
int lista::pop()
{
	if (empty())
		return 0;
	else {
		if (first == last) {
			delete first;
			first = NULL;
			last = NULL;
		}
		else {
			first = first->prev;
			delete first->next;
			first->next = NULL;
		}
		counter--;
	}
	return 1;
}

/*Chack if list is full according to set max value*/
int lista::full() {
	if (counter >= max)
		return 1;
	return 0;
}

/*Pop queue till empty*/
int lista::destroy() {
	if (empty())
		return 1;
	else
		while (!empty())
			pop();
	if (empty())
		return 1;
	return 0;
}

/*Search for a specific file to be sent on a given myfd*/
nodel* lista::search(string myfile, int myfd)
{
	if (empty())
		return NULL;

	nodel* current = first;
	while (current != NULL && !(current->file == myfile && current->clientsocketfd == myfd))
		current = current->prev;

	return current;
}

/*For completion purposes (not userd) function to delete a specific node*/
int lista::delete_node(string myfile, int myfd)
{
	nodel* temp = search(myfile, myfd);
	if (temp == NULL)
		return 0;
	if (first == last)
	{
		first = NULL;
		last = NULL;
	}
	else if (temp == first)
	{
		first = first->prev;
		first->next = NULL;
	}
	else if (temp == last)
	{
		last = last->next;
		last->prev = NULL;
	}
	else
	{
		temp->prev->next = temp->next;
		temp->next->prev = temp->prev;
	}
	delete temp;
	counter--;
	return 1;
}

/*Search if an fd in the list, if found returns node*/
nodel* lista::search_fd(int myfd) {
	if (empty())
		return NULL;

	nodel* current = first;
	while (current != NULL && !(current->clientsocketfd == myfd))
		current = current->prev;

	return current;
}
