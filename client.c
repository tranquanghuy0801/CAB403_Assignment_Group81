/** CAB403 - Systems Programming 
 *  Group 81 - DS Assignment 
 * 	Quang Huy Tran - n10069275 
 *  Tuan Minh Nguyen - 
 *  Ho Fong Law - 
 * */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX 1024

int main(int argc, char *argv[]) {
	int socket_fd;
	struct hostent *server_host;
	struct sockaddr_in server_address;
	if (argc != 3)
	{
		fprintf(stderr, "usage: client_hostname port_number\n");
		exit(1);
	}

	if ((server_host = gethostbyname(argv[1])) == NULL)
	{ /* get the host info */
		herror("gethostbyname");
		exit(1);
	}

	/* Initialise IPv4 server address with server host. */
	memset(&server_address, 0, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[2]));
	memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

	/* Create TCP socket. */
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* Connect to socket with server address. */
	if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
		perror("connect");
		exit(1);
	}

	/* TODO: Put server interaction code here. For example, use
	 * write(socket_fd,,) and read(socket_fd,,) to send and receive messages
	 * with the client.
	 */
	char messages[MAX];
	int channel_id;
	for(;;) { 
		bzero(messages, sizeof(messages)); 
		printf("Commands: \n\
			SUB <channelID> -- to subscribe to channel (0-255) \n\
			UNSUB <channelID> - to unsubscribe to channel (0-255) \n\
			SEND <channelID> <message> - to send the message to a given channel \n\
			NEXT <channelID> - to display the next unread message on a given channel\n\
			NEXT - display the next unread messages on all subscribed channles\n\
			LIVEFEED <channelID> - to display messages continuously in a given channel \n\
			LIVEFEED - to display messages continously across all channels\n\
			BYE - to exit the connection with server \n\n:");
		scanf("%s",messages);
		write(socket_fd, messages, sizeof(messages)); 
		if(strcmp(messages,"SUB") == 0 || strcmp(messages,"UNSUB") == 0 ){
			if (scanf("%d", &channel_id) < 1)
			{
				continue;
			}
			else{
				write(socket_fd, &channel_id, sizeof(channel_id)); 
			}
		}
		if ((strncmp(messages, "BYE", 4)) == 0) { 
			printf("Client Exit...\n");
			close(socket_fd); 
			return 0;
		} 
		bzero(messages, sizeof(messages));
		//n = 0; 
		// while ((buff[n++] = getchar()) != '\n') 
		//      ; 
		// write(socket_fd, buff, sizeof(buff)); 
		// bzero(buff, sizeof(buff)); 
		read(socket_fd, messages, sizeof(messages)); 
		printf("From Server : %s", messages); 
	} 


	close(socket_fd);
	return 0;
}
