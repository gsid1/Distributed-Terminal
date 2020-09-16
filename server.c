#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>


#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>


#define BUFFER 1024
#define LOG 1

int create_socket (int port);
int listFiles(char *path,char* list);
int findFiles(char *path, char* filename, char* filepath);

struct sockaddr_in sock_server,socket_clt;

// thread function for each new socket connection between multiple peers
// data transfer of file
void* connectClient(void *args){
    
    printf("inside connect Client\n") ;
    int nsid = *((int*)args) ;
    char list[1000],path[200];
    int fd ;    
    char dest[INET_ADDRSTRLEN];
    char buffer[BUFFER],filename[256];
    unsigned int length=sizeof(struct sockaddr_in);
    long int n, m,count=0;
    ushort clt_port;
    
    if(inet_ntop(AF_INET,&socket_clt.sin_addr,dest,INET_ADDRSTRLEN)==NULL){
        perror("Error connecting to client");
        exit (4);
    }
    clt_port=ntohs(socket_clt.sin_port);
    
    // received request for file list
    bzero(buffer,BUFFER);
    n=recv(nsid,buffer,BUFFER,0);
    printf("Recv command: %s\n",buffer);
    if(strcmp(buffer,"request file-list")==0)
    {
        printf("Received request sent by client\n");
        // fills list with all the files present in the current directory
        bzero(list,1000);
        listFiles(".",list);
        // send file list to requesting peer
        m=sendto(nsid,list,strlen(list),0,(struct sockaddr*)&socket_clt,length);
        printf("value of send bytes:%ld\n",m);
        if(m==-1){
            perror("send error");
            return EXIT_FAILURE;
         }
    }
    // received the requested file for download
    bzero(buffer,BUFFER);
    n=recv(nsid,buffer,BUFFER,0);
    printf("Recv filename: %s\n",buffer);
    bzero(path,200);

    // find file recursively in the current directory
    findFiles(".",buffer,path);
    //printf("Path is: %s\n",path);
        
    // sending error messages to peer
    if ((fd = open(path,O_RDONLY))==-1){
        perror("Error in opening file");
        char error[] = "Error in opening file";
        m=sendto(nsid,error,strlen(error),0,(struct sockaddr*)&socket_clt,length);
         if(m==-1){
            perror("Error in sending");
            return EXIT_FAILURE;
        }
    }
    else
    {
        // read the file
        bzero(&buffer,BUFFER);
        n=read(fd,buffer,BUFFER);
        // continue reading file and send chunks untill eof
        while(n){
                if(n==-1){
                perror("Error in reading the file.");
                return EXIT_FAILURE;
                }
                m=sendto(nsid,buffer,n,0,(struct sockaddr*)&socket_clt,length);
                if(m==-1){
                    perror("Error in sending");
                    return EXIT_FAILURE;
                }
            count+=m;
            bzero(buffer,BUFFER);
            n=read(fd,buffer,BUFFER);
        }
        
        m=sendto(nsid,buffer,0,0,(struct sockaddr*)&socket_clt,length);

        // prints total number of chunks transferred in connection link
        printf("Number of packets transferred : %ld\n",count);
            
        close(fd);
    }
    
    close(nsid); 
}

int main(int argc,char** argv){
    int sfd,fd;
    unsigned int length=sizeof(struct sockaddr_in);
    unsigned int nsid;
    char buf[BUFFER] ;
    
    if(argc!=2) {
        perror("Syntax ./server <num_port>\n");
        exit(3);
    }
    
    // create socket with listen port
    sfd = create_socket(atoi(argv[1]));
    printf("server socket created\n") ;
    bzero(buf,BUFFER);
    // listen to specified port
    listen(sfd,LOG);
    if(sfd == -1){
        printf("listen error\n") ;
    }
    printf("listening at port\n") ;
    pthread_t t[50] ;
    int i = 0 ;
    
    // accepts multiple peer requests
    while(1){
        printf("waiting for accept\n") ;
        nsid=accept(sfd,(struct sockaddr*)&socket_clt,&length);
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
}

// creating socket connection
int create_socket (int port){
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

// recursively finding all the files and copying it to the list
int listFiles(char *targetPath,char* list)
{
     char path[1000];
    
    struct dirent *dirp;
    DIR *dir = opendir(targetPath);

    if (!dir)
        return 0;

    while ((dirp = readdir(dir)) != NULL)
    {
        if (strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".") != 0)
        {
          
            strcpy(path, targetPath);
            strcat(path, "/");
            strcat(path, dirp->d_name);
            strcat(list,dirp->d_name);
            strcat(list,"\n");
            listFiles(path,list);
        }
    }

    closedir(dir);
    return 0;
}

// finding specified file recursively
int findFiles(char *targetPath, char* filename, char* filepath)
{
    char path[200];

    struct dirent *dirp;
    DIR *dir = opendir(targetPath);
   
    
    if (!dir)
        return 0 ;

    while ((dirp = readdir(dir)) != NULL)
    {
        if (strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".") != 0)
        {
            strcpy(path, targetPath);
            strcat(path, "/");
            strcat(path, dirp->d_name);
            if(strcmp(dirp->d_name,filename)==0)
            {   
                
                strcpy(filepath, path);
                closedir(dir);
                return 0;
            }
            findFiles(path,filename,filepath);
        }
    }
    closedir(dir);
    return 0;
}
