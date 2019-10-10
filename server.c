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
#include <unistd.h>
#define MAX 1024 
#define BACKLOG 10

typedef struct message message_t;

// temporary struct that saves new message on each channel
struct message
{
	char *text;
	int read;
	message_t *next;
};

typedef struct node node_t;

// temporary struct the saves info of new channel
typedef struct channel
{
	int channelID;
	int numMessage;
	int next_used;
	message_t *message;
} channel_t;

struct node
{
	channel_t *channel;
	node_t *next;
};

message_t *message_add(message_t *head, char *text)
{
	// create new node to add to list
	message_t *new = (message_t *)malloc(sizeof(message_t));
	if (new == NULL)
	{
		return NULL;
	}

	// insert new node
	new->text = text;
	new->next = head;
	new->read = 0;
	return new;
}

node_t *node_add(node_t *head, channel_t *channel)
{
	// create new node to add to list
	node_t *new = (node_t *)malloc(sizeof(node_t));
	if (new == NULL)
	{
		return NULL;
	}

	// insert new node
	new->channel = channel;
	new->next = head;
	return new;
}

node_t *node_find_channel(node_t *head, int channelID)
{
	for (; head != NULL; head = head->next)
	{
		if (channelID == head->channel->channelID)
		{
			return head;
		}
	}
	return NULL;
}

node_t *node_delete(node_t *head, int channelID)
{
	node_t *previous = NULL;
	node_t *current = head;
	while (current != NULL)
	{
		if (channelID == current->channel->channelID)
		{
			node_t *newhead = head;
			if (previous == NULL) // first item in list
				newhead = current->next;
			else
				previous->next = current->next;
			free(current);
			return newhead;
		}
		previous = current;
		current = current->next;
	}

	// name not found
	return head;
}

void set_read_mess(node_t *head, int channel_id)
{
	node_t *channel_found = node_find_channel(head, channel_id);
	for (; channel_found->channel->message != NULL; channel_found->channel->message = channel_found->channel->message->next)
	{
		channel_found->channel->message->read = 1;
	}
}

char *get_unread_mess(node_t *head)
{
	char *mess = (char *)malloc(sizeof(char) * 1024);
	node_t *channel_found;
	if (head == NULL)
	{
		printf("Not subscrived to any channels\n");
	}
	else
	{
		for (; head != NULL; head = head->next)
		{
			channel_found = head;
			for (; channel_found->channel->message != NULL; channel_found->channel->message = channel_found->channel->message->next)
			{
				if (channel_found->channel->message->read == 0)
				{
					sprintf(mess, "%d:%s\n", channel_found->channel->channelID, channel_found->channel->message->text);
					printf("%s", mess);
					channel_found->channel->message->read = 1;
				}
			}
		}
	}
	return mess;
}

typedef struct pthread_arg_t {
	int new_socket_fd;
	struct sockaddr_in client_address;
	node_t *channel_list;
	node_t *channel_list_unsub;
	/* TODO: Put arguments passed to threads here. See lines 116 and 139. */
} pthread_arg_t;

/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

