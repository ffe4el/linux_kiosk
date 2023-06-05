#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "locker.h"
#define DEFAULT_PROTOCOL 0
#define MAXLINE 100

 int main ( )
 {
   int clientfd, result, idx;
   char c;
   char inmsg[MAXLINE], outmsg[MAXLINE];
   struct locker record;
   struct sockaddr_un serverUNIXaddr;

   clientfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
   serverUNIXaddr.sun_family = AF_UNIX;
   strcpy(serverUNIXaddr.sun_path, "locker");
   do { /* 연결 요청 */
      result = connect(clientfd, &serverUNIXaddr, sizeof(serverUNIXaddr));
      if (result == -1) sleep(1);
   } while (result == -1);

   while(1)
   {
   	   printf("Insert locker index: ");
	   scanf("%d",&idx);
	   write(clientfd,&idx,sizeof(int));

	   read(clientfd,&record,sizeof(struct locker));
	   printf("Index: %d\t ID:%2d\t Name:%4s\t Thing:%s\n", idx, record.id, record.name, record.thing);
	   
	   if (strcmp(record.thing, "x") == 0)
	   {
		printf("Enter a thing to keep(x means blank): ");
		scanf("%s", record.thing);
	   }
	    else
	    {

		printf("Do you want to clean the drawer?(Y/N) ");
		scanf(" %c", &c);
		if(c == 'Y')
		{
		      strcpy(record.thing,"x");
		}
	    }
	    write(clientfd,&record,sizeof(struct locker));
   }
   
   close(clientfd);
   exit(0);
 }
