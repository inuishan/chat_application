#include "accesories.c"

static const int BUFFER_SIZE = 16*1024;

struct clients  //to store user information
{
	char name[50];
	char password[50];
}client;


void sigchld_handler(int s)
{
	logp(2,"peerclient Chat closed abnormally");
	printf("Thank You For Chatting");
	exit(0);
	errorp("peerclient sigchld handler",0,0,"Not exiting");
}

void *get_in_addr(struct sockaddr *sa) //getting ip address out of struct sockaddr
{
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//need to add one more string argument to show which part is calling this func

int login_check(int fd)
{	
	char data[50],choice[2],*name, *pass;
	int nbytes,ret_status,limit = 50;
	recv(fd,client.name,sizeof(client.name),0);
	if(( nbytes = recv(fd,client.name,sizeof(client.name),0)) <= 0) //receiving username
	{
		errorp("peerserver>login_check",0,0,"recv username");
		return 0;
	}
	if(( nbytes = recv(fd,client.password,sizeof(client.password),0)) <= 0) //receiving password
	{
		errorp("peerserver>login_check",0,0,"recv password");
		return 0;
	}
	//opening file containing login info
	FILE * record = fopen("records.txt","r+");
	if(record == NULL)
	{
		errorp("peerserver>login_check",1,errno,"record opening");
		return -1;
	}
	while(fgets(data,100,record)!=NULL)
	{//stored in file in one login info per lineas : username,password
		name = strtok(data,",");
		pass = strtok(NULL,",");
		pass = strtok(pass,"\n");
		if((strcmp(client.name,data)==0) && (strcmp(client.password,pass)==0))
		{
			if(send(fd,"1",2,0) == -1)//signaling client that everythin is fine
			{
				errorp("peerserver>login_check",0,0,"sending choice");
				return 0;
			}
			return 1;
		}
	}
			if(send(fd,"0",2,0) == -1)//if username already exists
			{
				errorp("peerserver>login_check",0,0,"sending choice");
				return 0;
			}
			//waiting for user chooice to enter pass again or not
			if((nbytes = recv(fd,choice,sizeof(choice),0)) <= 0)
			{
				errorp("peerserver>login_check",0,0,"sending choice");
				return 0;
			}
			if(!strcmp(choice,"1"))
			{
				ret_status = login_check(fd);
				
				
				if(ret_status == 1)
					return 1;
				else if(ret_status == 0)
					return 0;
				else if(ret_status == -1)
					return -1;		
			}
			else if(!strcmp(choice,"0"))
			{
				return 0;		
			}
		
	return 0;	
	logp(4,"peerserver>login_check exiting login without any region");
}

int rgstr(int fd)
{
	int nbytes,record_n,ret_status,limit = 50,passed =1;
	char data[50],choice[2], *name, *toBeWrit;
	
	if((nbytes = recv(fd,client.name,sizeof(client.name),0)) <= 0)//receiving username
	{
		errorp("peerserver>login_check",0,0,"recv username");
		return 0;
	}
	if((nbytes = recv(fd,client.password,sizeof(client.password),0)) <= 0) // receiving password
	{
		errorp("peerserver>login_check",0,0,"recv password");
		return 0;
	}
		//structing data to write in the file
	toBeWrit = (char *)malloc(strlen(client.name)+strlen(client.password)+3);
	strcpy(toBeWrit,client.name);
	strcat(toBeWrit,",");
	strcat(toBeWrit,client.password);
	strcat(toBeWrit,"\n");
		
	FILE * record = fopen("records.txt","r+");
	if(record == NULL)
	{
		errorp("peerserver>regstr",1,errno,"opening record");
		return -1;
	}
	//checking if username already exists
	while(fgets(data,100,record)!=NULL)
	{
		name = strtok(data,",");
		if(strcmp(client.name,data)==0)
		{
			passed = 0;
			break;
		}
	}	
			//if everything work fine
	if(passed == 1)
	{
		logp(1,"peerserver.c>regstr New user registered");	
		if(send(fd,"1",2,0) == -1)//singaling client
		{
			errorp("peerserver>regstr",0,0,"sending choice");
			return 0;
		}
		fclose(record);
			
		if((record_n = open("records.txt",O_WRONLY|O_APPEND)) == -1)
		{
			errorp("peerserver>regstr",0,0,"opening record");
			return 0;
		}
		if((nbytes =write(record_n,toBeWrit,strlen(toBeWrit))) <= 0)
		{
			errorp("peerserver>login_check",0,0,"Adding new user");
			logp(2,"writing new user to record");
			return 0;
		}

		return 1;
	}
	else//if username already exists
	{
		if(send(fd,"0",2,0) == -1) //singaling client to again ask to user whether to rgstr agian or not
		{
			errorp("peerserver>regstr",0,0,"sending choice");
			return 0;
		}

		//waiting for user chooice to enter pass again or not
		logp(4,"Again recieving choice");
		if((nbytes = recv(fd,choice,sizeof(choice),0)) <= 0) 
		{
			errorp("peerserver>regstr",0,0,"recv choice");
			return 0;
		}
		
		if(!strcmp(choice,"1"))
		{
			ret_status = rgstr(fd);
			logp(4,"Registering agian");
			if(ret_status == 1)
				return 1;
			else if(ret_status == 0)
				return 0;
			else if(ret_status == -1)
				return -1;		
		}
		else if(!strcmp(choice,"0"))
		{
			return 0;		
		}
	}
	return 0;
	logp(2,"Coming at end of regstr function");
}


void* chat_thread_func_to_write(void *arg)
{
	char msga[255],msg[255];
	int fd = (int) arg;
	
	
	printf("\nYou>");
	for(;;)
	{
		fgets(msga, 255, stdin);
		strcpy(msg,"Server: ");
		strcat(msg,msga);
		send(fd,msg,sizeof(msg),0);
		printf("You> ");
	}
	return (void *)0;
}

int chat(int fd)
{
	char msgb[255];	
	pthread_t tid;
	void * thread_arg;
	int err;
	
	thread_arg=(void *)fd;
	err = pthread_create(&tid, NULL, chat_thread_func_to_write, thread_arg );
	if(err != 0)
	{
		errorp("peerserver>chat",0,0,"Cannot create new thread");
	}
	err = pthread_detach(tid);
	if(err != 0)
	{
		errorp("peerserver>chat",0,0,"thread not detaching");
	}
	
	printf("Enter Something and then wait for response\n");
	for(;;)
	{
		if(recv(fd,msgb,sizeof(msgb),0) <= 0) //receiving username
		{
			errorp("peerserver>login_check",0,0,"recv username");
			return 0;
		}
		printf("%s", msgb);
	}	return 0;
}

int file_saver(int fd)
{
	logp(1,"User send the file");
	char fname[50], buf[BUFFER_SIZE];
	FILE * fpIn;
	ssize_t bytesReceived;
	char transferringQ[2];
	recv(fd,transferringQ,sizeof(transferringQ),0);
	if(strcmp(transferringQ,"1")==0)
	{
		recv(fd,fname,sizeof(fname),0);
		printf("Received file: %s\n\n",fname);
		fpIn = fopen(fname, "w");
		if (fpIn)
			{
				while(1)
	    		{
					bytesReceived = recv(fd, buf, sizeof(buf), 0);
//					printf("%s\n",buf);
	      			if (bytesReceived < 0) perror("recv");  // network error?
	      			if (bytesReceived == 0) break;   // sender closed connection, must be end of file
		    		if (fwrite(buf, 1, bytesReceived, fpIn) != (size_t) bytesReceived)
	        		{
	        			perror("fwrite");
	        			return 1;
	        		}
				}
				fclose(fpIn);
    		}
		else
			{ 
				printf("Error, couldn't open file [%s] to receive!\n", fname);
				return 1;
			}
           return 0;
     }
}

int voice_saver(int fd)
{
	char fname[50], buf[BUFFER_SIZE];;
	FILE * fpIn;
	ssize_t bytesReceived;
	recv(fd,fname,11,0);
	//recv(fd,fname,sizeof(fname),0);
	fpIn = fopen("message.wav", "w");
	if (fpIn)
	{
		while(1)
	    {
	    	bytesReceived = recv(fd, buf, sizeof(buf), 0);
	        if (bytesReceived < 0) perror("recv");  // network error?
	        if (bytesReceived == 0) break;   // sender closed connection, must be end of file
		    //printf("Received %i bytes from network, writing them to file...\n", (int) bytesReceived);
	        if (fwrite(buf, 1, bytesReceived, fpIn) != (size_t) bytesReceived)
	        {
	        	perror("fwrite");
	            return 1;
	        }
		}
	
  		fclose(fpIn);
    }
	else
	{ 
		printf("Error, couldn't open file [%s] to receive!\n", fname);
		return 1;
	}
	return 0;

}


void* thread_func(void* arg)// function that handles each connection
{
	int fd=(int)arg, login_status=0, rgstr_status=0, record_n,status;
	char choice[2];

    //opening record.txt- creating file
	record_n = open("records.txt",O_CREAT,0644);	
	close(record_n);

	printf("Set to receive the choice:\n");
	recv(fd,choice,sizeof(choice),0);
	printf("Received choice %s\n",choice);		
	if(!strcmp(choice,"1"))
	{
		rgstr_status = rgstr(fd);
		if(rgstr_status == 1)
		{
			printf("New User Registered\n");
			strcpy(choice,"2");
		}
		else if(rgstr_status == 0)
		{
			printf("rUsr close the connection\n");
			close(fd);
			return ((void*)0);
		}
		else if(rgstr_status == -1)
		{
			printf("There is some problem..\n");
			close(fd);
			return ((void*)0);
		}
	}
	if(!strcmp(choice,"2"))
	{
		login_status = login_check(fd);

		if(login_status == 1)
		{
			printf("Client %s log-ined\n",client.name);

			recv(fd,choice,sizeof(choice),0); //receiving user choice to chat, ftp, voice
			if(!strcmp(choice,"1"))
			{
				status = chat(fd);
				if(status == 0)
				{
					printf("Thanks for using chat. Session Ended\n");
					return 0;
				}
				else
				{
					printf("Some error occured while transfering the file");
					return (void *)1;
				}
			}
			else if(!strcmp(choice,"2"))
			{
				status = file_saver(fd);
				if(status == 0)
				{
					printf("File transfer successfully\n");
					return 0;
				}
				else
				{
					printf("Some error occured while transfering the file");
					return (void *)1;
				}
			}
			else if(!strcmp(choice,"3"))
			{
				status = voice_saver(fd);
				if(status == 0)
				{
					printf("Voice message transfer successfully and playing file\n");
					system("aplay message.wav");
					return 0;
				}
				else
				{
					printf("Some error occured while transfering the file");
					return (void *)1;
				}
			}
		}
		else if(login_status == 0)
		{
			printf("lUsr close the connection\n");
			close(fd);
			return ((void*)0);
		}
		else if(login_status == -1)
		{
			printf("There is some problem..\n");
			close(fd);
			return ((void*)0);
		}
	}

	printf("Done with user\n");
	return ((void*)0);
}



int main(void)
{
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGTSTP, &sa, NULL) == -1){
		perror("sigaction");
		exit(1);
	}
	logp(1,"peerserver used once agian");
	int listener_fd; //listening the socket descriptor which is listening for new calls
	int new_fd;    //newly accept()ed file descriptor , for each connection
		
	struct sockaddr_storage clientaddr; // client address is stored in this with a typecasting dependent on family of connection (IPv4 or IPv6)
	socklen_t addrlen;  //length of clientaddr for inet_ntoi

	char buf[256];  //buffer for client - reading buffer
	int nbytes;//return value of recv function

	char remoteIP[INET6_ADDRSTRLEN]; //store the ip of the remote client. Its size is defined as INET6_ADDRSTRLEN because now it can store both IP - IPv4 and IPv6

	int yes=1; // integer passed to setsockopt
	int i, j, rv;

	struct addrinfo hints, *link_struc, *struc_iter;// to use with getaddrinfo

	int err; // pthread_create error
	pthread_t ntid;
	void *thread_arg;
	void **thread_ret;
	
	//get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((rv = getaddrinfo(NULL, PORT, &hints, &link_struc)) != 0) {
		fprintf(stderr, "selectserver : %s\n",gai_strerror(rv));
		exit(1);
	}
	
	for(struc_iter = link_struc; struc_iter != NULL ; struc_iter = struc_iter->ai_next) {
		listener_fd = socket(struc_iter->ai_family, struc_iter->ai_socktype, struc_iter->ai_protocol);
		if(listener_fd < 0){
			continue;
		}
		logp(2,"peerserver : User came in addrinfo");
		//lose the pesky "address already in use " error message
		setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if(bind(listener_fd, struc_iter->ai_addr, struc_iter->ai_addrlen) < 0) {
			close(listener_fd);
			continue;
		}

		break;
	}

	//if we got here, it means we didnot get bound
	if(struc_iter == NULL){
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(link_struc); // all  done with this

	//listen
	if(listen(listener_fd, 10) == -1){ //make that socket active to listen with max connection queue - 10
		perror("listen");
		exit(3);
	}

	//here code to accept connection
		addrlen = sizeof clientaddr;
		new_fd = accept(listener_fd, (struct sockaddr *)&clientaddr, &addrlen);
		
		if(new_fd == -1){
			perror("accept");
			//continue;
		}
	
		inet_ntop(clientaddr.ss_family, get_in_addr((struct sockaddr *)&clientaddr), remoteIP, INET6_ADDRSTRLEN);
		printf("server : got connection from %s\n", remoteIP);
		
		thread_arg=(void *)new_fd;
		
		err = pthread_create(&ntid, NULL, thread_func,thread_arg );
		if(err != 0)
		{
			printf("can't create thread: %s\n", strerror(err));
		}

		err = pthread_join(ntid, thread_ret);
		if(err != 0)
		{
			printf("can't join thread: %s\n", strerror(err));
		}
		//TODO return pointer and atexit functions
		logp(4,"peerserver User close the connection");
	
	return 0;
}
