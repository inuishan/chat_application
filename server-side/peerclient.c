#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <error.h>
#define PORT "1618" //the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

static const int BUFFER_SIZE = 16*1024;
//get sockaddr ,IPv4 or IPv6:




void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family ==AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int registerYourself(int sockfd)
{	
	printf("2\n");
	char username[50], password[50],registerStatus[2];
	int return_value,choice;
	printf("You have chosen to register yourself\n");
	printf("Please enter a username and password\n");
	printf("Username:");
	scanf("%s",username);
	//printf("You entered %s\n",username);
	send(sockfd,username,sizeof(username),0); // sending username
	printf("\nPassword:");
	scanf("%s",password);    //sending password
	//printf("-- %s\n",password);
	send(sockfd,password,sizeof(password),0);			//sending password
	recv(sockfd,registerStatus,sizeof(registerStatus),0);//receiving from server that register is successful or not
	if(strcmp(registerStatus,"1")==0) //If register successful then promoting for log-in
		{
		printf("Registration successfull\n Please Log in\n");
		return_value=login(sockfd);
		return return_value;
		}
	else if(strcmp(registerStatus,"0")==0)// Asking user what to do
		{
		printf("Username already taken...Please try again with other username...\nPress:\t 1 to try again\n\t2 to quit");
		scanf("%d",&choice);
		switch(choice)
			{
			case 1:
			      send(sockfd,"1", 2, 0);
				registerYourself(sockfd);
				break;
			case 2:
				send(sockfd,"0",2,0);
				return 0;
			}			
		}
}
char username[50];
int login(int sockfd)
{	
	printf("3\n");
	char password[50],login_status[2];
	int choice;
	printf("You have chosen to login yourself\n");
	printf("Please enter a username and password\n");
	printf("Username:");
	scanf("%s",username);
//	printf("-- %s\n",username);
	send(sockfd,username,sizeof(username),0);
	send(sockfd,username,sizeof(username),0);//send username
	printf("\nPassword:");
	scanf("%s",password);
	//printf("-- %s\n",password);
	send(sockfd,password,sizeof(password),0);//send password
	recv(sockfd,login_status,sizeof(login_status),0); // //receiving from server that login is successful or not
	if(strcmp(login_status,"1")==0)
		{
		printf("login successfully 1\n");
		//while(1);// doing other job here
		return 1;
		
		}
		else if(strcmp(login_status,"0")==0)
		{
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
			//TODO default case	
			}
		}
		//break;//case 2 ends
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
	//	scanf("%s",msga);
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
	err = pthread_create(&tid, NULL, chat_thread_func_to_write, thread_arg );
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
		recv(fd,msgb,sizeof(msgb),0);
		printf("%s", msgb);
	}

}

void FTP(char filename[50],int sockfd)
{
send(sockfd,filename,strlen(filename),0); 
printf("Connected to remote host, sending file [%s]\n", filename);
char buf[BUFFER_SIZE];
FILE * fpIn = fopen(filename, "r");
while(1)
	{
	ssize_t bytesRead = fread(buf, 1, sizeof(buf), fpIn);
	if (bytesRead <= 0) 
	break;  // EOF
	printf("Read %i bytes from file, sending them to network...\n", (int)bytesRead);
	if (send(sockfd, buf, bytesRead, 0) != bytesRead)
		{
	      		      //perror("send");
		break;
		}
	}
}

void record(int sockfd)
{
printf("Press Cntl+C to stop recording\n");
system("arecord message.wav");
FTP("message.wav",sockfd);
}


int main(int argc, char *argv[])
{
	char fname[50], s[INET6_ADDRSTRLEN], buf[MAXDATASIZE], username[50], password[50], registerStatus[2], login_status[2];
	int sockfd, numbytes,fp,i,rv, choice,iflogin=0;
	
	struct addrinfo hints, *servinfo, *p;
	
	if(argc != 2) 
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

		printf("\n----------------Menu-------------------------\n1.Register\n2.Log in\n");
		scanf("%d",&i);
		int status_value;	
		//switch(i)
		//{
			//case 1: 
		if(i==1)
			{
				send(sockfd,"1", 2, 0);//sending server that user want to register
				printf("1\n");
				status_value = registerYourself(sockfd);
				if(status_value == 1)
					{
					printf("Logged in successful inside case 1\n");
					iflogin = 1;
					printf("HERE 1-----------------\n");
					printf("HERE 1-----------------\n");
					printf("HERE 1-----------------\n");
					printf("HERE 1-----------------\n");
					printf("HERE 1-----------------\n");
					printf("HERE 1-----------------\n");
					}
				else	{
					printf("Bye!!!");	
					printf("HERE 1!!!!!!!!!!!!!\n");		
					}
					printf("this is it\n");
			      }
		else
			{
				printf("Inside case 2");
				send(sockfd,"2", 2, 0); //sending server that user want to login
				status_value = login(sockfd);
				if(status_value == 1)
					{
					printf("Logged in successful inside case 2\n");
					iflogin = 1;
					}
				else	{
					printf("Bye!!!");			
					}
				//break;//case 2 ends*/
			}//switch ends
		//	scanf("%d",&i);
		//	printf("%d\n",i);
		fflush(stdout);
		fflush(stdin);
		fflush(stderr);
		//sleep(5);
//		int xyz;
//		scanf("%d",&xyz);
//		printf("%d\n",xyz);
		if(iflogin==1)
		{
		printf("You are successfully Logged in");
		printf("Here are the things you can do..\n");
		printf("1.chat\n\t2.send a file\n\t3.Record a message and send");
		int menu_choice;
		scanf("%d",&menu_choice);
		if(menu_choice==1)
			{
				send(sockfd,"1", 2, 0);
				chat(sockfd);
			}
		else if(menu_choice==2)
			{
			//TODO FTP code
			char filename[30];
			send(sockfd,"2", 2, 0);
			printf("Enter the file name\n"); 
			scanf("%s",filename); 
			FTP(filename,sockfd);
			}	
		else if(menu_choice==3)
			{
			//TODO arecord code
			send(sockfd,"3", 2, 0);
			record(sockfd);
			
			}	
		else  {
			//TODO Enter again code
			printf("Wrong choice entered\n");
			}	
		}
		else{
		printf("Final Bye");
		}
		//scanf("%d",&i);
		//while(1); //doing other job
		//for(i=0;i<20;)
		close(sockfd);
	
		return 0;
}
