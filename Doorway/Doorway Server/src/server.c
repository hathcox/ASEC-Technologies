#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <mysql.h>
#include <openssl/md5.h>

#define PORT "9034"   // port we're listening on

#define TRUE 1
#define FALSE 0

#define NAME_SIZE 64
#define PASSWORD_SIZE 64
#define AUTH_TOKEN "999999"
#define AUTH_TOKEN_SIZE 7
#define FAIL_TOKEN "000000"

MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;
char *server = "localhost";
char *user = "root";
char *password = "1234"; /* set me first */
char *database = "doorway";

/* Socket Stuffs */
fd_set master; // master file descriptor list
fd_set read_fds; // temp file descriptor list for select()
int fdmax; // maximum file descriptor number

int listener; // listening socket descriptor
int newfd; // newly accept()ed socket descriptor
struct sockaddr_storage remoteaddr; // client address
socklen_t addrlen;

char buf[256]; // buffer for client data
int nbytes;

char remoteIP[INET6_ADDRSTRLEN];

int yes = 1; // for setsockopt() SO_REUSEADDR, below
int i, j, rv;

struct addrinfo hints, *ai, *p;



void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

void print_hash(unsigned char hash[])
{
   int idx;
   for (idx=0; idx < 32; idx++)
      printf("%02x",hash[idx]);
   printf("\n");
}

int loadPassword(int index,char* passwordBuffer,char* message) {
	int counter = 0;
	while(index < index+65) {
		// we user | to mark the end of a section
		if(message[index] != (char) 0x7C) {
			passwordBuffer[counter] = message[index];
		} else {
			passwordBuffer[counter] = (char) 0x00;
			return index+1;
		}
		index++;
		counter++;
	}
	passwordBuffer[64] = (char) 0x00;
	return index;
}

int loadCharacterName(int index, char* passwordBuffer, char* message) {
	int counter = 0;
	while(index < index+65) {
		// we user | to mark the end of a section
		if(message[index] != (char) 0x7C) {
			passwordBuffer[counter] = message[index];
		} else {
			passwordBuffer[counter] = (char) 0x00;
			return index+1;
		}
		index++;
		counter++;
	}
	passwordBuffer[64] = (char) 0x00;
	return index;
}

int loadAuthToken(int index, char* tokenBuffer, char* message) {
	int counter = 0;
	while(index < index+8) {
		// we use | to mark the end of a section
		if(message[index] != (char) 0x7C) {
			tokenBuffer[counter] = message[index];
		} else {
			tokenBuffer[counter] = (char) 0x00;
			return index+1;
		}
		index++;
		counter++;
	}
	tokenBuffer[8] = (char) 0x00;
	return index;
}

int loadName(char* nameBuffer, char* message) {
	int index=1;
	while(index < 65) {
		// we user | to mark the end of a section
		if(message[index] != (char) 0x7C) {
			nameBuffer[index-1] = message[index];
		} else {
			nameBuffer[index-1] = (char) 0x00;
			return index+1;
		}
		index++;
	}
	nameBuffer[64] = (char) 0x00;
	return index;
}

void sendAuthToken() {
	if (FD_ISSET(i, &master)) {
		if (send(i, AUTH_TOKEN, AUTH_TOKEN_SIZE, 0) == -1) {
			perror("send");
		}
	}
}

void sendFailToken() {
	if (FD_ISSET(i, &master)) {
		if (send(i, FAIL_TOKEN, AUTH_TOKEN_SIZE, 0) == -1) {
			perror("send");
		}
	}
}

