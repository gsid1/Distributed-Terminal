#include <dirent.h>
    
struct sockaddr_in socket_clt1 ;//,socket_clt;
// unsigned int length=sizeof(struct sockaddr_in);

int create_socket_server (int port);
int listFiles(char *path,char* list);
int findFiles(char *path, char* filename, char* filepath);
void* connectClient(void *args) ;
string GetStdoutFromCommand(string cmd) ;

string executeOnRemoteTerminal2(string commandline,int *tokensize,string cl_ip,int port, char* tpsrc) ;

string tokenizefirst(std::string const &str, const char* delim,const char* delim2,string &remaining) ;

void actAsClient(string data) ;