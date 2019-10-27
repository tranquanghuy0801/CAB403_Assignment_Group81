/** CAB403 - Systems Programming 
 *  Group 81 - DS Assignment 
 * 	Quang Huy Tran - n10069275 
 *  Tuan Minh Nguyen - 
 *  Ho Fong Law - 
 * */
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "helper.h"
#include <unistd.h>
#define MAX 1024
#define BACKLOG 10 
int total[NUM_CHANNELS] = { 0 };
int isRunning; // for LIVEFEED exit
int running = 1;//for closing program
string channel_mess[NUM_CHANNELS][NUM_MESS];

char *DisplayStats(htab_t *hClient, string hChannel[NUM_CHANNELS][NUM_MESS], char *clientID)
{
	char *result = (char *)malloc(sizeof(char) * 1024);
	char *mess = (char *)malloc(sizeof(char) * 1024);
	if (htab_find(hClient, clientID) == NULL)
	{
		printf("Not subscribed to any channels\n");
	} else{
		node_t *channels = htab_find(hClient, clientID)->subChannel;
		for(; channels != NULL; channels = channels->next)
		{
			int channelID = channels->channelID;
			sprintf(mess,"%d \t%d \t %d \t%d\n",channelID,total[channelID],channels->countRead,total[channelID]-channels->countRead-channels->startPoint);
			strcat(result,mess);
		}
	}
	free(mess);
	if(strcmp(result,"")==0){
		return "No messages in channels\n";
	}
	else{
		return result;
	}
}


char *ReadNextMess(htab_t *hClient,string hChannel[NUM_CHANNELS][NUM_MESS], char *clientID, int channelID)
{
	node_t *channelFound = node_find_channel(hClient,clientID,channelID);
	int messID = channelFound->messIndex;
	printf("%d",messID);
	if(strlen(hChannel[channelID][messID].x) == 0){
		return "No new messages\n";
	}
	else{
		channelFound->messIndex++;
		channelFound->countRead++;
		channelFound->countUnread--;
		return hChannel[channelID][messID].x;
	}
}

char *ReadAllMess(htab_t *hClient, string hChannel[NUM_CHANNELS][NUM_MESS], char *clientID)
{
	char *result = (char *)malloc(sizeof(char) * 1024);
	char *mess = (char *)malloc(sizeof(char) * 1024);
	if (htab_find(hClient, clientID) == NULL)
	{
		printf("Not subscribed to any channels\n");
	}
	else
	{
		int i;
		node_t *channels = htab_find(hClient, clientID)->subChannel;
		for(; channels != NULL; channels = channels->next)
		{
			int channelID = channels->channelID;
			int messIndex = channels->messIndex;
			for(i = messIndex; i < 10;i++){
				if(strlen(hChannel[channelID][i].x) != 0){
					sprintf(mess,"%d:%s\n",channelID,hChannel[channelID][i].x);
					strcat(result,mess);
					channels->messIndex++;
				}
				else{
					break;
				}
			}
		}
	}
	free(mess);
	if(strcmp(result,"")==0){
		return "No new messages\n";
	}
	else{
		return result;
	}
}


/* Thread routine to serve connection to client. */
void main_function(int new_socket_fd, htab_t hClient,  string channel_mess[NUM_CHANNELS][NUM_MESS], char *str);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