int validateAuthToken(char* tokenBuffer) {
	int index;
	for(index=0; index < 7; index++) {
		if (tokenBuffer[index] != (char) 0x39) {
			return FALSE;
		}
	}
	if (strlen(tokenBuffer) == 7) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int checkHash(char* password, char* salt, char* attempt) {

	int length = strlen(attempt) + strlen(salt);
	char* passwordSaltBuffer [length];

	//password + salt
	strcpy(passwordSaltBuffer, attempt);
	strcat(passwordSaltBuffer, salt);

	unsigned char result [MD5_DIGEST_LENGTH];
	MD5(passwordSaltBuffer, strlen(passwordSaltBuffer), result);

	//Turn all of the hex into an ascii version of MD5
	int i;
	char asciiResult [(MD5_DIGEST_LENGTH*2)+1];
	char tempBuffer [2];
	for(i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(tempBuffer, "%02x", result[i]);
		asciiResult[i*2] = tempBuffer[0];
		asciiResult[(i*2)+1] = tempBuffer[1];
	}
	//Add a null terminator
	asciiResult[32] = (char) 0x00;

	//If the password is the same as the md5
	if(strcmp(asciiResult, password) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void setCharacterName(char* original, char* username) {
	/* send SQL query */
	startConnection();

	char* nameBuffer [NAME_SIZE];

	sprintf(nameBuffer, "update user set character_name = \"%s\" where _user_name = \"%s\" ", original, username);

	if (mysql_query(conn, nameBuffer)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
}

//This is load the message of the wire and attempt to respond to correct message
void parseMessage(char* message) {
/*
	If (message = auth user):
		Attempt to validate user and password
		If (valid user):
			Return auth token
	If (message = Set name):
		If (auth token is valid):
			Stack overflow load name
			Return World server down
		Return None
*/
	//Auth user
	if (strlen(message) < 5) {
		printf("Invalid Message\n");
		return;
	}

	if(message[0] == (char) 0x41) {
		printf("A\n");
		char* nameBuffer [NAME_SIZE+1];
		char* passwordBuffer [PASSWORD_SIZE+1];

		int index = loadName(nameBuffer, message);
		index = loadPassword(index, passwordBuffer, message);

		int result = validateUser(nameBuffer, passwordBuffer);
		//Valid auth
		if(result) {
			printf("Valid Login, returning token\n");
			sendAuthToken();
		} else {
			printf("Invalid Login Attempt!\n");
			sendFailToken();
		}

	}
	//Set name
	if(message[0] == (char) 0x42) {
		printf("B\n");
		char* nameBuffer [NAME_SIZE+1];
		char* authTokenBuffer [AUTH_TOKEN_SIZE+1];
		char* characterNameBuffer [NAME_SIZE+1];

		int index = loadName(nameBuffer, message);
		index = loadAuthToken(index, authTokenBuffer, message);
		index = loadCharacterName(index, characterNameBuffer, message);

		printf("token name :%s\n", authTokenBuffer);

		if (validateAuthToken(authTokenBuffer)) {
			printf("Valid Auth Token\n");
			setCharacterName(characterNameBuffer, nameBuffer);
			sendAuthToken();
		} else {
			printf("Invalid Auth Token!\n");
			sendFailToken();
		}

	}
//	printf("%c\n", message[0]);



}

void startConnection() {
	conn = mysql_init(NULL);
	/* Connect to database */
	if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
}

void stopConnection() {
	/* close connection */
	mysql_free_result(res);
	mysql_close(conn);
}

int validateUser(char* username, char* password) {
	/* send SQL query */
	startConnection();

	char* nameBuffer [NAME_SIZE];

	sprintf(nameBuffer, "select * from user where _user_name = \"%s\"", username);

	if (mysql_query(conn, nameBuffer)) {
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	res = mysql_use_result(conn);
	row = mysql_fetch_row(res);

	if(checkHash(row[5], row[4], password)) {
		stopConnection();
		return TRUE;
	} else {
		stopConnection();
		return FALSE;
	}
}



int main(void) {

	FD_ZERO(&master); // clear the master and temp sets
	FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}

		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(3);
	}

	// add the listener to the master set
	FD_SET(listener, &master);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	printf("Server Startup!\n");

	// main loop
	for (;;) {
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
				if (i == listener) {
					// handle new connections
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr *) &remoteaddr,
							&addrlen);

					if (newfd == -1) {
						perror("accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) { // keep track of the max
							fdmax = newfd;
						}
						printf(
								"selectserver: new connection from %s on "
									"socket %d\n",
								inet_ntop(
										remoteaddr.ss_family,
										get_in_addr(
												(struct sockaddr*) &remoteaddr),
										remoteIP, INET6_ADDRSTRLEN), newfd);
					}
				} else {
					// handle data from a client
					if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0) {
							// connection closed
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
					} else {
						// we got some data from a client
						parseMessage(buf);
//						for (j = 0; j <= fdmax; j++) {
//							// send to everyone!
//							if (FD_ISSET(j, &master)) {
//								// except the listener and ourselves
//								if (j != listener && j != i) {
//									if (send(j, buf, nbytes, 0) == -1) {
//										perror("send");
//									}
//								}
//							}
//						}
					}
				} // END handle data from client
			} // END got new incoming connection
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!

	return 0;
}
