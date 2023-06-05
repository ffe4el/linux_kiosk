#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "locker.h"

#define DEFAULT_PROTOCOL 0
#define MAXLINE 100
 int main ()
 {
   int listenfd, connfd, clientlen;
   char inmsg[MAXLINE], outmsg[MAXLINE];
   int idx, i, n;
   struct sockaddr_un serverUNIXaddr, clientUNIXaddr;
   struct locker *record; 
   
   printf("Set the number of lockers: ");
   scanf("%d", &n);
    
   record = (struct locker *) malloc(n * sizeof(struct locker));

   printf("Set %d lockers.\n", n);
   printf("%-4s %-9s %-8s %-4s\n", "Index", "ID", "Name", "Thing(x means blank)");
   for (i = 0; i < n; i++)
   {
    	printf("   %d   ", i+1);
        scanf("%d %s %s", &record[i].id, record[i].name, record[i].thing);
   }

   signal(SIGCHLD, SIG_IGN);
   clientlen = sizeof(clientUNIXaddr);

   listenfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
   serverUNIXaddr.sun_family = AF_UNIX;
   strcpy(serverUNIXaddr.sun_path, "locker");
   unlink("locker");
   bind(listenfd, &serverUNIXaddr, sizeof(serverUNIXaddr));

   listen(listenfd, 5);

   while (1) { /* 소켓 연결 요청 수락 */
      connfd = accept(listenfd, &clientUNIXaddr, &clientlen);
      if (fork ( ) == 0) {
         while(1)
         {
         	read(connfd, &idx, sizeof(int));
		write(connfd, &record[idx-1], sizeof(struct locker));
		read(connfd, &record[idx-1], sizeof(struct locker));
         }
         
         close(connfd);
         exit (0);
      } else close(connfd);
   }
 }