int main(int argc, char *argv[])
{
	int port;
	int socket_fd, new_socket_fd;
	struct sockaddr_in address;
	socklen_t client_address_len;
	signal(SIGINT,signal_handler);
	//Hash tables
	htab_t hClient;
	int buckets = 5;

	key_t ShmKEY = 1234;
	int ShmID;
	
	string (*ptr)[NUM_MESS];
	int texts = 1024;
	int rows = NUM_CHANNELS;
	int columns = NUM_MESS;
	int pid;

	ShmID = shmget(ShmKEY, sizeof(char[NUM_CHANNELS][NUM_MESS][1024]), IPC_CREAT | 0666);
	if (ShmID < 0)
	{
		printf("*** shmget error (server) ***\n");
		exit(1);
	}

	ptr = shmat(ShmID, NULL, 0);
	if (*ptr == (void *)(-1))
	{
		printf("*** shmat error (server) ***\n");
		exit(1);
	}



	printf("Server has attached the shared memory...\n");
	//Initialize client hashtable
	//char* channel_mess[255][10] = {{NULL}};
	if (!htab_init(&hClient, buckets))
	{
		printf("failed to initialise hash table\n");
		return EXIT_FAILURE;
	}
	/* Get port from command line arguments or stdin. */
	if (argc < 2)
	{
		port = 12345;
		fprintf(stderr, "Using default port:12345\n");
	}else if(argc > 2){
		port = atoi(argv[1]);
		fprintf(stderr, "Port number needed\n");
	}

	/* Initialise IPv4 address. */
	memset(&address, 0, sizeof address);
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	/* Create TCP socket. */
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		exit(1);
	}
	puts("Socket created");
	int length = snprintf(NULL, 0, "%d", socket_fd);
	char *str = malloc(length + 1);
	snprintf(str, length + 1, "%d", socket_fd);

	/* Bind address to socket. */
	if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1)
	{
		perror("bind");
		exit(1);
	}
	puts("Bind done");

	/* Listen on socket. */
	if (listen(socket_fd, BACKLOG) == -1)
	{
		perror("listen");
		exit(1);
	}
	puts("Waiting for incoming connections...");

	/* Assign signal handlers to signals. */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		perror("signal");
		exit(1);
	}
	if (signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		perror("signal");
		exit(1);
	}
	if (signal(SIGINT, signal_handler) == SIG_ERR)
	{
		perror("signal");
		exit(1);
	}

	while (running)
	{
		/* Accept connection to client. */
		//client_address_len = sizeof pthread_arg->client_address;
		struct sockaddr_in client_address;
		client_address_len = sizeof(client_address);
		new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_len);
		if (new_socket_fd == -1)
		{
			perror("accept");
			//free(pthread_arg);
			continue;
		}
		puts("Connection accepted");
		if ((pid = fork()) == -1)
		{
			printf("Fork not working");
			close(new_socket_fd);
			continue;
		}
		else if (pid == 0) // if pid is child
		{
			close(socket_fd);
			printf("child\n");
			main_function(new_socket_fd, hClient,ptr, str);
			printf("end main\n");
		}
	}

	close(socket_fd);
	/*
	   * TODO: If you really want to close the socket, you would do it in
	   * signal_handler(), meaning socket_fd would need to be a global variable.
	   */
	  shmdt(ptr);
	return 0;
}


