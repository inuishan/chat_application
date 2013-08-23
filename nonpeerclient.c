#include "accesories.c"

#define MAXDATASIZE 100 // max number of bytes we can get at once
char username[50];
//get sockaddr ,IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family ==AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
		if((recv(fd,msgb,sizeof(msgb),0)) <= 0) //receiving username
		{
			errorp("peerserver>login_check",0,0,"recv username");
			return 0;
		}
		printf("%s", msgb);
	}

}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if(argc != 2) {
		fprintf(stderr,"usage: client hostname\n");
		exit(1);
	}
	
	printf("Welcome to client.\n\nEnter your promt name\n");
	scanf("%s",username);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(rv));
		return 1;
	}

	//lopp through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
			perror("client: socket");
			continue;
		}

		if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}
	
	if(p ==NULL) {
		fprintf(stderr,"client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	printf("client : connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	chat(sockfd);
/*	if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {*/
/*		perror("recv");*/
/*		exit(1);*/
/*	}*/

/*	buf[numbytes] = '\0';*/

/*	printf("client: received '%s'\n",buf);*/

	close(sockfd);
	
	return 0;
}
