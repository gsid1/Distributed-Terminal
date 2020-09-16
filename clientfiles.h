#include "client.h"

string ltrim(const string& s)
{
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == string::npos) ? "" : s.substr(start);
}

string rtrim(const string& s)
{
	size_t end = s.find_last_not_of(WHITESPACE);
	return (end == string::npos) ? "" : s.substr(0, end + 1);
}

string trim(const string& s)
{
	return rtrim(ltrim(s));
}

// creates socket for client
int create_socket (int port, char* ipaddr){
    int sfd;
    
	sfd = socket(PF_INET,SOCK_STREAM,0);
    if (sfd == -1){
        cout << "socket fail" << endl ;
        return EXIT_FAILURE;
	}

	bzero(&sock_serv,l);
	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(port);
    if (inet_pton(AF_INET,ipaddr,&sock_serv.sin_addr)==0){
		printf("Invalid IP adress\n");
		return EXIT_FAILURE;
	}
    return sfd;
}

// adds friend to hostList
void add_host(){
    struct hostList newHost ;
	printf("Enter hostname: ");
	scanf("%s",newHost.hostname);	
    cout << newHost.hostname << endl ;
	printf("Enter ip: ");
	scanf("%s",newHost.ip);
    cout << newHost.ip << endl ;
	printf("Enter port: ");
	scanf("%d",&newHost.port);
    cout << newHost.port << endl ;
    hl[hostind] = newHost;
    hostMap[newHost.hostname] = &hl[hostind] ;
    printf("New hostname=%s, ip=%s, port=%d\n",newHost.hostname, newHost.ip, newHost.port);
	hostind++ ;
}

void add_host_details(int clt_port,char * clt_ip){
    struct hostList newHost ;
    strcpy(newHost.hostname,"client") ;
    strcpy(newHost.ip,clt_ip) ;
    newHost.port = clt_port ;
    hl[hostind] = newHost;
    hostMap[newHost.hostname] = &hl[hostind] ;
    printf("New hostname=%s, ip=%s, port=%d\n",newHost.hostname, newHost.ip, newHost.port);
	hostind++ ;
}

void dummy_host()
{
    struct hostList clientHost ;
    strcpy(clientHost.hostname,"client") ;
    strcpy(clientHost.ip,"172.17.87.176") ;
    clientHost.port = 1240 ;
    hl[hostind] = clientHost;
    hostMap[clientHost.hostname] = &hl[hostind] ;
    printf("New hostname=%s, ip=%s, port=%d\n",clientHost.hostname, clientHost.ip, clientHost.port);
	hostind++ ;
    struct hostList newHost ;
    strcpy(newHost.hostname,"a") ;
    strcpy(newHost.ip,"172.17.87.206") ;
    newHost.port = 1234 ;
    hl[hostind] = newHost;
    hostMap[newHost.hostname] = &hl[hostind] ;
    printf("New hostname=%s, ip=%s, port=%d\n",newHost.hostname, newHost.ip, newHost.port);
	hostind++ ;
    struct hostList newHost1 ;
    strcpy(newHost1.hostname,"b") ;
    strcpy(newHost1.ip,"172.17.87.206") ;
    newHost1.port = 1235 ;
    hl[hostind] = newHost1 ;
    hostMap[newHost1.hostname] = &hl[hostind] ;
    printf("New hostname=%s, ip=%s, port=%d\n",newHost1.hostname, newHost1.ip, newHost1.port);
	hostind++ ;
    struct hostList newHost2 ;
    strcpy(newHost2.hostname,"c") ;
    strcpy(newHost2.ip,"172.17.87.206") ;
    newHost2.port = 1236 ;
    hl[hostind] = newHost2;
    hostMap[newHost2.hostname] = &hl[hostind] ;
    printf("New hostname=%s, ip=%s, port=%d\n",newHost2.hostname, newHost2.ip, newHost2.port);
	hostind++ ;
}

// request file list from frienc
string request_exec(int sfd, struct sockaddr_in sock_serv ,int l,string inputfile,bool ispipe,string hostcommand,string sendcommand)
{
   char buf[BUFFER] ;
    char *command = new char[hostcommand.size()+1]; 
    strcpy(command,hostcommand.c_str()) ;
 	bzero(&buf,BUFFER);
    if(ispipe){
        cout << "sending input file to remote pipe" << endl ;
        string sendbuf = "file:"+inputfile ;
        int m=sendto(sfd,sendbuf.c_str(),strlen(sendbuf.c_str()),0,(struct sockaddr*)&sock_serv,l);    
        if(m == -1){
            cout << "data not send" << endl ;
            return "";
        }
    }
	// printf("Requesting execution of command %s\n",command);
    sendcommand += "$command:"+string(command) ;
    int m = sendto(sfd,sendcommand.c_str(),strlen(sendcommand.c_str()),0,(struct sockaddr*)&sock_serv,l);
	if(m==-1){
        cout << "send error" << endl ;
        return "";
	}
	int bytesrcv = recv(sfd,buf,BUFFER,0);
    if(bytesrcv == -1) cout << "recv error" << endl ;
    printf("\nResult: \n%s",buf);
    string data = string(buf) ;
    return data; 	
}

