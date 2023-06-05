#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/movie_kiosk_socket"
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
    read(sock, welcome_message, strlen(welcome_message));
    printf("%s\n", welcome_message);

    // 2. receive num_movies
    read(sock, num_movies, sizeof(num_movies));
    printf("num_movies : %d", num_movies);
    
    // 3. receive struct movie_list 
    Movie movies[MAX_MOVIES];
    for(int i=0; i<num_movies; i++){
	read(sock, &movies[i], sizeof(Movie));
    }

    

    printf("Enter a message movie or food? (or 'exit' to quit): ");
    string choose;
    scanf("%s", choose); //movie or food 입력
    //buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거
    write(sock, choose, strlen(choose));

    // 종료 명령 확인
    if (strcmp(buffer, "exit") == 0)
        // 소켓 닫기
    	close(sock);

    else if(strcmp(buffer, "movie") == 0){
        memset(buffer, 0, sizeof(buffer)); //이전 버퍼값 초기화
        valread = read(sock, buffer, BUFFER_SIZE); //영화목록 서버에서 받기
        printf("Server: %s\n", buffer); //영화목록 출력
        memset(buffer, 0, sizeof(buffer));

        // 영화 제목 입력 받기
        printf("Enter movie🎬 name you see. => ");
        fgets(buffer, BUFFER_SIZE, stdin); //영화 제목 입력받기
        buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거
        send(sock, buffer, strlen(buffer), 0); // 서버로 메시지 전송
        memset(buffer, 0, sizeof(buffer));

	//receive movie list
	//valread = read(sock, buffer, BUFFER_SIZE);
	//Movie movies[] = buffer;
	//memset(buffer, 0, sizeof(buffer));

	
	

        //고른 영화에 인덱스 부여하기
        int movie_index = -1;
        for (int i = 0; i < num_movies; i++) {
            if (strcmp(buffer, movies[i].title) == 0) {
                movie_index = i;
                send(sock, movie_index, 1, 0); // 서버로 메시지 전송
                break;
            }
        } 
        memset(buffer, 0, sizeof(buffer));

        //서버에서 해당 영화의 남은 티켓수 받기
        valread = read(sock, buffer, sizeof(buffer));
        int last_tk = atoi(buffer);
        memset(buffer, 0, sizeof(buffer));

        // 사람 수 입력받기
        int num_people;
        while(1){
            printf("Enter the number of seats you want to book:  => ");
            fgets(buffer, BUFFER_SIZE, stdin); 
            buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거
            num_people = atoi(buffer);
            if(last_tk - num_people >= 0){
                send(sock, buffer, strlen(buffer), 0); // 서버로 메시지 전송
                memset(buffer, 0, sizeof(buffer));
                break;
            }
            else{
                //구매가능개수를 초과하면 다시 입력할 수 있도록..
                printf("you have exceeded the number of available ticket.");
                memset(buffer, 0, sizeof(buffer));
            }
        }

        // 서버 응답 수신(나이입력)
        // memset(buffer, 0, sizeof(buffer));
        // valread = read(sock, buffer, BUFFER_SIZE);
        // printf("Server: %s\n", buffer);
        int ticket_price=0;
        for(int i=0; i<num_people; i++){
            memset(buffer, 0, sizeof(buffer));
            fgets(buffer, BUFFER_SIZE, stdin); //나이입력받기
            buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거
            int age = atoi(buffer);
            if(movies[movie_index].minimum_age == 19 && age < 19){
                printf("This is R-grade moive. please choose different movie.");
                //다시영화고르는 단계로 돌려보내고 싶다...
                close(sock);
                printf("still here..?");
                break;
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
        printf("Total price : %d", ticket_price);

        


        // 서버 응답 수신
        // memset(buffer, 0, sizeof(buffer));
        // valread = read(sock, buffer, BUFFER_SIZE);
        // printf("Server: %s\n", buffer);
    }




    // 소켓 닫기
    close(sock);

    return 0;
}