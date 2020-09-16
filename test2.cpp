#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER 50000

int create_socket (int port, char* ipaddr);
struct friendList add_friend(struct friendList );
void request_list(int, struct sockaddr_in  ,int );

struct sockaddr_in sock_serv;

// struct to add friend 
struct friendList
{
	char hostname[60];
	char ip[15];
	int port;
};

int main (int argc, char**argv){
	
    int sfd,fd;
    char choice,ch='Y',choice1,yn;
    struct friendList fl[10];
    int str_index=0;
    char buf[BUFFER], filename[1000], hostname[60];
    long long int count=0, m,sz;
	long int n;
    int l=sizeof(struct sockaddr_in);
    
   while(ch=='Y')
   {
   	   printf("Press A to Add Friend\nPress B to Get File List\nEnter your choice: ");
	   scanf(" %c",&choice);
	   switch (choice)
	   {
		   // add friend to friend list
	   		case 'A':{
				struct friendList newFriend;
	   			fl[str_index] = add_friend(newFriend);
	   			printf("New hostname=%s, ip=%s, port=%d\n",fl[str_index].hostname,fl[str_index].ip,fl[str_index].port);
	   			str_index++;
	   			break;
			   }
			// request particular file after choosing a friend from list of friends
	   		case 'B':
	   			printf("\nList of hostnames:\n");
	   			int i ;

				for(i=0;i< str_index;i++)	printf("%s\n",fl[i].hostname );
	   			
	   			printf("\nEnter hostname: ");
	   			int found=0;
	   			scanf("%s",hostname);
	   			for (i=0;i< str_index;i++)
			    	if(strcmp(hostname,fl[i].hostname)==0){
			    		sfd=create_socket(1238, "127.0.0.1");
			    		found=1;
			    		break;
			    	}
			    
			    if(found==0){
			    		printf("\nNo such hostname.");
			    		break;
			    }
			    if(connect(sfd,(struct sockaddr*)&sock_serv,l)==-1){
			        perror("connection error\n");
			        exit (3);
			    }

				// copies all the friends hostname to list l
	   			request_list(sfd,sock_serv,l);
	   			printf("You found the file you are interested in(Y/N)? ");
	   			
	   			scanf(" %c",&choice1);
	   			switch (choice1)
	   			{
					//    file not present with the friend, continue to ask other friend for file
	   				case 'N': break ;
					//    choose a file name you want to download
	   				case 'Y':
	   					printf("Select file name:");
						scanf("%s",filename);
						
						// sending filename to the friend 
						m=sendto(sfd,filename,strlen(filename),0,(struct sockaddr*)&sock_serv,l);

						if(m==-1){
							perror("Error in sending");
							return EXIT_FAILURE;
						}

						bzero(buf,BUFFER);

						// receiving file from friend
					    n=recv(sfd,buf,BUFFER,0);
					    if(strcmp(buf,"Error in opening file")==0)
					    {
					    	printf("Either file not found or error in opening file.");
					    }
					    else
					    {
					    	printf("Creating the copied output file : %s\n",filename);
						    if ((fd=open(filename,O_CREAT|O_WRONLY,0600))==-1){
						                perror("File not opened");
						                exit (3);
						    }
					    	while(1) {
						        if(n==-1){
						            perror("Receive Failed.");
						            exit(5);
						        }
						        if((m=write(fd,buf,n))==-1){
						            perror("Not able to write in file.");
						            exit (6);
						        }
						        count=count+m;
						        
						        bzero(buf,BUFFER);
						        n=recv(sfd,buf,BUFFER,0);
						        if(n < 1024)
						        	break;
		        			}

		        			if((m=write(fd,buf,n))==-1){
						        perror("Not able to write in file.");
						    	exit (6);
						    }
						    count=count+m;
						    printf("Total Bytes written:%lld\n", count);
		        			close(fd);
					    }
					    
	    				break;
	   				default: 
	   					printf("\nWrong choice entered for interested file.");
	   					break;
	   			}
	   			break;
	   		default: 
	   			printf("\nWrong choice entered from menu.");
	   			break;
	   			
		}
		printf("\nDo you want to continue(press Y)? ");
		scanf(" %c",&ch);
   }
   close(sfd);
   
}


// creates socket for client
int create_socket (int port, char* ipaddr){
    int l;
	int sfd;
    
	sfd = socket(PF_INET,SOCK_STREAM,0);
	if (sfd == -1){
        perror("socket fail");
        return EXIT_FAILURE;
	}

	l=sizeof(struct sockaddr_in);
	bzero(&sock_serv,l);
	
	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(port);
    if (inet_pton(AF_INET,ipaddr,&sock_serv.sin_addr)==0){
		printf("Invalid IP adress\n");
		return EXIT_FAILURE;
	}
    
    return sfd;
}

