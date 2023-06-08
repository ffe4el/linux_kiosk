#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#define DEFAULT_PROTOCOL 0
#define MAXLINE 100
#define MAX 1024
#define FIXED_QUANTITY 30

typedef struct {
   char name[MAX];
   int price;
   int quantity;
} Food;
   
int food_client(int connfd, FILE *fp, int listSize) {
   int price_sum = 0, TodayTotalPrice = 0;
   int idx, i, n, input_index, input_quantity;
   Food *foods = (Food*)malloc(listSize * sizeof(Food));
   
   while(1){
      fseek(fp, 0, SEEK_SET);
      fread(foods, sizeof(Food), listSize, fp);
      write(connfd, &listSize, sizeof(listSize)); // 1
      
      for(int i=0 ; i<listSize ; i++) {
         write(connfd, &foods[i], sizeof(foods[i]));//2
      }
      
      read(connfd, &input_index, sizeof(int)); // 3
      if(input_index == 0) break;

      
      int last_quantity = foods[input_index-1].quantity;
      write(connfd, &last_quantity, sizeof(int)); // 4
      
      /*수량 선택*/
      int input_quantity;
      read(connfd, &input_quantity, sizeof(int));//5
      foods[input_index - 1].quantity -= input_quantity;
      
      /* 가격 계싼해서 보내버리기 */
      price_sum += (foods[input_index-1].price *input_quantity);
      printf("%d\n", price_sum);
      write(connfd, &price_sum, sizeof(int)); //6 
      
      
      fseek(fp, (input_index-1) * sizeof(Food), SEEK_SET);
      fwrite(&foods[input_index-1], sizeof(Food), 1, fp);
      
      for (int i = 0; i < listSize; i++)
      {
         printf("[%d] ", i+1);
         printf("%s %d %d\n", foods[i].name, foods[i].price, foods[i].quantity);
      }
   }
      
   int save_sum;
   read(connfd, &save_sum, sizeof(int));
   printf("%d원이 계산되었습니다.\n", save_sum);
   

      close(connfd);
      exit (0);
   
}
   

  
int main ()
{
   int listenfd, connfd, clientlen, price_sum = 0, TodayTotalPrice = 0;
   char inmsg[MAXLINE], outmsg[MAXLINE];
   int idx, i, n, input_index, input_quantity;
   struct sockaddr_un serverUNIXaddr, clientUNIXaddr;

   Food foodlist[] = {{"팝콘", 8000, FIXED_QUANTITY}, {"카라멜 팝콘", 9000, FIXED_QUANTITY}, {"치즈 팝콘", 9000, FIXED_QUANTITY}, {"콜라", 2000, FIXED_QUANTITY}, {"사이다", 2000, FIXED_QUANTITY}, {"환타", 2000, FIXED_QUANTITY}};
   int listSize = (sizeof(foodlist)/sizeof(*foodlist));
   FILE *fp = fopen("food_db", "wb+");
   fwrite(foodlist, listSize * sizeof(Food), 1, fp);
   fclose(fp);
   fp = fopen("food_db", "rb+");

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
         food_client(connfd, fp, listSize);
      } else close(connfd);
   }
}