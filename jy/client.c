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

 int main ( )
 {
   int clientfd, result, idx, input_index, last_quantity, listSize, price_sum = 0;
   char c;
   char inmsg[MAXLINE], outmsg[MAXLINE];
   char Sendlist[MAX] = {0};
   struct sockaddr_un serverUNIXaddr;
   
   clientfd = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
   serverUNIXaddr.sun_family = AF_UNIX;
   strcpy(serverUNIXaddr.sun_path, "Food");
   do { /* 연결 요청 */
      result = connect(clientfd, &serverUNIXaddr, sizeof(serverUNIXaddr));
      if (result == -1) sleep(1);
   } while (result == -1);

   while(1)
   {
      printf("소극장 음식 메뉴\n");
      printf("※ 0을 기입하시면 계산 메뉴로 이동 ※\n");
      read(clientfd, &listSize, sizeof(listSize));//1
      Food foods[MAXLINE] = {0};
      for(int i = 0 ; i < listSize ; i++){
         read(clientfd, &foods[i], sizeof(foods[i]));//2
      }
      
      for(int i = 0 ; i < listSize ; i++){
         printf("[%d] ", i+1);
        printf("%s %d %d\n", foods[i].name, foods[i].price, foods[i].quantity);

      }
      
      if(price_sum > 0) printf("현재 계산하실 금액은 %d원 입니다.\n", price_sum);

         while(1){ // 인덱스 기입창
            printf("원하는 음식의 인덱스를 기입해주세요. : ");
            scanf("%d", &input_index);
            if(input_index > listSize || input_index  < 0){
               printf("잘못된 인덱스입니다. 다시 입력해주세요.\n");
               continue;
            }
            else break;
         }
         write(clientfd, &input_index, sizeof(int)); // 3
         if(input_index == 0) break;
           read(clientfd, &last_quantity, sizeof(int)); // 4
           
      /* 수량 선택*/
           int input_quantity;
           while(1){
              printf("구매하실 음식의 수량을 입력해주세요 : ");
              scanf("%d", &input_quantity);
              if(last_quantity - input_quantity < 0 || input_quantity < 1){
                 printf("구매할 수 없는 수량이 입력되었습니다. 다시 입력해주세요.\n");
                 continue;
              }
              else{
                  write(clientfd, &input_quantity, sizeof(int)); // 5
                  read(clientfd, &price_sum, sizeof(int)); // 6 가격 받기
                  break;
              }
           }       
   }
   if(price_sum == 0){
      printf("계산 할 수 없습니다. 프로그램을 종료합니다.");
      exit(0);
   }
   int save_sum = price_sum;
   while(1){
      printf("계산하실 금액은 %d입니다. 계산하실 금액을 지불해주세요:", price_sum);
      int input_price;
      scanf("%d", &input_price);
      if(input_price <= 0){
         printf("지불할 수 없는 금액입니다.\n");
         continue;
      }
      price_sum -= input_price;
      if(price_sum > 0){
      printf("%d 원 지불이 확인되었습니다.\n", input_price); 
      printf("남은 ");
      continue;
   }
   else if(price_sum == 0) {
      printf("계산이 완료되었습니다. 구매해주셔서 감사합니다!\n");
      write(clientfd, &save_sum, sizeof(int));
      break;
   }
}
   
   close(clientfd);
   exit(0);
 }