void main_function(int new_socket_fd, htab_t hClient, string channel_mess[NUM_CHANNELS][NUM_MESS], char *str)
{
	char messages[MAX];
	int channel_id;
	

	// infinite loop for chat
	for (;;)
	{
		memset(messages,'\0',sizeof(messages));
		read(new_socket_fd, messages, sizeof(messages));
		if (strcmp(messages, "SUB") == 0)
		{
			memset(messages,'\0',sizeof(messages));
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id >= 0 && channel_id <= 255 && (node_find_channel(&hClient, str, channel_id) == NULL))
				{
					htab_add_node(&hClient,channel_mess, str, channel_id);
					sprintf(messages, "Subscribed to channel %d\n", channel_id);
				}
				else if (channel_id >= 0 && channel_id <= 255 && (node_find_channel(&hClient, str, channel_id) != NULL))
				{
					sprintf(messages, "Already subscribed to channel %d\n", channel_id);
				}
				else{
					sprintf(messages, "Invalid channel: %d\n", channel_id);
				}
			}
			else
			{
				sprintf(messages, "Miss channel ID\n");
			}
		}
		else if (strcmp(messages, "UNSUB") == 0)
		{
			memset(messages,'\0',sizeof(messages));
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id >= 0 && channel_id <= 255 && node_find_channel(&hClient, str, channel_id) != NULL)
				{
					htab_delete_node(&hClient, str, channel_id);
					sprintf(messages, "Unsubscribed from channel %d\n", channel_id);
				}
				else if (channel_id >= 0 && channel_id <= 255 && node_find_channel(&hClient, str, channel_id) == NULL)
				{
					sprintf(messages, "Not subscribed from channel %d\n", channel_id);
				}
				else
				{
					sprintf(messages, "Invalid channel: %d\n\n", channel_id);
				}
			}
			else
			{
				sprintf(messages, "Miss channel ID\n");
			}
		}
		else if (strcmp(messages, "SEND") == 0)
		{
			char *send_message = malloc(sizeof(char) * 1024);
			memset(messages,'\0',sizeof(messages));
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id >= 0 && channel_id <= 255 && read(new_socket_fd, send_message, 1024) != -1)
				{
					total[channel_id]++;
					message_add(send_message,channel_mess,channel_id);
					sprintf(messages, "Send to channel %d successfully\n", channel_id);
				}
				else
				{
					sprintf(messages, "Invalid channel: %d\n", channel_id);
				}
			}
			else
			{
				sprintf(messages, "Miss channel ID\n");
			}
		}
		else if (strcmp(messages, "NEXT") == 0)
		{
			memset(messages,'\0',sizeof(messages));
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id == 265)
				{
					sprintf(messages, "%s",ReadAllMess(&hClient,channel_mess,str));
				}
				else if(channel_id >= 0 && channel_id <= 255 && node_find_channel(&hClient,str, channel_id) != NULL)
				{
					//sprintf(messages, "%s\n", ReadNextMess(&hClient,&hChannel,str,channel_id));
					sprintf(messages, "%s\n", ReadNextMess(&hClient,channel_mess,str,channel_id));
				}
				else if(channel_id >= 0 && channel_id <= 255 && node_find_channel(&hClient,str, channel_id) == NULL)
				{
					sprintf(messages, "Not subscribed to channel %d\n", channel_id);
				}
				else
				{
					sprintf(messages, "Invalid channel: %d\n", channel_id);
				}
			}
			else
			{
				sprintf(messages, "Miss channel ID\n");
			}
		}
		else if (strcmp(messages, "LIVEFEED") == 0)
		{
			memset(messages,'\0',sizeof(messages));
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				isRunning = 1; 
				if (channel_id == 265)
				{

				} 
				else if(channel_id >= 0 && channel_id <= 255 && node_find_channel(&hClient,str, channel_id) != NULL)
				{
					isRunning=1;
					while(isRunning == 1)
					{
						read(new_socket_fd,&isRunning,sizeof(isRunning));
						sprintf(messages, "%s\n", ReadNextMess(&hClient,channel_mess,str,channel_id));
						write(new_socket_fd, messages, 1024);
					} 
				}
				else
				{
					sprintf(messages, "Invalid channel: %d\n", channel_id);
				}
			}else
			{
				sprintf(messages, "Error in receiving the data\n");
			}
			
		}
		else if (strcmp(messages, "CHANNELS") == 0)
		{
			memset(messages,'\0',sizeof(messages));
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id == 265)
				{
					sprintf(messages, "%s",DisplayStats(&hClient,channel_mess,str));
				}
			}else
			{
				sprintf(messages, "Error in receiving the data\n");
			}

		}
		else
		{
			memset(messages,'\0',sizeof(messages));
			sprintf(messages, "Cannot recognise your command\n");
		}
		write(new_socket_fd, messages, 1024);
	}
	close(new_socket_fd);
}

void signal_handler(int signal_number)
{
	/* TODO: Put exit cleanup code here. */
	running=0;
	exit(0);
}
