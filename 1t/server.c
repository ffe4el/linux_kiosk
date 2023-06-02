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

typedef struct {
    char title[50];
    char director[50];
    char year[10];
    char summary[10000];
} Movie;

void handle_client(int client_socket, Movie *movies, int num_movies) {
    char buffer[BUFFER_SIZE];
    char *welcome_message = "🎀------------------------------🎀\n|                                |\n|  Welcome to the Movie🎬 Kiosk! | \n|                                |\n🎀------------------------------🎀";
    int valread;

    printf("Server Start!\n");

    // 클라이언트에게 환영 메시지 전송
    send(client_socket, welcome_message, strlen(welcome_message), 0);
    printf("Welcome message sent to the client\n");

    // 클라이언트와의 통신
    while (1) {
        // 클라이언트로부터 메시지 수신
        valread = read(client_socket, buffer, BUFFER_SIZE);
        printf("Client: %s\n", buffer);

        // 종료 명령 확인
        if (strcmp(buffer, "exit") == 0)
            break;

        // 영화 목록 전송
        char movie_list[BUFFER_SIZE] = {0};
        for (int i = 0; i < num_movies; i++) {
            sprintf(movie_list, "%s\nTitle: %s\nDirector: %s\nYear: %s\nSummary: %s\n", movie_list, movies[i].title, movies[i].director, movies[i].year, movies[i].summary);
        }
        send(client_socket, movie_list, strlen(movie_list), 0);
        printf("Movie list sent to the client\n");

        // 버퍼 초기화
        memset(buffer, 0, sizeof(buffer));
    }

    // 클라이언트 소켓 닫기
    close(client_socket);
}

int main() {
    int server_fd, client_sockets[MAX_CLIENTS];
    struct sockaddr_un address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 영화 목록 초기화
    Movie movies[] = {
        {"Avatar", "James Cameron", "2009","하반신 마비를 "},
        {"Transformers", "Michael Bay", "2007", "지구 밖 "},
        {"Avengers", "Joss Whedon", "2012", "쉴드의 국장 "},
        {"The Devil Wears Prada", "David Frankel", "2006","앤드리아는 최고의 패션 매거진"},
        {"About Time", "Richard Curtis", "2013","평범한 소년 팀은 성인이 되던 날, 아버"}
        // {"Begin Again", "John Carney", "2013"},
        // {"La La Land", "Damien Chazelle", "2016"}
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

