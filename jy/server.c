#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#define DEFAULT_PROTOCOL 0
#define MAXLINE 100
#define MAX 1024

typedef struct {
   char name[MAX];
   int price;
   int quantity;
   } Food;

   Food foodlist[] = {{"Popcorn", 8000, 50}, {"Cock", 2000, 50}};
   int listSize = (sizeof(foodlist)/sizeof(*foodlist));
 int main ()
 {
   int listenfd, connfd, clientlen, price_sum = 0, TodayTotalPrice = 0;
   char inmsg[MAXLINE], outmsg[MAXLINE];
   int idx, i, n, input_index, input_quantity;
   struct sockaddr_un serverUNIXaddr, clientUNIXaddr;
   struct Food *record;
   
   
   printf("음식 관리 서버 Start!\n");
   printf("등록된 음식 메뉴\n");
    
   //record = (struct locker *) malloc(n * sizeof(struct locker));

   //printf("Set %d lockers.\n", n);
   //printf("%-4s %-9s %-8s %-4s\n", "Index", "ID", "Name", "Thing(x means blank)");
   for (int i = 0; i < listSize; i++)
   {
       printf("[%d] ", i+1);
        printf("%s %d %d\n", foodlist[i].name, foodlist[i].price, foodlist[i].quantity);
   }

   signal(SIGCHLD, SIG_IGN);
   clientlen = sizeof(clientUNIXaddr);

   listenfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
   serverUNIXaddr.sun_family = AF_UNIX;
   strcpy(serverUNIXaddr.sun_path, "Food");
   unlink("Food");
   bind(listenfd, &serverUNIXaddr, sizeof(serverUNIXaddr));

   listen(listenfd, 5); //연최몇?

   while (1) { /* 소켓 연결 요청 수락 */
      connfd = accept(listenfd, &clientUNIXaddr, &clientlen);
      if (fork ( ) == 0) {
         
   while(1){
      char Sendlist[MAX] = {0};
      for(int i = 0 ; i < listSize ; i++) {
               sprintf(Sendlist, "%s\n[%d] %s %d %d", Sendlist, i+1, foodlist[i].name,             foodlist[i].price, foodlist[i].quantity);
         }
   
      write(connfd, Sendlist, MAX); // 1
      write(connfd, &listSize, sizeof(int)); // 2
      read(connfd, &input_index, sizeof(int)); // 3
      if(input_index == 0) break;

      
      int last_quantity = foodlist[input_index-1].quantity;
      write(connfd, &last_quantity, sizeof(int)); // 4
      /*수량 선택*/
      int input_quantity;
      read(connfd, &input_quantity, sizeof(int));//5
      foodlist[input_index - 1].quantity -= input_quantity;
      /* 가격 계싼해서 보내버리기 */
      price_sum += (foodlist[input_index-1].price *input_quantity);
      printf("%d\n", price_sum);
      write(connfd, &price_sum, sizeof(int)); //6 
      
      for (int i = 0; i < listSize; i++)
         {
               printf("[%d] ", i+1);
                printf("%s %d %d\n", foodlist[i].name, foodlist[i].price,                foodlist[i].quantity);
         }
      }
      
      int save_sum;
      read(connfd, &save_sum, sizeof(int));
      printf("%d원이 계산되었습니다.\n", save_sum);
      TodayTotalPrice += save_sum;
      printf("금일 현재 판매금액 : %d원\n", TodayTotalPrice);
         
         close(connfd);
         exit (0);
      } else close(connfd);
   }
 }
