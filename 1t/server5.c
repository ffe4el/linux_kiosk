#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SOCKET_PATH "movie_kiosk_socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
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

// typedef struct {
//     int client_sock;
//     char seat_map[4][5];
// } ClientInfo;


// int select_seat(int *seat_status,int row, int col) {
//     if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS) {
//         return 0; // 좌석이 유효하지 않음
//     }

//     if (seat_status[row][col] == 1) {
//         return 0; // 이미 예약된 좌석
//     }

//     seat_status[row][col] = 1; // 좌석 예약
//     return 1; // 좌석 예약 성공
// }



// 클라이언트와의 통신
void handle_client(int client_socket, FILE *fp, int num_movies) {
    char buffer[BUFFER_SIZE];
    char *welcome_message = "🎀------------------------------🎀\n|                                |\n|  Welcome to the Movie🎬 Kiosk! | \n|                                |\n🎀------------------------------🎀";
    int valread;
    int num_people;
    Movie *movies = (Movie *)malloc(num_movies * sizeof(Movie));

    // 1. 클라이언트에게 환영 메시지 전송
    write(client_socket, welcome_message, strlen(welcome_message));
    printf("Welcome message sent to the client\n");

    // 2. send num_movies
    fseek(fp, 0, SEEK_SET);
    fread(movies, sizeof(Movie), num_movies, fp);
    write(client_socket, &num_movies, sizeof(num_movies));
    
    // 3. send struct movie_list
    for(int i=0; i<num_movies;i++){
        write(client_socket, &movies[i], sizeof(movies[i]));
    }
    
    // 4. 클라이언트로부터 choose 메세지 수신
    int choose;
    valread = read(client_socket, &choose, sizeof(choose));
    printf("Client: %d\n", choose);

    // 종료 명령 확인
    if (choose==3){
        close(client_socket);
    }
        
    //관리자모드
    int pwd=0;
    else if (choose==4){
        read(client_socket, &pwd, sizeof(pwd));//비번 받기
        if(pwd==1234){
            printf("welcom! manager~");//관리자모드로 들어오기 성공
            printf("add food\n");
            read(sock, &chose, sizeof(chose));//선택 받기
            fseek(fp,0,SEEK_END);//파일위치 맨뒤로 옮기고
            
            printf("%s %s %s %s %s %s %s %s %s\n", "Index", "Title", "Director", "Year", "cast","cast_num","Minimum_age","Last_ticket","Seats");
            scanf("%d %s %s %s %s %d %d %s %s\n", &movie[num_movies+1].index, movie[num_movies+1].title, movie[num_movies+1].director, movie[num_movies+1].cast, );
            num_movies = num_movies+1;
            fwrite();

            
            
        }
        else{ //비번이 틀리면 바로 종료
            close(sock);
        }
    }

    else if(choose==1){
        int adult =1;
        int row, col;
        while(1){
            // 6. 영화제목수신
            int movie_index1;
            read(client_socket, &movie_index1, sizeof(movie_index1));
            printf("Client: %d\n", movie_index1);

            int movie_index = movie_index1-1;

            // 8. 해당 영화의 남은 티켓수 보내기
            int last_tk = movies[movie_index].last_ticket;
            write(client_socket, &last_tk, sizeof(last_tk));
            printf("last_ticket : %d\n", last_tk);

            //9. 인원 수신
            while(1){
                read(client_socket, &num_people, sizeof(num_people));
                if(last_tk-num_people >= 0){
                    break;
                }
                else{
                    printf("exceed the number of available ticket.\n");
                    continue;
                }
            }
            
            // 10,11. 나이 입력 받기
            adult=1;
            int ticket_price=0;
            int age;
            for(int i=0; i<num_people; i++){
                read(client_socket, &age, sizeof(age)); //10
                if(movies[movie_index].minimum_age == 19 && age < 19){
                    printf("R-grade movie. Send warning message\n");
                    adult = 0;
                    break;
                }
            }
            if (adult == 0){
                continue;
            }
            read(client_socket, &ticket_price, sizeof(ticket_price)); //11
            printf("ticket price : %d\n", ticket_price);
            printf("last_ticket : %d\n", movies[movie_index].last_ticket);
            movies[movie_index].last_ticket -= num_people; //영화남은 인원에서 현재 인원을 뺌
            
            //12. 좌석 선택하기
            for(int i=0; i<num_people; i++){
                while(1){
                    //현재 좌석 상황 보내기
                    for (int i = 0; i < NUM_ROWS; i++) {
                        for (int j = 0; j < NUM_COLS; j++) {
                            write(client_socket, &movies[movie_index].seat_status[i][j], sizeof(movies[movie_index].seat_status[i][j]));//12
                            // printf("[%d]", movies[movie_index].seat_status[i][j]);
                        }
                        // printf("\n");
                    }
                    // printf("\n");

                    //좌석입력받기
                    read(client_socket, &row, sizeof(row));//13
                    read(client_socket, &col, sizeof(col));//14
                    row = row-1;
                    col = col-1;
                    int seat_selection_result;
                    //좌석이 유효한지 검사
                    if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS) {
                        seat_selection_result = 0; // 좌석이 유효하지 않음
                    }
                    if (movies[movie_index].seat_status[row][col] == 1) {
                        seat_selection_result = 0; // 이미 예약된 좌석
                    }
                    else if(movies[movie_index].seat_status[row][col] == 0){
                        movies[movie_index].seat_status[row][col] = 1; // 좌석 예약
                        seat_selection_result = 1;
                    }
                    write(client_socket, &seat_selection_result, sizeof(seat_selection_result));//15
                    if (seat_selection_result) {
                        printf("Seat selected: %d행 %d열\n", row, col);
                        //현재 상태 보여주기
                        printf("Seat Status:\n");
                        for (int i = 0; i < NUM_ROWS; i++) {
                            for (int j = 0; j < NUM_COLS; j++) {
                                write(client_socket, &movies[movie_index].seat_status[i][j], sizeof(movies[movie_index].seat_status[i][j]));
                            }
                        }
                        break;
                    } else {
                        printf("Seat selection failed: %d행 %d열\n", row, col);
                        continue;
                    }
                }
            }
            fseek(fp, (movie_index)*sizeof(Movie),SEEK_SET);
            fwrite(&movies[movie_index], sizeof(Movie), 1, fp);
            break;
        }
    }
    // 클라이언트 소켓 닫기
    free(movies);
    close(client_socket);
}

