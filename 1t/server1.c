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

typedef struct {
    char title[50];
    char director[50];
    char year[10];
    char cast[MAX_CAST_MEMBERS][50];
    int num_cast_members;
} Movie;

void handle_client(int client_socket, Movie *movies, int num_movies) {
    char buffer[BUFFER_SIZE];
    char *welcome_message = "Welcome to the Movie Kiosk!";
    int valread;

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
            sprintf(movie_list, "%s\nTitle: %s\nDirector: %s\nYear: %s\nCast Members:\n", movie_list, movies[i].title, movies[i].director, movies[i].year);

            for (int j = 0; j < movies[i].num_cast_members; j++) {
                sprintf(movie_list, "%s- %s\n", movie_list, movies[i].cast[j]);
            }
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
        {"Avatar", "James Cameron", "2009", {"Sam Worthington", "Zoe Saldana", "Sigourney Weaver", "Stephen Lang"}, 4},
        {"Transformers", "Michael Bay", "2007", {"Shia LaBeouf", "Megan Fox", "Josh Duhamel", "Tyrese Gibson"}, 4},
        {"Avengers", "Joss Whedon", "2012", {"Robert Downey Jr.", "Chris Evans", "Mark Ruffalo", "Chris Hemsworth"}, 4},
        {"The Devil Wears Prada", "David Frankel", "2006", {"Meryl Streep", "Anne Hathaway", "Emily Blunt", "Stanley Tucci"}, 4},
        {"
