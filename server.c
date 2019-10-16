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
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "helper.h"
#include <unistd.h>
#define MAX 1024
#define BACKLOG 10
int my_port = 0;   // the port of the current client, initalized when MSG_UP returns
int isRunning = 1; // for LIVEFEED exit

// temporary struct that saves new message on each channel

// struct Semaphore
// {
// 	int value;

// 	// q contains all Process Control Blocks(PCBs)
// 	// corresponding to processes got blocked
// 	// while performing down operation.
// 	Queue<process> q;

// }



// int getCount(struct note_t *head)
// {
// 	int count = 0;
// 	node_t *current = head;
// 	while (current != NULL)
// 	{
// 		count++;
// 		current = current->next;
// 	}
// 	return count;
// }

// void *creat_share_memory(size_t size = 1000 * 1024, key_t key = 5555, struct node_t *head)
// {
// 	char c;
// 	int shmid;
// 	node_t *shm, *s;
// 	shm = &head;

// 	//create the segment

// 	if ((shmid = shmget(key, size, IPC_CREAT | 0666)) < 0)
// 	{
// 		perror("shmget");
// 		exit(1);
// 	}

//   if((shm = shmat(shmid,NULL,0) == (char *) -1
//     {
// 		perror("shmat");
// 		exit(1);
//     }
//       // Put somthing to read
//       s = shm;

//       while(head.next != null)
// 	{
// 		*s++ = head;
// 		head = head.next;
// 	}
//       *s = 0;
// }

// fork_somthing()
// {
// 	//fork somthing that divided into two or more

// 	if (fork() == 0)
// 		printf("Hello from Child!\n");
// 	//Do somthing that on real work on server-client service

// 	// parent process because return value non-zero.
// 	else{
// 		printf("Hello from Parent!\n");
// 	// if the client number is not enought , fork more times
// 	fork_somthing();
// 	wait(NULL);
// 	print("All child process complete");
// 	}
// 	return 0;
// }

// read_data_form_share_memory(Semaphore s)
// {
// 	s.value = s.value - 1;
// 	if (s.value < 0)
// 	{

// 		// add process to queue
// 		// here p is a process which is currently executing
// 		q.push(p);
// 		block();
// 	}
// 	else
// 		return;
// }

// write_data_form_share_memory(Semaphore s)
// {
// 	s.value = s.value - 1;
// 	if (s.value < 0)
// 	{

// 		// add process to queue
// 		// here p is a process which is currently executing
// 		q.push(p);
// 		block();
// 	}
// 	else
// 		return;
// }

// complete_write_data_form_share_memory(Semaphore s)
// {
// 	s.value = s.value + 1;
// 	if (s.value <= 0)
// 	{

// 		// remove process p from queue
// 		q.pop();
// 		wakeup(p);
// 	}
// 	else
// 		return;
// }

// complete_read_data_form_share_memory(Semaphore s)
// {
// 	s.value = s.value + 1;
// 	if (s.value <= 0)
// 	{

// 		// remove process p from queue
// 		q.pop();
// 		wakeup(p);
// 	}
// 	else
// 		return;
// }

char *ReadNextMess(htab_t *hClient, htab_t *hChannel, char *clientID, int channelID)
{
	char *mess = (char *)malloc(sizeof(char) * 1024);
	int length = snprintf(NULL, 0, "%d", channelID);
	char *str2 = malloc(length + 1);
	snprintf(str2, length + 1, "%d", channelID);

	// size_t bucket = htab_index(hClient, clientID);
	node_t *channelFound = node_find_channel(hClient, clientID, channelID);
	int messID = channelFound->messIndex;
	//printf("ID = %d\n",messID);
	//Message queue
	channel_t *messageList = htab_find(hChannel, str2)->messList;
	message_t *messageQueue = messageList->message;
	for (; messageQueue != NULL; messageQueue = messageQueue->next)
	{
		if (messageQueue->messIndex == messID)
		{
			if (messageQueue->text != NULL)
			{
				//printf("%s\n", mess);
				sprintf(mess,"%s",messageQueue->text);
				messageList->countRead++;
				messageList->countUnread--;
				channelFound->messIndex++;
				//free(mess);
			}
		}
	}
	return mess;
}

char *ReadAllMess(htab_t *hClient, htab_t *hChannel, char *clientID)
{
	char *result = (char *)malloc(sizeof(char) * 1024);
	char *mess = (char *)malloc(sizeof(char) * 1024);
	// node_t* channels = htab_find(hClient,clientID)->subChannel;

	if (htab_find(hClient, clientID) == NULL)
	{
		printf("Not subscribed to any channels\n");
	}
	else
	{

		node_t *channels = htab_find(hClient, clientID)->subChannel;
		for (; channels != NULL; channels = channels->next)
		{
			int channelID = channels->channelID;
			int length = snprintf(NULL, 0, "%d", channelID);
			char *str2 = malloc(length + 1);
			snprintf(str2, length + 1, "%d", channelID);

			// size_t bucket = htab_index(hClient, clientID);
			node_t *channelFound = node_find_channel(hClient, clientID, channelID);
			int messID = channelFound->messIndex;
			//printf("ID = %d\n",messID);
			//Message queue
			channel_t *messageList = htab_find(hChannel, str2)->messList;
			message_t *i = messageList->message;
			if (i == NULL)
			{
				continue;
			}
			else
			{
				for (message_t *i = messageList->message; i != NULL; i = i->next)
				{
					if (i->messIndex == messID)
					{
						if (i->text != NULL)
						{
							sprintf(mess,"%d:%s\n",channelID,i->text);
							strcat(result,mess);
							//printf("%d:%s\n", channelID, mess);
							messageList->countRead++;
							messageList->countUnread--;
							channelFound->messIndex++;
							messID++;
							//free(mess);
							// return;
						}
					}
				}
			}
		}
	}
	free(mess);
	return result;
}