int main() {
    int server_fd, client_sockets[MAX_CLIENTS];
    struct sockaddr_un address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    printf("Server Start!\n");

    // 영화 목록 초기화
    Movie movies[] = {
        {1, "Avatar", "James Cameron", "2009", {"Sam Worthington", "Zoe Saldana", "Sigourney Weaver", "Stephen Lang"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {2, "Transformers", "Michael Bay", "2007", {"Shia LaBeouf", "Megan Fox", "Josh Duhamel", "Tyrese Gibson"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {3, "Avengers", "Joss Whedon", "2012", {"Robert Downey Jr.", "Chris Evans", "Mark Ruffalo", "Chris Hemsworth"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {4, "The Devil Wears Prada", "David Frankel", "2006", {"Meryl Streep", "Anne Hathaway", "Emily Blunt", "Stanley Tucci"}, 4, 15,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {5, "About Time", "Richard Curtis", "2013", {"Domhnall Gleeson", "Rachel McAdams", "Bill Nighy", "Margot Robbie"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {6, "Begin Again", "John Carney", "2013", {"Keira Knightley", "Mark Ruffalo", "Adam Levine", "Hailee Steinfeld"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {7, "La La Land", "Damien Chazelle", "2016", {"Ryan Gosling", "Emma Stone", "John Legend", "Rosemarie DeWitt"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {8,"Resident Evil", "Paul Anderson", "2002", {"Milla Jovovich", "Michelle Rodriguez", "Ryan McCluskey", "Oscar Pearce"}, 4, 19,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}}
    };
    int num_movies = sizeof(movies) / sizeof(movies[0]);

    
    FILE *fp = fopen("movie_db", "wb+");
    fwrite(movies, num_movies * sizeof(Movie), 1, fp);
    fclose(fp);
    fp = fopen("movie_db", "rb+");

    //fseek(fp, (n-1)*sizeof(Movie),SEEK_SET);
    //fwrite(movies[n-1], sizeof(Movie), 1, fp);
    // fread(movies2, num_movies * sizeof(Movie), 1, fp);
    // for (int i=0;i<num_movies;i++)
    // {
    //     printf("%d %s %s\n", movies2[i].index, movies2[i].title, movies2[i].director);
    // }


    // 서버 소켓 생성
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 소켓 주소 설정
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    // 이전에 생성된 소켓 파일 제거
    unlink(SOCKET_PATH);

    // 소켓과 주소를 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 클라이언트의 연결 대기
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    int num_clients = 0;

    //현재 좌석 상황 받고 출력하기
    // printf("Seat Status:\n");
    // for (int i = 0; i < NUM_ROWS; i++) {
    //     for (int j = 0; j < NUM_COLS; j++) {
    //         // read(sock, &seat_status[i][j], sizeof(seat_status[i][j]));//12
    //         printf("[%d] ", movies[movie_index].seat_status[i][j]);
    //     }
    //     printf("\n");
    // }
    // printf("\n");

    // 다중 클라이언트 처리
    while (1) {
        // 클라이언트의 연결 수락
        int client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_socket < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // 새로운 클라이언트 소켓을 배열에 저장
        client_sockets[num_clients] = client_socket;
        num_clients++;

        // 자식 프로세스 생성
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // 자식 프로세스에서 클라이언트 처리
            handle_client(client_socket, fp, num_movies);

            // 자식 프로세스 종료
            exit(EXIT_SUCCESS);
        } else {
            // 부모 프로세스는 클라이언트 연결 대기를 위해 계속 진행
            close(client_socket);
        }

        // 최대 클라이언트 수를 초과하면 대기 중인 클라이언트들을 처리
        if (num_clients >= MAX_CLIENTS) {
            for (int i = 0; i < num_clients; i++) {
                int status;
                waitpid(pid, &status, 0);
            }
            num_clients = 0;
        }
    }

    // 소켓 닫기
    close(server_fd);

    // 소켓 파일 제거
    unlink(SOCKET_PATH);

    return 0;
}

//지금 문제점이 클라이언트를 한번 실행해서 7번 영화로 티켓 4장을 소비하면 다음 클라이언트가 들어올때 
//7번영화 티켓이 16장으로 남아야하는데... 왜 반영이 안될까
