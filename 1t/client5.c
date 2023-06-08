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
#define NUM_ROWS 4
#define NUM_COLS 5

typedef struct {
    int index;
    char title[50];
    char director[50];
    char year[10];
    char cast[MAX_CAST_MEMBERS][50];
    int num_cast_members;
    int minimum_age;
    int last_ticket; //남은 티켓 개수
    int seat_status[NUM_ROWS][NUM_COLS];
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
    //welcome_message[valread] = '\0'; // 널 문자 추가
    printf("%s\n", welcome_message);

    // 2. receive num_movies
    read(sock, &num_movies, sizeof(num_movies));
    printf("num_movies : %d\n", num_movies);
    
    // 3. receive struct movie_list 
    Movie movies[MAX_MOVIES] = {0};
    for(int i=0; i<num_movies; i++){
	    read(sock, &movies[i], sizeof(movies[i]));
    }
    // printf("")

    
    printf("Enter a number 1. movie or 2. food? (or 3. quit): ");
    int choose;
    scanf("%d", &choose); //movie or food 입력
    printf("%d", choose);
    // 4. choose 메세지 전달
    write(sock, &choose, sizeof(choose));

    // 종료 명령 확인
    if (choose==3){
        // 소켓 닫기
    	close(sock);
    }

    //관리자모드
    int pwd=0;
    else if (choose==4){
        scanf("%d", &pwd);
        write(sock, &pwd, sizeof(pwd));//비번 보내기
        if(pwd==1234){
            printf("welcom! manager~");//관리자모드로 들어오기 성공
            int chose=0;
            printf("1. add movie\n2. add food\n");
            scanf("%d", &chose);
            write(sock, &chose, sizeof(chose));//선택 보내기
            if(chose==1){
                Movie movies = {}
                fwrite();
            }
            else if(chose==2){

            }



        }
        else{
            close(sock);
        }
        
    }

    else if (choose==1){
        // 영화목록 보여주기
        printf("Movie List~!\n");
        for (int i = 0; i < num_movies; i++) {
            printf("index : %d\nTitle: %s\nDirector: %s\nYear: %s\nminimum_age: %d\nAvailable Ticket:%d\nCast Members:\n", movies[i].index, movies[i].title, movies[i].director, movies[i].year,movies[i].minimum_age,movies[i].last_ticket);
            for (int j = 0; j < movies[i].num_cast_members; j++) {
                printf("- %s\n", movies[i].cast[j]);
            }
            printf("\n");
            printf("\n");
        }

        printf("0-13 : 8000won\n14-18 : 12000won\n19-64 : 15000won\nover 64 : 8000won\n");


        int adult =1;
        while(1){
            // 6. 영화 제목 입력 받기
            int movie_index1;
            printf("Enter movie🎬 number you see. => ");
            scanf("%d", &movie_index1); //영화 제목 입력받기
            write(sock, &movie_index1, sizeof(movie_index1)); // 서버로 메시지 전송

            // 고른 영화에 인덱스 부여하기
            int movie_index = movie_index1-1;
            if (movie_index > num_movies-1 && movie_index1 < 0) {
                printf("Invalid movie selection.\n");
                continue;
            }

            // 8. 서버에서 해당 영화의 남은 티켓수 받기
            int last_tk;
            read(sock, &last_tk, sizeof(last_tk));
            printf("available ticket🎟️ : %d\n", last_tk);

            // 9. 사람 수 입력받기
            int num_people=0;
            while(1){
                printf("Enter the number of seats you want to book:  => ");
                scanf("%d" , &num_people);
                printf("you enter %d.\n", num_people);
                write(sock, &num_people, sizeof(num_people)); // 서버로 메시지 전송
                if(last_tk - num_people >= 0){
                    printf("available!!\n"); 
                    break;
                }
                else{
                    //구매가능개수를 초과하면 다시 입력할 수 있도록..
                    printf("you have exceeded the number of available ticket.");
                    continue;
                }
            }

            // 10,11. 나이 입력 받기
            int ticket_price=0;
            int age;
            for(int i=0; i<num_people; i++){
                printf("Enter age!(one by one plz)\n");
                scanf("%d", &age);
                write(sock, &age, sizeof(age)); //10
                if(movies[movie_index].minimum_age == 19 && age < 19){
                    printf("This is R-grade moive. please choose different movie.\n");
                    adult = 0;
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
            if (adult == 0){
                continue;
            }
            write(sock, &ticket_price, sizeof(ticket_price)); //11
            printf("Total price : %d\n", ticket_price);

            //12. 좌석 선택하기
            for(int i=0; i<num_people; i++){ //입력된 사람 수만큼 좌석 선택
                while(1){
                    //현재 좌석 상황 받고 출력하기
                    printf("Seat Status:\n");
                    for (int i = 0; i < NUM_ROWS; i++) {
                        for (int j = 0; j < NUM_COLS; j++) {
                            read(sock, &movies[movie_index].seat_status[i][j], sizeof(movies[movie_index].seat_status[i][j]));//12
                            printf("[%d] ", movies[movie_index].seat_status[i][j]);
                        }
                        printf("\n");
                    }
                    printf("\n");

                    int row, col;
                    //앉고 싶은 좌석 입력받기
                    printf("Enter the row and column of the seat you want to select (e.g., 3 4): ");
                    scanf("%d %d", &row, &col);
                    write(sock, &row, sizeof(row));//13
                    write(sock, &col, sizeof(col));//14

                    int result;
                    read(sock, &result, sizeof(result));//15. 좌석 유효 검사 받기, 좌석 선점하기
                    if (result) {
                        printf("Seat selected: %d행 %d열\n", row, col);
                        printf("Seat selection successful\n");
                        //현재 상태 보여주기
                        printf("Seat Status:\n");
                        for (int i = 0; i < NUM_ROWS; i++) {
                            for (int j = 0; j < NUM_COLS; j++) {
                                read(sock, &movies[movie_index].seat_status[i][j], sizeof(movies[movie_index].seat_status[i][j]));
                                printf("[%d] ", movies[movie_index].seat_status[i][j]);
                            }
                            printf("\n");
                        }
                        printf("\n");
                        printf("Thank you for your purchase! Please enjoy your time~");
                        break;
                    } else {
                        printf("Seat selection failed: %d행 %d열\n", row, col);
                        continue;
                    }

                }
            }
            break;
        }
    }
    // 소켓 닫기
    close(sock);
    return 0;
}