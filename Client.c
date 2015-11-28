/* to compile me in Linux, type:   gcc -o Client Client.c -lpthread */
/* then to run me in Linux, type:  ./Client                         */

/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 128

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
	
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char *localhost = "127.0.0.1";
    char buf[BUFSIZE];
    
    int repeat = 0;
    

    /* check command line arguments */
    if (argc != 4 && argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port> <msg>\nor usage: %s <hostname> <port>", argv[0], argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");
      
    printf("Host %s connecting to %s\n", localhost, hostname); // JASON: Step1

	/* JASON: Step 2 */
    /* get message line from the user or through arg[3] */
    if (argc == 3){
      repeat = 1;	//tells the program to loop
	  printf("Please enter the character messages to be checked (CTRL-Z to exit): ");

      bzero(buf, BUFSIZE);
      fgets(buf, BUFSIZE, stdin);
      
      char *pos;  //this gets rid of the newline character
      if ((pos = strchr(buf, '\n')) != NULL)
        *pos = '\0';
        
	}
    else if (strncpy(buf, argv[3], BUFSIZE) < 0)
      error("ERROR line 67");


	/*JASON: Step 3 */
    /* send the message line to the server */
    n = write(sockfd, buf, strlen(buf));
    if (n < 0) 
      error("ERROR writing to socket");

    /* print the server's reply */
    bzero(buf, BUFSIZE);
    n = read(sockfd, buf, BUFSIZE);
    if (n < 0) 
      error("ERROR reading from socket");
    printf("Echo from server: %s\n", buf);
    close(sockfd);
    if(repeat)
      main(argc, argv);
}
