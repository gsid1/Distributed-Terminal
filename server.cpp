#include "clientfiles.h"
#include "server.h"

int main(int argc,char** argv){
    char buf[BUFFER] ;
    dummy_host() ;

    if(argc!=2) {
        perror("Syntax ./server <num_port>\n");
        exit(3);
    }
    int sfd = create_socket_server(atoi(argv[1]));
    // printf("server socket created\n") ;
    bzero(buf,BUFFER);
    // listen to specified port
    listen(sfd,LOG);
    if(sfd == -1){
        printf("listen error\n") ;
    }
    // printf("listening at port\n") ;
    int i = 0 ;
    pthread_t t[50] ;
    // accepts multiple peer requests
    while(1){
        // printf("waiting for accept\n") ;
        int nsid=accept(sfd,(struct sockaddr*)&socket_clt,&length);
        if(nsid==-1){
            perror("accept fail");
            return EXIT_FAILURE;
        }
        // creating the thread for channel communication with accepted peer 
        pthread_create(&t[i],NULL,connectClient,&nsid) ;
        printf("thread created for file transfer\n") ;
        i++ ;   
    }
    int j ;
    // waiting for all the channel communication to complete
    for(j = 0 ; j<i ; j++){
        pthread_join(t[j],NULL) ;
    }
    return EXIT_SUCCESS;
   return 0 ;
}


// creating socket connection
int create_socket_server (int port){
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
    // printf("bind successful\n") ;
    return sfd;
}


void* connectClient(void *args){
    
    printf("inside connect Client\n") ;
    int nsid = *((int*)args) ;
    char dest[INET_ADDRSTRLEN];
    char buffer[BUFFER] ;
    ushort clt_port;
    struct in_addr ipAddr ;
    
    if(inet_ntop(AF_INET,&socket_clt.sin_addr,dest,INET_ADDRSTRLEN)==NULL){
        perror("Error connecting to client");
        exit (4);
    }
    
    clt_port=ntohs(socket_clt.sin_port);
    ipAddr = socket_clt.sin_addr ;
    char clt_ip[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &ipAddr, clt_ip, INET_ADDRSTRLEN );

    // received request for file list
    bzero(buffer,BUFFER);
    int n=recvfrom(nsid,buffer,BUFFER,0,NULL,NULL);
    if(n == -1) cout << "recv error" << endl ;
    printf("Recv command: %s\n",buffer);
    
    string tmpdata = buffer ;
    string rcv1 = strtok((char*)tmpdata.c_str(),"$") ;
    string rcv2 = strtok(nullptr,"$") ;
    string rcv3 = strtok(nullptr,"$") ;
    cout << "Recieve 1:" << rcv1 << endl ;
    cout << "Recieve 2:" << rcv2 << endl ;
    cout << "Recieve 3:" << rcv3 << endl ;
    
        
    string type1 = strtok((char*)rcv1.c_str(),":") ;
    cout << "type1-->" << type1 << endl ;
    char * tpsrc = strtok(nullptr,"--") ;
    string finaldata = "" ;
    if(tpsrc == nullptr){
        string srcdetails = "src:"+ string(clt_ip) + " " + to_string((int)clt_port) + "#" ;
        finaldata += srcdetails ;
    }else{
        cout << tpsrc << endl ;
    }

    string type2 = strtok((char*)rcv2.c_str(),":") ;
    cout << "type2-->" << type2 << endl ;
    char * tpdst = strtok(nullptr,"##") ;

    string filecontent ;
    if(tpdst != nullptr){
        cout << "filecontent = " << tpdst << endl ;
        filecontent = tpdst ;
        ofstream myfile ;
        myfile.open("tmp.txt") ;
        myfile << filecontent ;
        myfile.close() ;
    }

    string type3 = strtok((char*)rcv3.c_str(),":") ;
    cout << "type3-->" << type3 << endl ;
    string command = strtok(nullptr,"#") ;
    cout << type3 << " " << command << endl ;
    int tokensize ;

    string data = executeOnRemoteTerminal2(command,&tokensize,clt_ip,clt_port,tpsrc) ;
    if(tokensize == 1){
        pid_t pid= fork() ;
        if(pid == 0){
            actAsClient(data) ;
        }
    }
    close(nsid); 
    return nullptr ;
}

string GetStdoutFromCommand(string cmd) {
    string data;
    FILE * stream;
    // const int max_buffer = BUFFER;
    char buffer[BUFFER];
    cmd.append(" 2>&1");

    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, BUFFER, stream) != NULL) 
                data.append(buffer);
        pclose(stream);
    }
    return data;
}

string executeOnRemoteTerminal2(string commandline,int *tokensize,string clt_ip,int clt_port,char *tpsrc){
    vector<pair<string,string>> tokens ;
    tokenize(commandline,"||",tokens) ;
    *tokensize = tokens.size() ;
    string filecontent = "" ;
    string command = tokens[0].first ;

    cout << "#### executing command --" << command << endl ;
    if(tpsrc != nullptr){
        filecontent = GetStdoutFromCommand(command+" tmp.txt");
        GetStdoutFromCommand("rm tmp.txt") ;
    }else{
        filecontent = GetStdoutFromCommand(command);
    }
    cout << "Executed command output : " << filecontent << endl ;
    if(tokens.size() == 1){
        return filecontent ;
    }
    string finaldata = "" ;
    string srcdetails = "src:"+ string(clt_ip) + " " + to_string((int)clt_port) ;
    finaldata += srcdetails ;
    
    finaldata += "$file:"+filecontent ;
    cout << "------------------" << endl ;
    for(auto token:tokens){
        cout << token.first << " " << token.second << endl ;
    }
    cout << "------------------" << endl ;
    
    string inputfile = "";
    string sendcommands = "" ;
    
    string hostname = "$" ;
    string mycommand = "" ;
    if(tokens.size() > 1){
        sendcommands += tokens[1].second ;
        hostname = tokens[1].first ;
        mycommand = tokens[1].second ;
        for(int i = 2 ; i < (int)tokens.size() ; i++){
            sendcommands += "||" + tokens[i].first + ">" + tokens[i].second ;
        }    
    }
    finaldata += "$command:"+sendcommands ;

    struct hostList *host ;
        if(hostMap.find(hostname) != hostMap.end()){
            host = hostMap[hostname] ;
            int sfd=create_socket(host->port, host->ip); 
            if(connect(sfd,(struct sockaddr*)&sock_serv,l)!=-1){
                cout << "### sendind data to next terminal" << endl ;
                int m=sendto(sfd,finaldata.c_str(),strlen(finaldata.c_str()),0,(struct sockaddr*)&socket_clt,length) ; 
                if(m == -1) cout << "send error" << endl ;
            }else{
                cout << "can not connect" << endl ;
            }
        }
    return finaldata ;
}

void actAsClient(string data){

    int sock = 0 ; 
    struct sockaddr_in serv_addr; 
    char *hello = (char *)data.c_str() ; 
    
    struct hostList *host = hostMap["client"] ;
    int port = host->port ;
    char* ip = host->ip ;
    // cout << "connecting to " << ip << " " << port << endl ;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("\n Socket creation error \n"); 
    }
    else{ 
        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(port); 
        
        // Convert IPv4 and IPv6 addresses from text to binary form 
        if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0){ 
            printf("\nInvalid address/ Address not supported \n"); 
        }else{                     
            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
                printf("\nConnection Failed \n"); 
            }else{
                send(sock , hello , strlen(hello) , 0 ); 
                cout << "msg sent to client" << endl ;
            } 
        }
    }
}