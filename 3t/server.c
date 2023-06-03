#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>

#define SOCKET_PATH "/tmp/movie_kiosk1.sock"
#define BUFFER_SIZE 1024

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

void* handle_client(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_sock = client_info->client_sock;
    char (*seat_map)[5] = client_info->seat_map;
    char buffer[BUFFER_SIZE];

    // 영화 목록 수신 및 출력
    memset(buffer, 0, sizeof(buffer));
    read(client_sock, buffer, sizeof(buffer));
    printf("Movie List:\n%s\n", buffer);

    // 영화 선택
    printf("Enter the movie title you want to watch: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    // 나이 입력
    printf("Enter your age (-1 to exit): ");
    int age;
    scanf("%d", &age);

    while (age != -1) {
        // 영화표 가격 계산
        int price;
        if (age >= 1 && age <= 13) {
            price = 8000;
        } else if (age >= 14 && age <= 18) {
            price = 12000;
        } else if (age >= 19 && age <= 64) {
            price = 15000;
        } else {
            price = 8000;
        }

        // 좌석 선택
        memset(seat_map, 'O', sizeof(*seat_map));

        int num_seats;
        printf("Enter the number of seats you want to book: ");
        scanf("%d", &num_seats);

        printf("Select seats (row column):\n");
        print_seat_map(seat_map);

        for (int i = 0; i < num_seats; i++) {
            int row, column;
            printf("Seat %d: ", i + 1);
            scanf("%d %d", &row, &column);

            seat_map[row - 1][column - 1] = 'X';
        }

        // 좌석 맵 전송
        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, seat_map, sizeof(*seat_map));
        write(client_sock, buffer, sizeof(buffer));

        // 영화표 금액 출력
        printf("Total price: %d\n", price * num_seats);

        // 다음 나이 입력
        printf("Enter your age (-1 to exit): ");
        scanf("%d", &age);
    }

    // 클라이언트 소켓 종료
    close(client_sock);
    free(client_info);

    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_len;
    pthread_t thread;

    // 서버 소켓 생성
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Failed to create server socket");
        exit(1);
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    int reuseaddr = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));


    // 서버 소켓 바인딩
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind server socket");
        exit(1);
    }

    // 서버 소켓 수신 대기 상태로 설정
    if (listen(server_sock, 5) == -1) {
        perror("Failed to listen on server socket");
        exit(1);
    }

    printf("Movie Kiosk server is running.\n");

    while (1) {
        // 클라이언트 연결 수락
        client_addr_len = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == -1) {
            perror("Failed to accept client connection");
            continue;
        }

        printf("New client connected.\n");

        // 클라이언트 정보를 담은 구조체 생성
        ClientInfo* client_info = (ClientInfo*)malloc(sizeof(ClientInfo));
        client_info->client_sock = client_sock;

        // 클라이언트와 통신을 처리할 스레드 생성
        if (pthread_create(&thread, NULL, handle_client, (void*)client_info) != 0) {
            perror("Failed to create thread");
            continue;
        }

        // 스레드 분리
        if (pthread_detach(thread) != 0) {
            perror("Failed to detach thread");
            continue;
        }
    }

    // 서버 소켓 종료
    close(server_sock);
    unlink(SOCKET_PATH);

    return 0;
}
