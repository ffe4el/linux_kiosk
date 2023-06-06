#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "movie_kiosk_socket"
#define BUFFER_SIZE 1024
#define MAX_CAST_MEMBERS 4
#define MAX_MOVIES 10

typedef struct {
    char title[50];
    char director[50];
    char year[10];
    char cast[MAX_CAST_MEMBERS][50];
    int num_cast_members;
    int minimum_age;
    int last_ticket; //남은 티켓 개수
} Movie;

int main() {
    int sock = 0, valread;
    struct sockaddr_un serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char welcome_message[BUFFER_SIZE] = {0};
    int num_movies;
    char movie_list[BUFFER_SIZE] = {0};


    // 클라이언트 소켓 생성
    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error\n");
        return -1;
    }

    // 소켓 주소 설정
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKET_PATH, sizeof(serv_addr.sun_path) - 1);

    // 서버에 연결 요청
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed\n");
        return -1;
    }
//------------------------------------------------------------------------------------------

    // 1. 서버로부터 환영 메시지 수신
    valread = read(sock, welcome_message, sizeof(welcome_message) - 1); // 수정: read의 길이 인자 수정
    welcome_message[valread] = '\0'; // 널 문자 추가
    printf("%s\n", welcome_message);

    // 2. receive num_movies
    read(sock, &num_movies, sizeof(num_movies));
    printf("num_movies : %d\n", num_movies);
    
    // 3. receive struct movie_list 
    Movie movies[MAX_MOVIES];
    for(int i=0; i<num_movies; i++){
	    read(sock, &movies[i], sizeof(movies[i]));
    }

    

    printf("Enter a message movie or food? (or 'exit' to quit): ");
    char choose[20];
    scanf("%s", choose); //movie or food 입력
    // 4. choose 메세지 전달
    write(sock, choose, strlen(choose));

    // 종료 명령 확인
    if (strcmp(choose, "exit") == 0)
        // 소켓 닫기
    	close(sock);

    else if(strcmp(choose, "movie") == 0){
        // 5. 영화목록 서버에서 받기
        read(sock, movie_list, sizeof(movie_list)); 
        printf("Server: %s\n", movie_list); //영화목록 출력

        int adult =1;
        while(adult){
            // 6. 영화 제목 입력 받기
            char movie_name[20];
            printf("Enter movie🎬 name you see. => ");
            scanf("%s", movie_name); //영화 제목 입력받기
            write(sock, movie_name, strlen(movie_name)); // 서버로 메시지 전송

            //receive movie list
            //valread = read(sock, buffer, BUFFER_SIZE);
            //Movie movies[] = buffer;
            //memset(buffer, 0, sizeof(buffer));

            // 7. 고른 영화에 인덱스 부여하기
            int movie_index = -1;
            for (int i = 0; i < num_movies; i++) {
                if (strcmp(movie_name, movies[i].title) == 0) {
                    movie_index = i;
                    write(sock, &movie_index, sizeof(movie_index)); // 서버로 메시지 전송
                    break;
                }
            }
            if (movie_index == -1) {
                printf("Invalid movie selection.\n");
                continue;
            }

            // 8. 서버에서 해당 영화의 남은 티켓수 받기
            int last_tk;
            read(sock, &last_tk, sizeof(last_tk));

            // 9. 사람 수 입력받기
            int num_people;
            while(1){
                printf("Enter the number of seats you want to book:  => ");
                scanf("%d" , &num_people);
                if(last_tk - num_people >= 0){
                    write(sock, &num_people, sizeof(num_people)); // 서버로 메시지 전송
                    break;
                }
                else{
                    //구매가능개수를 초과하면 다시 입력할 수 있도록..
                    printf("you have exceeded the number of available ticket.");
                    continue;
                }
            }

            // 10,11,12. 나이 입력 받기
            adult=1;
            int ticket_price=0;
            int age;
            for(int i=0; i<num_people; i++){
                scanf("%d", &age);
                write(sock, &age, sizeof(age)); //10
                if(movies[movie_index].minimum_age == 19 && age < 19){
                    printf("This is R-grade moive. please choose different movie.");
                    write(sock, &adult,sizeof(adult)); //11
                    adult=0; //다시 영화 고르자~~
                }
                // 가격 계산
                if (age < 0)
                    ticket_price = 0;
                else if ((18 < age) && (age<= 64)) //성인
                    ticket_price += 15000;
                else if ((13 < age) && (age<= 18)) //청소년
                    ticket_price += 12000;
                else    //어린이, 노인
                    ticket_price += 8000;
            }
            write(sock, &ticket_price, sizeof(ticket_price)); //12
            printf("Total price : %d", ticket_price);
        }
        


        // 서버 응답 수신
        // memset(buffer, 0, sizeof(buffer));
        // valread = read(sock, buffer, BUFFER_SIZE);
        // printf("Server: %s\n", buffer);
    }




    // 소켓 닫기
    close(sock);

    return 0;
}