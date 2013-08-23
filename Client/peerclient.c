#include "accesories.c"

#define MAXDATASIZE 100 // max number of bytes we can get at once

static const int BUFFER_SIZE = 16*1024;
//get sockaddr ,IPv4 or IPv6:

void sigchld_handler(int s)
{
	logp(2,"peerclient Chat closed abnormally");
	printf("Thank You For Chatting");
	exit(0);
	errorp("peerclient sigchld handler",0,0,"Not exiting");
}



void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family ==AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//This function is called if someone registers himself
int registerYourself(int sockfd)
{	
	char username[50], password[50],registerStatus[2];
	int return_value,choice;
	printf("You have chosen to register yourself\n");
	printf("Please enter a username and password\n");
	printf("Username:");
	scanf("%s",username);
	send(sockfd,username,sizeof(username),0); // sending username
	printf("\nPassword:");
	scanf("%s",password);    
	send(sockfd,password,sizeof(password),0);			//sending password
	recv(sockfd,registerStatus,sizeof(registerStatus),0);//receiving from server that register is successful or not
	if(strcmp(registerStatus,"1")==0) //If register successful then promoting for log-in
		{
		printf("Registration successfull\n Please Log in\n");
		return_value=login(sockfd);   //calling login function
		return return_value;		//returning the login_status to main
		}
	else if(strcmp(registerStatus,"0")==0)// If username is already taken then asking user what to do
		{
		printf("Username already taken...Please try again with other username...\nPress:\t 1 to try again\n\t2 to quit");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:				//user chooses to register again
			      send(sockfd,"1", 2, 0);
				registerYourself(sockfd);
				break;
			case 2:				//user chooses to quit
				send(sockfd,"0",2,0);	
				return 0;
				break;
			dafault:
				printf("Wrong Choice Entered...\n");
				return 0;
			}			
		}
}
char username[50];
int login(int sockfd)			//This function is called when login is selected
{	
	char password[50],login_status[2];
	int choice;
	printf("You have chosen to login yourself\n");
	printf("Please enter a username and password\n");
	printf("Username:");
	scanf("%s",username);
	send(sockfd,username,sizeof(username),0);
	send(sockfd,username,sizeof(username),0);//send username
	printf("\nPassword:");
	scanf("%s",password);
	send(sockfd,password,sizeof(password),0);//send password
	recv(sockfd,login_status,sizeof(login_status),0); // //receiving from server that login is successful or not
	if(strcmp(login_status,"1")==0)
		{
		printf("login successfully 1\n");		
		return 1;
		
		}
		else if(strcmp(login_status,"0")==0)
		{
		logp(4,"peerclient>login Wrong Username Or Password");
		printf("Wrong Username And/Or Password\nPress:\n__1)__To Enter Again  |  __2)__To Quit");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:
				send(sockfd,"1", 2, 0);// sending that user want to login again
				login(sockfd);
				break;
			case 2:
				send(sockfd,"0",2,0);// sending that user want to stop
				return 0;
				break;
			dafault: printf("Wrong choice entered\n");
				return 0;
				break;	
			
			}
		}
		
}

void* chat_thread_func_to_write(void *arg)
{
	char msga[255],msg[255];
	int fd = (int)arg;	
	
	printf("\nYou>");

	getchar();
	for(;;)
	{
		
		fgets(msga, 255, stdin);
		strcpy(msg,username);
		strcat(msg,": ");
		strcat(msg,msga);
		send(fd,msg,sizeof(msg),0);
		printf("You>");
	}
	return (void *)0;
}

void chat(int fd)
{
	char msgb[255];	
	pthread_t tid;
	void * thread_arg;
	int err;
	
	thread_arg=(void *)fd;
	err = pthread_create(&tid, NULL, chat_thread_func_to_write, thread_arg );	//creating another thread
	if(err != 0)
	{
		printf("can't create thread: %s\n", strerror(err));
	}
	err = pthread_detach(tid);
	if(err != 0)
	{
		printf("can't detach thread: %s\n", strerror(err));
	}
	
	printf("Enter Something and then wait for response\n");
	for(;;)
	{
		if((recv(fd,msgb,sizeof(msgb),0)) <= 0) //receiving username
		{
			errorp("peerserver>login_check",0,0,"recv username");
			return;
		}
		printf("%s", msgb);
	}

}

