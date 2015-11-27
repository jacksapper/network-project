/* to compile me in Linux, type:   gcc -o concurrentserver concurrentserver.c -lpthread */

/* server.c - code for example server program that uses TCP */
/* From Computer Networks and Internets by Douglas F. Comer */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h> //Added to fix implicit definition of exit()

void * serverthread(void * parm);       /* thread function prototype    */

pthread_mutex_t  mut;

#define PROTOPORT         5193          /* default protocol port number */
#define QLEN              6             /* size of request queue        */

int visits =  0;                        /* counts client connections     */

/*************************************************************************
 Program:        concurrent server

 Purpose:        allocate a socket and then repeatedly execute the folllowing:
                          (1) wait for the next connection from a client
                          (2) create a thread to handle the connection
                          (3) go back to step (1)

                 The server thread will
                          (1) update a global variable in a mutex
                          (2) send a short message to the client
                          (3) close the connection

 Syntax:         server [ port ]

                            port  - protocol port number to use

 Note:           The port argument is optional. If no port is specified,
                        the server uses the default given by PROTOPORT.

**************************************************************************
*/

int main (int argc, char *argv[])
{
     struct   hostent   *ptrh;     /* pointer to a host table entry */
     struct   protoent  *ptrp;     /* pointer to a protocol table entry */
     struct   sockaddr_in sad;     /* structure to hold server's address */
     struct   sockaddr_in cad;     /* structure to hold client's address */
     int      sd;             /* socket descriptors */
     uintptr_t sd2;					/*ADDED BY JASON to prevent L 119 unsafe void* cast.  sd2 was previously an int*/
     int      port;                /* protocol port number */
     int      alen;                /* length of address */
     pthread_t  tid;             /* variable to hold thread ID */

     pthread_mutex_init(&mut, NULL);
     memset((char  *)&sad,0,sizeof(sad)); /* clear sockaddr structure   */
     sad.sin_family = AF_INET;            /* set family to Internet     */
     sad.sin_addr.s_addr = INADDR_ANY;    /* set the local IP address */

     /* Check  command-line argument for protocol port and extract      */
     /* port number if one is specfied.  Otherwise, use the default     */
     /* port value given by constant PROTOPORT                          */
    
     if (argc > 1) {                        /* if argument specified     */
                     port = atoi (argv[1]); /* convert argument to binary*/
     } else {
                      port = PROTOPORT;     /* use default port number   */
     }
     if (port > 0)                          /* test for illegal value    */
                      sad.sin_port = htons((u_short)port);
     else {                                /* print error message and exit */
                      fprintf (stderr, "bad port number %s/n",argv[1]);
                      exit (1);
     }

     /* Map TCP transport protocol name to protocol number */
     
     if ( ((ptrp = getprotobyname("tcp"))) == 0)  {
                     fprintf(stderr, "cannot map \"tcp\" to protocol number");
                     exit (1);
     }

     /* Create a socket */
     sd = socket (PF_INET, SOCK_STREAM, ptrp->p_proto);
     if (sd < 0) {
                       fprintf(stderr, "socket creation failed\n");
                       exit(1);
     }

     /* Bind a local address to the socket */
     if (bind(sd, (struct sockaddr *)&sad, sizeof (sad)) < 0) {
                        fprintf(stderr,"bind failed\n");
                        exit(1);
     }

     /* Specify a size of request queue */
     if (listen(sd, QLEN) < 0) {
                        fprintf(stderr,"listen failed\n");
                         exit(1);
     }

     alen = sizeof(cad);

     /* Main server loop - accept and handle requests */
     fprintf( stderr, "Server up and running.\n");
     while (1) {

         printf("SERVER: Waiting for contact ...\n");
         
         if (  (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
	                      fprintf(stderr, "accept failed\n");
                              exit (1);
	 }
	 pthread_create(&tid, NULL, serverthread, (void *) sd2 );
     }
     close(sd);
}


void * serverthread(void * parm)
{
   uintptr_t tsd;				/* JASON:fixes unsafe cast void* -> int */
   int tvisits;
   char     buf[100];           /* buffer for string the server sends */

   tsd = (uintptr_t) parm;

   pthread_mutex_lock(&mut);
        tvisits = ++visits;
   pthread_mutex_unlock(&mut);

   sprintf(buf,"This server has been contacted %d time%s\n",
	   tvisits, tvisits==1?".":"s.");

   printf("SERVER thread: %s", buf);
   send(tsd,buf,strlen(buf),0);
   close(tsd);
   pthread_exit(0);
}    
