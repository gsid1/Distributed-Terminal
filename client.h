#include<bits/stdc++.h>
#include <cstdlib>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
// #include <string.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h> 
#define BUFFER 50000
#define FILESIZE 1000
#define HOSTLIMIT 60
#define HOSTNAME 1000 
#define LOG 1
#define PORT 1238

using namespace std ;

// struct to add friend 
struct hostList
{
	char hostname[60];
	char ip[15];
	int port;
};
struct hostList hl[HOSTLIMIT];

int hostind = 0;
    
unordered_map<string,struct hostList *> hostMap ;
struct sockaddr_in socket_clt,sock_server;
unsigned int length=sizeof(struct sockaddr_in);
int l = sizeof(struct sockaddr_in);
const std::string WHITESPACE = " \n\r\t\f\v";
struct sockaddr_in sock_serv;

void dummy_host() ;
int create_socket (int port, char* ipaddr);
void add_host();
string request_exec(int, struct sockaddr_in  ,int ,string,string);
vector<pair<string,string>> executeOnRemoteTerminal(string command) ;

void tokenize(string const &str, const char* delim,
			vector<string> &out) ;

void tokenizecommand(string const &str, const char* delim,
			vector<string> &out) ;
int create_socket_cl (int port, char* ipaddr) ;
int create_socket_port (int port) ;

int listenAsServer() ; 
