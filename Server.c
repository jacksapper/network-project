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
#include <stdlib.h>

void * serverthread(void * parm);       /* thread function prototype    */

pthread_mutex_t  mut;

#define PROTOPORT         5193          /* default protocol port number */
#define QLEN              6             /* size of request queue        */
#define BUFSIZE	          128

int visits =  0;                        /* counts client connections     */

struct tpass
{
   int sd2;
   char buf[BUFSIZE];
}

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

main (int argc, char *argv[])
{
     struct   hostent   *ptrh;     /* pointer to a host table entry */
     struct   protoent  *ptrp;     /* pointer to a protocol table entry */
     struct   sockaddr_in sad;     /* structure to hold server's address */
     struct   sockaddr_in cad;     /* structure to hold client's address */
     int      sd, sd2;             /* socket descriptors */
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
     
     if ( ((int)(ptrp = getprotobyname("tcp"))) == 0)  {
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

         struct tpass tp;
         tp.sd2 = sd2;
         if(recv(sd2, tp.buf, BUFSIZE, 0) < 0)
         {
             fprintf(stderr, "recv failed\n");
             exit (1);
         }

	 pthread_create(&tid, NULL, serverthread, (void *) sd2 );
     }
     close(sd);
}

int string_palin(char str[])
{
 int i,j;
 for(i=0; str[i]!=NULL; i++);
 for(j=0,i--; j<=i; )
 {
   if(str[i]==str[j])
   {
      i--;
      j++;
   }
  else
      break;
 }
 if(j>i)
    return(1);
 else
    return(0);
}

void * serverthread(void * parm)
{
   int tsd, tvisits;
   char     buf[128];           /* buffer for string the server sends */

   tsd = (int) parm;


   sprintf(buf,"This server has been contacted %d time%s\n",
	   tvisits, tvisits==1?".":"s.");

   printf("SERVER thread: %s", buf);
   send(tsd,buf,strlen(buf),0);
   close(tsd);
   pthread_exit(0);
}    