int main(int argc, char *argv[]) {
	int socket_fd, new_socket_fd;
	struct sockaddr_in address;
	pthread_attr_t pthread_attr;
	pthread_arg_t *pthread_arg;
	pthread_t pthread;
	socklen_t client_address_len;
	node_t *channel_list = NULL;
	node_t *channel_list_unsub = NULL;
	/* Get port from command line arguments or stdin. */
	if(argc != 2){
		fprintf(stderr,"usage: client port_number\n");
		exit(1);
	}

	/* Initialise IPv4 address. */
	memset(&address, 0, sizeof address);
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(argv[1]));
	address.sin_addr.s_addr = INADDR_ANY;

	/* Create TCP socket. */
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	puts("Socket created");

	/* Bind address to socket. */
	if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1) {
		perror("bind");
		exit(1);
	}
	puts("Bind done");

	/* Listen on socket. */
	if (listen(socket_fd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	puts("Waiting for incoming connections...");

	/* Assign signal handlers to signals. */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
	if (signal(SIGTERM, signal_handler) == SIG_ERR) {
		perror("signal");
		exit(1);
	}
	if (signal(SIGINT, signal_handler) == SIG_ERR) {
		perror("signal");
		exit(1);
	}

	/* Initialise pthread attribute to create detached threads. */
	if (pthread_attr_init(&pthread_attr) != 0) {
		perror("pthread_attr_init");
		exit(1);
	}
	if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
		perror("pthread_attr_setdetachstate");
		exit(1);
	}

	while (1) {
		/* Create pthread argument for each connection to client. */
		/* TODO: malloc'ing before accepting a connection causes only one small
		 * memory when the program exits. It can be safely ignored.
		 */
		pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
		if (!pthread_arg) {
			perror("malloc");
			continue;
		}

		/* Accept connection to client. */
		client_address_len = sizeof pthread_arg->client_address;
		new_socket_fd = accept(socket_fd, (struct sockaddr *)&pthread_arg->client_address, &client_address_len);
		if (new_socket_fd == -1) {
			perror("accept");
			free(pthread_arg);
			continue;
		}
		puts("Connection accepted");

		/* Initialise pthread argument. */
		pthread_arg->new_socket_fd = new_socket_fd;
		pthread_arg->channel_list = channel_list;
		pthread_arg->channel_list_unsub = channel_list_unsub;
		/* TODO: Initialise arguments passed to threads here. See lines 22 and
		 * 139.
		 */

		/* Create thread to serve connection to client. */
		if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0) {
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

void *pthread_routine(void *arg) {
	pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
	int new_socket_fd = pthread_arg->new_socket_fd;
	node_t *channel_list = pthread_arg->channel_list;
	node_t *channel_list_unsub = pthread_arg->channel_list_unsub;
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
	for (;;) { 
		bzero(messages,MAX);
		read(new_socket_fd, messages, sizeof(messages));
		if(strcmp(messages,"SUB")==0){
			bzero(messages,MAX);
			printf("Found\n");		
			if(read(new_socket_fd,&channel_id, sizeof(channel_id)) != -1){
				printf("%d\n",channel_id);
				channel_t *channel_new = (channel_t *)malloc(sizeof(channel_t));
				if(channel_id >= 0 && channel_id <= 255 && node_find_channel(channel_list, channel_id) == NULL){
					if(node_find_channel(channel_list_unsub, channel_id) != NULL){
						set_read_mess(channel_list_unsub, channel_id);
						channel_new = node_find_channel(channel_list_unsub, channel_id)->channel;
					}
					else{
					channel_new->channelID = channel_id;
					channel_new->message = NULL;
					channel_new->next_used = 0;
					channel_new->numMessage = 0;
					}
					node_t *newhead = node_add(channel_list, channel_new);
					channel_list = newhead;
					sprintf(messages, "Subscribed to channel %d\n",channel_id );
				}
				else if(channel_id >= 0 && channel_id <= 255 && node_find_channel(channel_list, channel_id) != NULL){
					sprintf(messages, "Already subscribed to channel %d\n",channel_id );
				}
				else
				{
					sprintf(messages,"Invalid channel: %d\n", channel_id);
				}
			} 
			else{
				sprintf(messages,"Miss channel ID\n");
			}

		}
		else if(strcmp(messages,"UNSUB")==0){
			bzero(messages,MAX);
			printf("Found\n");
			if(read(new_socket_fd,&channel_id, sizeof(channel_id)) != -1){
				printf("%d\n",channel_id);
				if(channel_id >= 0 && channel_id <= 255  && node_find_channel(channel_list, channel_id) != NULL){
					node_t *newhead = node_delete(channel_list, channel_id);
					channel_list = newhead;
					sprintf(messages, "Unsubscribed from channel %d\n",channel_id );
				}
				else if(channel_id >= 0 && channel_id <= 255 && node_find_channel(channel_list, channel_id) == NULL){
					sprintf(messages, "Not subscribed from channel %d\n",channel_id);
				}
				else{
					sprintf(messages,"Invalid channel: %d\n\n", channel_id);
				}
			}
			else{
				sprintf(messages,"Miss channel ID\n");
			}
		}
		else if(strcmp(messages,"SEND") == 0){
			char *send_message = malloc(sizeof(char) * 1024);
			bzero(messages,MAX);
			printf("Found\n");
			if(read(new_socket_fd,&channel_id, sizeof(channel_id)) != -1){
				printf("%d\n",channel_id);
				if (channel_id >= 0 && channel_id <= 255 && read(new_socket_fd,send_message, sizeof(send_message)) != -1 )
				{
					if (node_find_channel(channel_list, channel_id) != NULL)
					{
						node_t *channel_send = node_find_channel(channel_list, channel_id);
						message_t *newMess = message_add(channel_send->channel->message, send_message);
						channel_send->channel->message = newMess;
						channel_send->channel->numMessage++;
					}
					else if (node_find_channel(channel_list_unsub, channel_id) != NULL)
					{
						node_t *channel_send = node_find_channel(channel_list_unsub, channel_id);
						message_t *newMess = message_add(channel_send->channel->message, send_message);
						channel_send->channel->message = newMess;
						channel_send->channel->numMessage++;
					}
					else
					{
						channel_t *channel_new = (channel_t *)malloc(sizeof(channel_t));
						message_t *newMess = (message_t *)malloc(sizeof(message_t));
						newMess->text = send_message;
						newMess->next = NULL;
						newMess->read = 0;
						channel_new->channelID = channel_id;
						channel_new->message = newMess;
						channel_new->next_used = 0;
						channel_new->numMessage = 1;
						node_t *newhead = node_add(channel_list_unsub, channel_new);
						channel_list_unsub = newhead;
					}
					sprintf(messages,"Send to channel %d successfully\n", channel_id);
				}
				else
				{
					sprintf(messages,"Invalid channel: %d\n", channel_id);
				}
			}
			else{
				sprintf(messages,"Miss channel ID\n");
			}
		}
		else if(strcmp(messages,"NEXT") == 0){
			bzero(messages,MAX);
			printf("Found\n");
			if(read(new_socket_fd,&channel_id, sizeof(channel_id)) != -1){
				printf("%d\n",channel_id);
				if(channel_id == 265){
					sprintf(messages,"%s",get_unread_mess(channel_list));
				}
				else if(channel_id >= 0 && channel_id <= 255 && node_find_channel(channel_list, channel_id) != NULL){
					node_t *channel_found = node_find_channel(channel_list, channel_id);
					char *mess = (char *)malloc(sizeof(char) * 1024);
					if (channel_found->channel->message != NULL)
					{
						mess = channel_found->channel->message->text;
						channel_found->channel->message->read = 1;
						sprintf(messages,"%s\n", mess);
						channel_found->channel->message = channel_found->channel->message->next;
					}
					else
					{
						sprintf(messages,"No new messages\n");
					}
					free(mess);
				}
				else if (channel_id >= 0 && channel_id <= 255 && node_find_channel(channel_list, channel_id) == NULL)
				{
					sprintf(messages,"Not subscribed to channel %d\n", channel_id);
				}
				else
				{
					sprintf(messages,"Invalid channel: %d\n", channel_id);
				}
			}
		}
		else if(strcmp(messages,"LIVEFEED") == 0){

		}
		else if(strcmp(messages,"CHANNELS") == 0){

		}
		else{
			bzero(messages,MAX);
			sprintf(messages, "Cannot recognise your command\n");
		}
		write(new_socket_fd, messages, sizeof(messages)); 
	} 
	close(new_socket_fd);
	return NULL;
}

void signal_handler(int signal_number) {
	/* TODO: Put exit cleanup code here. */
	exit(0);
}
