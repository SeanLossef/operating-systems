#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#define MAXCLIENTS 32
#define MAXMESSAGE 990
#define MAXLENGTH 1200
#define MAXUSERNAME 16
#define MAX(a,b) (((a)>(b))?(a):(b))

struct Chat {
	int clientsock[MAXCLIENTS];
	char userids[MAXCLIENTS][MAXUSERNAME];
	int client_i;
};

int hasUserId(struct Chat *chat, char *userid) {
	for (int i = 0; i < MAXCLIENTS; i++)
		if (strncmp(chat->userids[i], userid, MAXUSERNAME) == 0)
			return 1;
	return 0;
}

static int compareStrings(const void* a, const void* b) { 
	return strcmp(*(const char**)a, *(const char**)b); 
}

void* tcpHandler(void *arg) {
	struct Chat *chat = *((struct Chat**)arg);
	char buffer[MAXLENGTH];
	char original[MAXLENGTH];
	char delim[] = " ";
	int n;
	int client_i = chat->client_i;
	int connfd = chat->clientsock[client_i];

	while (1) {
		bzero(buffer, MAXLENGTH);

		if ((n = read(connfd, buffer, sizeof(buffer))) > 0) {
			strcpy(original, buffer);
			if (strchr(buffer, '\n'))
				buffer[strlen(buffer) - 1] = '\0';
			char *ptr = strtok(buffer, " ");

			// LOGIN Command
			if (strcmp(ptr, "LOGIN") == 0) {
				ptr = strtok(NULL, delim);
				char *userid = calloc(1, strlen(ptr) + 1);
				strcpy(userid, ptr);
				printf("CHILD %ld: Rcvd LOGIN request for userid %s\n", pthread_self(), userid);

				if (strlen(userid) < 4 || strlen(userid) > MAXUSERNAME) {
					sprintf(buffer, "ERROR Invalid userid\n");
					write(connfd, buffer, strlen(buffer));
					printf("CHILD %ld: Sent ERROR (Invalid userid)\n", pthread_self());
				} else if (hasUserId(chat, userid)) {
					sprintf(buffer, "ERROR Already connected\n");
					write(connfd, buffer, strlen(buffer));
					printf("CHILD %ld: Sent ERROR (Already connected)\n", pthread_self());
				} else {
					strcpy(chat->userids[client_i], userid);
					sprintf(buffer, "OK!\n");
					write(connfd, buffer, strlen(buffer));
				}

				free(userid);
				continue;
			}

			// SEND Command
			if (strcmp(ptr, "SEND") == 0) {
				ptr = strtok(NULL, delim);
				char *userid = calloc(1, strlen(ptr) + 1);
				strcpy(userid, ptr);

				ptr = strtok(NULL, delim);
				int msglen = atoi(ptr);

				printf("CHILD %ld: Rcvd SEND request to userid %s\n", pthread_self(), userid);

				if (msglen < 1 || msglen > MAXMESSAGE) {
					sprintf(buffer, "ERROR Invalid msglen\n");
					write(connfd, buffer, strlen(buffer));
					printf("CHILD %ld: Sent ERROR (Invalid msglen)\n", pthread_self());
				} else {
					char *ptr2 = strtok(original, "\n");
					ptr2 = strtok(NULL, "\n");

					int found = -1;
					for (int i = 0; i < MAXCLIENTS; i++) {
						if (strcmp(chat->userids[i], userid) == 0) {
							found = i;
						}
					}

					if (found == -1) {
						sprintf(buffer, "ERROR Unknown userid\n");
						write(connfd, buffer, strlen(buffer));
						printf("CHILD %ld: Sent ERROR (Unknown userid)\n", pthread_self());
					} else {
						sprintf(buffer, "OK!\n");
						write(connfd, buffer, strlen(buffer));

						sprintf(buffer, "FROM %s %d %s\n", chat->userids[client_i], msglen, ptr2);
						write(chat->clientsock[found], buffer, strlen(buffer));
					}
				}

				free(userid);
				continue;
			}

			// BROADCAST Command
			if (strcmp(ptr, "BROADCAST") == 0) {
				ptr = strtok(NULL, delim);
				int msglen = atoi(ptr);

				printf("CHILD %ld: Rcvd BROADCAST request\n", pthread_self());

				if (msglen < 1 || msglen > MAXMESSAGE) {
					sprintf(buffer, "ERROR Invalid msglen\n");
					write(connfd, buffer, strlen(buffer));
					printf("CHILD %ld: Sent ERROR (Invalid msglen)\n", pthread_self());
				} else {
					char *ptr2 = strtok(original, "\n");
					ptr2 = strtok(NULL, "\n");

					sprintf(buffer, "OK!\n");
					write(connfd, buffer, strlen(buffer));
					sprintf(buffer, "FROM %s %d %s\n", chat->userids[client_i], msglen, ptr2);

					for (int i = 0; i < MAXCLIENTS; i++) {
						if (strlen(chat->userids[i]) > 0) {
							write(chat->clientsock[i], buffer, strlen(buffer));
						}
					}
				}

				continue;
			}

			// WHO Command
			if (strcmp(ptr, "WHO") == 0) {
				sprintf(buffer, "OK!\n");
				
				char *sorted[MAXCLIENTS];
				for (int i = 0; i < MAXCLIENTS; i++) {
					if (strcmp(chat->userids[i], "") != 0) {
						sorted[i] = calloc(1, strlen(chat->userids[i]) + 1);
						strcpy(sorted[i], chat->userids[i]);
					} else
						sorted[i] = malloc(0);
				}
				qsort(sorted, MAXCLIENTS, sizeof(const char*), compareStrings);

				for (int i = 0; i < MAXCLIENTS; i++)
					if (strcmp(sorted[i], "") != 0)
						sprintf(buffer, "%s%s\n", buffer, sorted[i]);
				write(connfd, buffer, strlen(buffer));
				printf("CHILD %ld: Rcvd WHO request\n", pthread_self());
				continue;
			}

			// LOGOUT Command
			if (strcmp(ptr, "LOGOUT") == 0) {
				chat->userids[client_i][0] = '\0';
				sprintf(buffer, "OK!\n");
				write(connfd, buffer, strlen(buffer));
				printf("CHILD %ld: Rcvd LOGOUT request\n", pthread_self());
				continue;
			}

		} else {
			printf("CHILD %ld: Client disconnected\n", pthread_self());
			close(connfd);
			chat->clientsock[client_i] = 0;
			bzero(chat->userids[client_i], MAXUSERNAME);
			pthread_exit(NULL);
			return NULL;
		}
	}
}

