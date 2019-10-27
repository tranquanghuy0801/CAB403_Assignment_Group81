/** CAB403 - Systems Programming 
 *  Group 81 - DS Assignment 
 * 	Quang Huy Tran - n10069275 
 *  Tuan Minh Nguyen - 
 *  Ho Fong Law - n1010321
 * */

#include <arpa/inet.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
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
		perror("gethostbyname");
		exit(1);
	}

	/* Initialise IPv4 server address with server host. */
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[2]));
	server_address.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_address.sin_zero),8); 

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
	char buff[MAX];
	char messages[MAX];
	int channel_id;
	for(;;) { 
		memset(buff,'\0',sizeof(buff));
		memset(messages,'\0',sizeof(messages)); 
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
		if(strcmp(messages,"SUB") == 0 || strcmp(messages,"UNSUB") == 0 || strcmp(messages,"NEXT") == 0 || strcmp(messages,"LIVEFEED") == 0 || strcmp(messages,"SEND") == 0 || strcmp(messages,"CHANNELS") == 0){
			fgets(buff,MAX,stdin);
			if (sscanf(buff,"%d",&channel_id) == 1)
			{
				printf("%d\n",channel_id);
				write(socket_fd,&channel_id,sizeof(channel_id));
				if(strcmp(messages,"SEND") == 0){
					char *send_message = (char *)malloc(sizeof(char) * 1024);
					if(sscanf(buff,"%[^\n]%*c", send_message) == 1){
						send_message+=3;
						printf("%s\n",send_message);
						write(socket_fd,send_message,1024);
					}
				}
			}
			else{
				channel_id = 265;
				if(strcmp(messages,"CHANNELS") == 0){
					printf("Channel  Total  Read Unread\n");
				}
				write(socket_fd,&channel_id,sizeof(channel_id));
			}
		}
		if ((strncmp(messages,"BYE", 4)) == 0) { 
			printf("Client Exit...\n");
			close(socket_fd); 
			return 0;
		} 
		memset(buff,'\0',sizeof(buff));
		memset(messages,'\0',sizeof(messages));
		read(socket_fd, messages, 1024); 
		printf("%s", messages); 
	} 


	close(socket_fd);
	return 0;
}
