#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>

using namespace std;

class client {
	private:
		int port;
		string ip;
		string dir;
		unsigned long int fsbs;
	public:
		client(char*, int, char*, string);
		virtual ~client();
		int run();
		int makethedir(string);
		string parentstr(string);
		string execpath;
		string relative_to_absolute(string path);
		int readstr(int);
};

#endif /* CLIENT_H_ */