/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

int main(int argc, char *argv[])
{
	int socket_fd, new_socket_fd;
	struct sockaddr_in address;
	pthread_attr_t pthread_attr;
	pthread_arg_t *pthread_arg;
	pthread_t pthread;
	socklen_t client_address_len;
	//Hash tables
	htab_t hClient;
	htab_t hChannel;
	int buckets = 5;
	//Initialize client hashtable
	if (!htab_init(&hClient, buckets))
	{
		printf("failed to initialise hash table\n");
		return EXIT_FAILURE;
	}

	//Initialize channel hashtable
	if (!htab_init(&hChannel, buckets))
	{
		printf("failed to initialise hash table\n");
		return EXIT_FAILURE;
	}
	/* Get port from command line arguments or stdin. */
	if (argc != 2)
	{
		fprintf(stderr, "usage: client port_number\n");
		exit(1);
	}

	/* Initialise IPv4 address. */
	memset(&address, 0, sizeof address);
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(argv[1]));
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

	/* Initialise pthread attribute to create detached threads. */
	if (pthread_attr_init(&pthread_attr) != 0)
	{
		perror("pthread_attr_init");
		exit(1);
	}
	if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0)
	{
		perror("pthread_attr_setdetachstate");
		exit(1);
	}

	while (1)
	{
		/* Create pthread argument for each connection to client. */
		/* TODO: malloc'ing before accepting a connection causes only one small
	 * memory when the program exits. It can be safely ignored.
	 */
		pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
		if (!pthread_arg)
		{
			perror("malloc");
			continue;
		}

		/* Accept connection to client. */
		client_address_len = sizeof pthread_arg->client_address;
		new_socket_fd = accept(socket_fd, (struct sockaddr *)&pthread_arg->client_address, &client_address_len);
		if (new_socket_fd == -1)
		{
			perror("accept");
			free(pthread_arg);
			continue;
		}
		puts("Connection accepted");

		/* Initialise pthread argument. */
		pthread_arg->new_socket_fd = new_socket_fd;
		pthread_arg->hChannel = hChannel;
		pthread_arg->hClient = hClient;
		pthread_arg->str = str;
		/* TODO: Initialise arguments passed to threads here. See lines 22 and
	 * 139.
	 */

		/* Create thread to serve connection to client. */
		if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0)
		{
			perror("pthread_create");
			free(pthread_arg);
			continue;
		}
	}

	close(socket_fd);
	/*
       * TODO: If you really want to close the socket, you would do it in
       * signal_handler(), meaning socket_fd would need to be a global variable.
       */
	return 0;
}

void *pthread_routine(void *arg)
{
	pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
	int new_socket_fd = pthread_arg->new_socket_fd;
	htab_t hClient = pthread_arg->hClient;
	htab_t hChannel = pthread_arg->hChannel;
	char *str = pthread_arg->str;
	struct sockaddr_in client_address = pthread_arg->client_address;
	/* TODO: Get arguments passed to threads here. See lines 22 and 116. */

	free(arg);

	/* TODO: Put client interaction code here. For example, use
       * write(new_socket_fd,,) and read(new_socket_fd,,) to send and receive
       * messages with the client.
       */
	char messages[MAX];
	int channel_id;
	

	// infinite loop for chat
	for (;;)
	{
		bzero(messages, MAX);
		read(new_socket_fd, messages, sizeof(messages));
		if (strcmp(messages, "SUB") == 0)
		{
			bzero(messages, MAX);
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id >= 0 && channel_id <= 255 && (node_find_channel(&hClient, str, channel_id) == NULL))
				{
					htab_add_node(&hClient,&hChannel, str, channel_id);
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
			bzero(messages, MAX);
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
			bzero(messages, MAX);
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id >= 0 && channel_id <= 255 && read(new_socket_fd, send_message, 1024) != -1)
				{
					int length = snprintf(NULL, 0, "%d", channel_id);
					char *str2 = malloc(length + 1);
					snprintf(str2, length + 1, "%d", channel_id);
					//Add message to channel(str2), hashtable hChannel
					htab_add_mess(&hChannel, str2, send_message);
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
			bzero(messages, MAX);
			printf("Found\n");
			if (read(new_socket_fd, &channel_id, sizeof(channel_id)) != -1)
			{
				printf("%d\n", channel_id);
				if (channel_id == 265)
				{
					sprintf(messages, "%s",ReadAllMess(&hClient,&hChannel,str));
				}
				else if(channel_id >= 0 && channel_id <= 255 && node_find_channel(&hClient,str, channel_id) != NULL)
				{
					sprintf(messages, "%s\n", ReadNextMess(&hClient,&hChannel,str,channel_id));

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
		}
		else if (strcmp(messages, "CHANNELS") == 0)
		{
		}
		else
		{
			bzero(messages, MAX);
			sprintf(messages, "Cannot recognise your command\n");
		}
		write(new_socket_fd, messages, 1024);
	}
	close(new_socket_fd);
	return NULL;
}

void signal_handler(int signal_number)
{
	/* TODO: Put exit cleanup code here. */
	exit(0);
}