int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Invalid command line arguments\n");
		return EXIT_FAILURE;
	}

	setvbuf( stdout, NULL, _IONBF, 0 );

	////// Open control server socket //////
	int sockfd1, sockfd2;
	struct sockaddr_in servaddr1, servaddr2, cliaddr;
	uint16_t port;
	sscanf(argv[1], "%hu", &port);

	bzero((char *) &servaddr1, sizeof(servaddr1));
	bzero((char *) &servaddr2, sizeof(servaddr2));

	sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
	sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);

	int optval = 1;
	setsockopt(sockfd1, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
	setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

	servaddr1.sin_family = AF_INET;
	servaddr1.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr1.sin_port = htons(port);

	servaddr2.sin_family = AF_INET;
	servaddr2.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr2.sin_port = htons(port);

	bind(sockfd1, (struct sockaddr *) &servaddr1, sizeof(servaddr1));
	bind(sockfd2, (struct sockaddr *) &servaddr2, sizeof(servaddr2));
	listen(sockfd1, 5);

	// Setup select to listen for commands from both stdin and the server
	char buffer[MAXLENGTH];
	char original[MAXLENGTH];

	fd_set master;
	fd_set read_fds;

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	FD_SET(sockfd1, &master);
	FD_SET(sockfd2, &master);

	struct Chat *chat = calloc(1, sizeof(struct Chat));

	printf("MAIN: Started server\n");
	printf("MAIN: Listening for TCP connections on port: %d\n", port);
	printf("MAIN: Listening for UDP datagrams on port: %d\n", port);

	//////// Accept incoming connections and create new thread ///////
	while (1) {
		read_fds = master;
		if (select(sockfd2 + 1, &read_fds, NULL, NULL, NULL) == -1) {
			return EXIT_FAILURE;
		}

		bzero(buffer, MAXLENGTH);
		bzero(original, MAXLENGTH);


		// UDP Socket
		if (FD_ISSET(sockfd2, &read_fds)) {
			socklen_t len = sizeof(cliaddr);
			recvfrom(sockfd2, (char *)buffer, MAXLENGTH, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);

			strcpy(original, buffer);

			if (strchr(buffer, '\n'))
				buffer[strlen(buffer) - 1] = '\0';

			printf("MAIN: Rcvd incoming UDP datagram from: %s\n", inet_ntoa(cliaddr.sin_addr));

			char *ptr = strtok(buffer, " ");

			// WHO Command
			if (strcmp(ptr, "WHO") == 0) {
				sprintf(buffer, "OK!\n");
				
				char *sorted[MAXCLIENTS];
				for (int i = 0; i < MAXCLIENTS; i++) {
					if (strcmp(chat->userids[i], "") != 0) {
						sorted[i] = calloc(1, strlen(chat->userids[i]) + 1);
						strcpy(sorted[i], chat->userids[i]);
					} else
						sorted[i] = malloc(0);
				}
				qsort(sorted, MAXCLIENTS, sizeof(const char*), compareStrings);

				for (int i = 0; i < MAXCLIENTS; i++)
					if (strcmp(sorted[i], "") != 0)
						sprintf(buffer, "%s%s\n", buffer, sorted[i]);
				sendto(sockfd2, (const char *)buffer, strlen(buffer), 2048, (const struct sockaddr*) &cliaddr, len);
				printf("MAIN: Rcvd WHO request\n");
				continue;
			}

			// BROADCAST Command
			if (strcmp(ptr, "BROADCAST") == 0) {
				ptr = strtok(NULL, " ");
				int msglen = atoi(ptr);

				printf("MAIN: Rcvd BROADCAST request\n");

				if (msglen < 1 || msglen > MAXMESSAGE) {
					sprintf(buffer, "ERROR Invalid msglen\n");
					sendto(sockfd2, (const char *)buffer, strlen(buffer), 2048, (const struct sockaddr*) &cliaddr, len);
					printf("MAIN: Sent ERROR (Invalid msglen)\n");
				} else {
					char *ptr2 = strtok(original, "\n");
					ptr2 = strtok(NULL, "\n");

					char buffer[MAXLENGTH];
					sprintf(buffer, "OK!\n");
					sendto(sockfd2, (const char *)buffer, strlen(buffer), 2048, (const struct sockaddr*) &cliaddr, len);
					sprintf(buffer, "FROM UDP-client %d %s\n", msglen, ptr2);

					for (int i = 0; i < MAXCLIENTS; i++) {
						if (strlen(chat->userids[i]) > 0) {
							write(chat->clientsock[i], buffer, strlen(buffer));
						}
					}
				}
				continue;
			}

			// SEND Command
			if (strcmp(ptr, "SEND") == 0) {
				sprintf(buffer, "SEND not supported over UDP\n");
				sendto(sockfd2, (const char *)buffer, strlen(buffer), 2048, (const struct sockaddr*) &cliaddr, len);
				printf("MAIN: Sent ERROR (SEND not supported over UDP)\n");
			}
		}

		// TCP Socket
		if (FD_ISSET(sockfd1, &read_fds)) {
			pthread_t tid;
			socklen_t len = sizeof(cliaddr);
			int connfd = accept(sockfd1, (struct sockaddr *)&cliaddr, &len);

			if (connfd < 0) {
				fprintf(stderr, "Accept error errno=%d\n", errno);
				continue;
			}

			printf("MAIN: Rcvd incoming TCP connection from: %s\n", inet_ntoa(cliaddr.sin_addr));

			int i = 0;
			for (; i < MAXCLIENTS; i++) {
				if (chat->clientsock[i] == 0) {
					chat->clientsock[i] = connfd;
					break;
				}
			}
			chat->client_i = i;

			pthread_create(&tid, NULL, tcpHandler, &chat);
		}

	}
	
	close(sockfd1);
	free(chat);

	return EXIT_SUCCESS;
}