void tokenizecommand(std::string const &str, const char* delim,pair<string,string> &out){
    char *token = strtok(const_cast<char*>(str.c_str()), delim);
    out.first = trim(string(token)) ;
    while (token != nullptr)
	{
    	token = strtok(nullptr, delim);
        if(token != nullptr) 
        out.second = trim(string(token)) ;
	}
}

void tokenize(std::string const &str, const char* delim,
			vector<pair<string,string>> &out)
{
    vector<string> tokens ;
    char *token = strtok(const_cast<char*>(str.c_str()), delim);
    string newtoken = trim(string(token)) ;
    tokens.push_back(newtoken) ;
    while (token != nullptr)
	{
    	token = strtok(nullptr, delim);
        if(token != nullptr){
            string newtoken1 = trim(string(token)) ;
            tokens.push_back(newtoken1) ;
        } 
	}

    for(auto token : tokens){
        pair<string,string> p ;
        tokenizecommand(token,">",p) ;
        out.push_back(p) ;
    }
}

vector<pair<string,string>> executeOnRemoteTerminal(string commandline){
    vector<pair<string,string>> tokens ;
    tokenize(commandline,"||",tokens) ;
    cout << "------------------" << endl ;
    for(auto token:tokens){
        cout << token.first << " " << token.second << endl ;
    }
    cout << "------------------" << endl ;
    bool ispipe = false ;
    string inputfile = "";
    string sendcommands = "" ;
    sendcommands += tokens[0].second ;
    for(int i = 1 ; i < (int)tokens.size() ; i++){
        sendcommands += "||" + tokens[i].first + ">" + tokens[i].second ;
    }
    
    pid_t pid= fork() ;
    if(pid == 0){
        listenAsServer() ;     
        exit(0) ;
    }

    auto p = tokens[0] ;
    string hostname = trim(p.first) ;
    string command = p.second ;
    struct hostList *host ;
    if(hostMap.find(hostname) != hostMap.end()){
        host = hostMap[hostname] ;
        int sfd = create_socket(host->port, host->ip); 
        if(connect(sfd,(struct sockaddr*)&sock_serv,length)!=-1){
            string file = "src:$file:" ;
            cout << "### sending data to terminal" << endl ;
            string outputdata = request_exec(sfd,sock_serv,l,inputfile,ispipe,sendcommands,file);
            inputfile = "" ;
            inputfile.assign(outputdata) ;
        }
        else{
            cout << "connection failed" ;
            return tokens ;
        }
    }
    
    int status ;
    waitpid(pid, &status, 0); 
  
    if ( WIFEXITED(status) ) 
    { 
        WEXITSTATUS(status);         
    }
    return tokens ;
}

int listenAsServer(){

    int server_fd, new_socket,valread ; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[BUFFER] = {0}; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 

    struct hostList * host = hostMap["client"] ;
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(host->port); 
       
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){ 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (listen(server_fd, 3) < 0){ 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){ 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 

    valread = read( new_socket , buffer, BUFFER); 
    if(valread == -1) cout << "rcv error" << endl ;
    printf("%s\n",buffer ); 
    return 0 ;
}

// creating socket connection
int create_socket_port (int port){
    int l;
	int sfd;
    int yes=1;
    
	sfd = socket(PF_INET,SOCK_STREAM,0);
	if (sfd == -1){
        perror("socket creation error");
        return EXIT_SUCCESS;
	}
    
    if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,&yes,sizeof(int)) == -1 ) {
        perror("Socket option error");
        exit(5);
    }
    
	l=sizeof(struct sockaddr_in);
	bzero(&sock_server,l);
	
	sock_server.sin_family=AF_INET;
	sock_server.sin_port=htons(port);
	sock_server.sin_addr.s_addr=htonl(INADDR_ANY);
    
	if(bind(sfd,(struct sockaddr*)&sock_server,l)==-1){
		perror("bind fails");
		return EXIT_FAILURE;
	}
    printf("bind successful\n") ;
    return sfd;
}