int FTP(char filename1[50],int sockfd)			//Function that will transfer a file
{		
	printf("Enter Filename\n");
	char filename[50];
	scanf("%s",filename);

	printf("Connected to remote host, sending file [%s]\n", filename);
	char buf[BUFFER_SIZE];
	FILE * fpIn = fopen(filename, "r");
	if(fpIn==NULL)			//Error while opening the file
	{
		send(sockfd,"0", 2, 0);		//Notifying server accordingly
		return 0;
	}
	send(sockfd,"1", 2, 0);			//Telling server that I am sending file
	send(sockfd,filename,sizeof(filename),0); 		//sending filename to server
	while(1)								//reading till EOF occurs
	{
		ssize_t bytesRead = fread(buf, 1, sizeof(buf), fpIn);
		if (bytesRead <= 0) 
			break;  // EOF
		printf("Read %i bytes from file, sending them to network...\n", (int)bytesRead);
		printf("%s\n",buf);
		if (send(sockfd, buf, bytesRead, 0) != bytesRead)
		{
			  		      //perror("send");
			break;
		}
	}
	return 1;
}


int send_voice(char filename[50],int sockfd)			//Function that will transfer a file
{		
	//printf("Enter Filename\n");
	//char filename[50];
	//scanf("%s",filename);

	printf("Connected to remote host, sending file [%s]\n", filename);
	char buf[BUFFER_SIZE];
	FILE * fpIn = fopen(filename, "r");
	if(fpIn==NULL)			//Error while opening the file
	{
		send(sockfd,"0", 2, 0);		//Notifying server accordingly
		return 0;
	}
	send(sockfd,"1", 2, 0);			//Telling server that I am sending file
	send(sockfd,filename,sizeof(filename),0); 		//sending filename to server
	while(1)								//reading till EOF occurs
	{
		ssize_t bytesRead = fread(buf, 1, sizeof(buf), fpIn);
		if (bytesRead <= 0) 
			break;  // EOF
		//printf("Read %i bytes from file, sending them to network...\n", (int)bytesRead);
		printf("%s\n",buf);
		if (send(sockfd, buf, bytesRead, 0) != bytesRead)
		{
			  		      //perror("send");
			break;
		}
	}
	return 1;
}



void record(int sockfd)					//function that records a voice and then sends there.
{
printf("Press Cntl+C to stop recording\n");
system("arecord message.wav");			//Using system call to record a message
send_voice("message.wav",sockfd);				//Transferring the file through FTP function
}


int main(int argc, char *argv[])
{
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGTSTP, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}
	char fname[50], s[INET6_ADDRSTRLEN], buf[MAXDATASIZE], username[50], password[50], registerStatus[2], login_status[2];
	int sockfd, numbytes,fp,i,rv, choice,iflogin=0;
	
	struct addrinfo hints, *servinfo, *p;
	
	if(argc != 2) 							//if no of arguments not equal to 2
		{
			fprintf(stderr,"usage: client hostname\n");
			exit(1);
		}

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
		{
			fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
			return 1;
		}

		//lopp through all the results and connect to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next) 
		{
			if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
				perror("client: socket");
				continue;
			}

			if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
			{
				close(sockfd);
				perror("client: connect");
				continue;
			}
			break;
		}

		if(p ==NULL) 
		{
			fprintf(stderr,"client: failed to connect\n");
			return 2;
		}

		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
		printf("client : connecting to %s\n", s);

		freeaddrinfo(servinfo); // all done with this structure
		//Now prompt a user to register or login

		printf("\n----------------Menu-------------------------\n1.Register\n2.Log in\n");
		scanf("%d",&i);
		int status_value;	
		if(i==1)
			{
			send(sockfd,"1", 2, 0);//sending server that user want to register
			status_value = registerYourself(sockfd);     
			if(status_value == 1)
				{
				printf("Logged in successful\n");
				iflogin = 1;
				}
			else	{
				printf("Bye!!!");			
				}
			}
		else if(i==2)
			{
				send(sockfd,"2", 2, 0); //sending server that user want to login
				status_value = login(sockfd);
				if(status_value == 1)
					{
					printf("Logged in successful\n");
					iflogin = 1;
					}
				else	{
					printf("Bye!!!");			
					}
			}
		else{
		printf("Wrong choice entered\n");
		}
		if(iflogin==1)      //if the user is logged in then only...
		{
		int menu_choice;
		printf("Here are the things you can do..\n");
		printf("\t1.chat\n\t2.send a file\n\t3.Record a message and send\n");
		scanf("%d",&menu_choice);
		if(menu_choice==1)
			{
			send(sockfd,"1", 2, 0);    //informing server about the choice
			chat(sockfd);		   //initialize chat
			}
		else if(menu_choice==2)			
			{
			char filename[50];
			send(sockfd,"2", 2, 0);		//informing server about the choice
			printf("Enter the file name\n"); 
			scanf("%s",filename); 
			int sent_q = FTP(filename,sockfd);
			if(sent_q==1)
			printf("Sent Successfully\n");
			else printf("Could not send the file\n");
			}	
		else if(menu_choice==3)
			{
			send(sockfd,"3", 2, 0);		//informing server about the choice
			record(sockfd);			//calling record function
			}	
		else  {
			printf("Wrong choice entered\n");
			}	
		}
		else{
		printf("Final Bye\n");
		}
		close(sockfd);
	
		return 0;
}
