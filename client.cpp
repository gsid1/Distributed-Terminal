#include "clientfiles.h"

int main(int argc,char** argv){
    char choice,ch='Y' ;
    if(argc!=3) {
        perror("Syntax ./client <ip> <port>\n");
        exit(3);
    }
    
    dummy_host() ;

    while(ch=='Y')
    {
       cout << "Press A to Add Host\nPress B to access remote terminal\nEnter your choice: " ;
	   cin >> choice ;
       cin.ignore() ;
       cout << choice << endl ;
       switch (choice)
	   {
		   // add friend to friend list
	   		case 'A':
				add_host() ;
                break;

			// request particular file after choosing a friend from list of friends
	   		case 'B':
                cout << "Enter command (format: hostname > command): " << endl ;
                string command ;
                getline(cin,command) ;
                vector<pair<string,string>> tokens = executeOnRemoteTerminal(command) ;
                break ;
		}
		printf("\nDo you want to continue(press Y)? ");
		scanf(" %c",&ch);
   }
}


