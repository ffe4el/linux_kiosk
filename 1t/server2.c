#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SOCKET_PATH "/tmp/movie_kiosk_socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
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

typedef struct {
    int client_sock;
    char seat_map[4][5];
} ClientInfo;

void print_seat_map(char seat_map[][5]) {
    printf("Seat Map:\n");
    printf("  1 2 3 4 5\n");

    for (int i = 0; i < 4; i++) {
        printf("%d ", i + 1);
        for (int j = 0; j < 5; j++) {
            printf("%c ", seat_map[i][j]);
        }
        printf("\n");
    }
}


// 클라이언트와의 통신
void handle_client(int client_socket, Movie *movies, int num_movies) {
    char buffer[BUFFER_SIZE];
    char *welcome_message = "🎀------------------------------🎀\n|                                |\n|  Welcome to the Movie🎬 Kiosk! | \n|                                |\n🎀------------------------------🎀";
    int valread;
    int num_people;

    // 1. 클라이언트에게 환영 메시지 전송
    write(client_socket, welcome_message, strlen(welcome_message));
    printf("Welcome message sent to the client\n");

    // 2. send num_movies
    write(client_socket, &num_movies, sizeof(num_movies));
    
    // 3. send struct movie_list
    for(int i=0; i<num_movies;i++){
        write(client_socket, &movies[i], sizeof(movies[i]));
    }
    
    // 4. 클라이언트로부터 choose 메세지 수신
    char choose[20];
    read(client_socket, choose, strlen(choose));
    printf("Client: %s\n", choose);

    // 종료 명령 확인
    if (strcmp(choose, "exit") == 0)
        close(client_socket);

    else if(strcmp(choose, "movie") == 0){
        // 5. 영화 목록 전송
        char movie_list[BUFFER_SIZE] = {0};
        for (int i = 0; i < num_movies; i++) {
            sprintf(movie_list, "%s\nTitle: %s\nDirector: %s\nYear: %s\nminimum_age: %d\nCast Members:\n", movie_list, movies[i].title, movies[i].director, movies[i].year,movies[i].minimum_age);
            for (int j = 0; j < movies[i].num_cast_members; j++) {
                sprintf(movie_list, "%s- %s\n", movie_list, movies[i].cast[j]);
            }
        }
        write(client_socket, movie_list, strlen(movie_list));
        printf("Movie list sent to the client\n");
        
        int adult =1;
        while(adult){
            // 6. 영화제목수신
            char movie_name[20];
            read(client_socket, movie_name, strlen(movie_name));

            //send movie list
            // sprintf(buffer, "%s", (struct Movie*) &movies);
            // send(client_socket, buffer, strlen(buffer), 0);
            // memset(buffer, 0, sizeof(buffer));
        
            // 7. 영화인덱스수신
            int movie_index = -1;
            read(client_socket, &movie_index, sizeof(movie_index));

            // 8. 해당 영화의 남은 티켓수 보내기
            int last_tk = movies[movie_index].last_ticket;
            write(client_socket, &last_tk, sizeof(last_tk));
            printf("last_ticket : %d\n", last_tk);

            //9. 인원 수신
            read(client_socket, &num_people, sizeof(num_people));
            movies[movie_index].last_ticket -= num_people; //영화남은 인원에서 현재 인원을 뺌

            // 10,11,12. 나이 입력 받기
            adult=1;
            int ticket_price=0;
            int age;
            for(int i=0; i<num_people; i++){
                read(client_socket, &age, sizeof(age)); //10
                if(movies[movie_index].minimum_age == 19 && age < 19){
                    printf("R-grade movie. Send warning message");
                    read(client_socket, &adult, sizeof(adult)); //11
                    adult=0; //다시 영화 고르자~~
                }
            }
            read(client_socket, ticket_price, sizeof(ticket_price)); //12
        }
        

    }
    // 클라이언트 소켓 닫기
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
        {"Avatar", "James Cameron", "2009", {"Sam Worthington", "Zoe Saldana", "Sigourney Weaver", "Stephen Lang"}, 4, 12, 20},
        {"Transformers", "Michael Bay", "2007", {"Shia LaBeouf", "Megan Fox", "Josh Duhamel", "Tyrese Gibson"}, 4, 12, 20},
        {"Avengers", "Joss Whedon", "2012", {"Robert Downey Jr.", "Chris Evans", "Mark Ruffalo", "Chris Hemsworth"}, 4, 12, 20},
        {"The Devil Wears Prada", "David Frankel", "2006", {"Meryl Streep", "Anne Hathaway", "Emily Blunt", "Stanley Tucci"}, 4, 15, 20},
        {"About Time", "Richard Curtis", "2013", {"Domhnall Gleeson", "Rachel McAdams", "Bill Nighy", "Margot Robbie"}, 4, 12, 20},
        {"Begin Again", "John Carney", "2013", {"Keira Knightley", "Mark Ruffalo", "Adam Levine", "Hailee Steinfeld"}, 4, 12, 20},
        {"La La Land", "Damien Chazelle", "2016", {"Ryan Gosling", "Emma Stone", "John Legend", "Rosemarie DeWitt"}, 4, 12, 20}
    };
    int num_movies = sizeof(movies) / sizeof(movies[0]);
    
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
            handle_client(client_socket, movies, num_movies);

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