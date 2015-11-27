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
#include <unistd.h> //Added to fix implicit definition of close()
#include <stddef.h> //Added to fix bad signedness on alen at L 117


void * serverthread(void * parm);       /* thread function prototype    */
char string_palin(char str[]);			/* palindrome function prototype */

pthread_mutex_t  mut;

#define PROTOPORT         5193          /* default protocol port number */
#define QLEN              6             /* size of request queue        */
#define TBUFSIZE          100           /*size of thread message buffers */

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
     // UNUSED IN ORIGINAL SAMPLE CODE: struct   hostent   *ptrh;     /* pointer to a host table entry */
     struct   protoent  *ptrp;     /* pointer to a protocol table entry */
     struct   sockaddr_in sad;     /* structure to hold server's address */
     struct   sockaddr_in cad;     /* structure to hold client's address */
     
     int      sd;             /* socket descriptors */
     uintptr_t sd2;					/*ADDED BY JASON to prevent L 119 unsafe void* cast.  sd2 was previously an int*/
     int      port;                /* protocol port number */
     socklen_t      alen;                /* length of address */
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
   printf("SERVER thread: Opening thread.\n");
   uintptr_t tsd;				/* JASON: fixes unsafe cast void* -> int */
   //int tvisits;
   char     bufout[TBUFSIZE];           /* buffer for string the server sends */
   char		bufin[TBUFSIZE];				/* buffer for string the client sends */

   tsd = (uintptr_t) parm;

   //pthread_mutex_lock(&mut);
   //     tvisits = ++visits;
   //pthread_mutex_unlock(&mut);
   
   if(read(tsd, bufin, TBUFSIZE) < 0){
	   printf("SERVER thread: read failure at 146.\n");
	   exit(1);
   }
   
   printf("SERVER thread: %s\n", bufin);
   if(string_palin(bufin) == 1)
     sprintf(bufout, "This is a palindrome.\n");
   else if (string_palin(bufin) == 0)
     sprintf(bufout, "This is not a palindrome.\n");
   else
     sprintf(bufout, "The palindrome function has problems.\n");

   //sprintf(bufout,"This server has been contacted %d time%s\n",
	 //  tvisits, tvisits==1?".":"s.");
	 

   

   printf("SERVER thread: %s", bufout);
   
   printf("SERVER thread: Sending response to client.");
   send(tsd,bufout,strlen(bufout),0);
   
   printf("SERVER thread: Closing socket.");
   close(tsd);
   
   printf("SERVER thread: Exiting thread\n");
   pthread_exit(0);
}    

char string_palin(char str[])
{
 int i,j;
 //for(i=0; str[i]!= NULL; i++);
 i = strlen(str);
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
