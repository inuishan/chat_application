#include <malloc.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

void logp(int typ, char* msg) // typ --> type(category) of message [1-Normal Log, 2-Warning(any unexpected thing happened), 3-Error, 4-Debugging Log ]
{
	int fd;
	time_t now;
	ssize_t wlength=0;
	char * dat;
	char * str;
	int size = 45+strlen(msg);//14+24+5+sizeof msg+1
	
	str= (char *) malloc(size);
	
	time(&now);//system time in seconds
	dat = ctime(&now); // converting seconds to date-time format
	dat = strtok(dat,"\n");
	
	//Appending type of log
	switch(typ)
	{
	case 1:
		strcpy(str,"__LOG__    |  ");
		strcat(str,dat);
		break;
	case 2:
		strcpy(str,"__WARN__   |  ");
		strcat(str,dat);
		break;
	case 3:
		strcpy(str,"__ERR__    |  ");
		strcat(str,dat);
		break;
	case 4:
		strcpy(str,"__DEBUG__  |  ");
		strcat(str,dat);
		break;
	default:
		strcpy(str,"__UNDEF__  |  ");
		strcat(str,dat);
		break;
	}
	
	
	strcat(str,"  |  ");
	strcat(str,msg);//appending message
	strcat(str,"\n");
	
	fd = open("logass", O_WRONLY | O_CREAT | O_APPEND, 0644); // should be opened somewhere else
	if (fd == -1)
		printf("Could not open log - %s\n",strerror(errno));
	else
	{//need to add lock to the file and printing error message
		while ( wlength < strlen(str) )
		{
			wlength = write(fd, str,strlen(str));
			if (wlength == -1)
			{
				printf("Error : writing log\n");
				break;
			}
		}
		
		
	}
}

void voice_record()
{
	printf("Press Cntrl+c to stop\n");
	system("arecord voice.wav");
}

void voice_play()
{
	printf("Press Cntrl+c to stop\n");
	system("aplay voice.wav");
}

void menu()
{
	//printf();
}

void main()
{

printf("aa %d  aa",(int)strlen(NULL));
voice_record();
voice_play();
}
