#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/movie_kiosk1.sock"
#define BUFFER_SIZE 1024

int main() {
    int client_sock;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // 클라이언트 소켓 생성
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("Failed to create client socket");
        exit(1);
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // 서버에 연결
    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to server");
        exit(1);
    }

    // 영화 목록 요청
    strcpy(buffer, "Movie List Request");
    write(client_sock, buffer, sizeof(buffer));

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
        // 나이 전송
        sprintf(buffer, "%d", age);
        write(client_sock, buffer, sizeof(buffer));

        // 좌석 맵 수신 및 출력
        memset(buffer, 0, sizeof(buffer));
        read(client_sock, buffer, sizeof(buffer));
        printf("Seat Map:\n%s\n", buffer);

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
        int num_seats;
        printf("Enter the number of seats you want to book: ");
        scanf("%d", &num_seats);

        printf("Select seats (row column):\n");

        for (int i = 0; i < num_seats; i++) {
            int row, column;
            printf("Seat %d: ", i + 1);
            scanf("%d %d", &row, &column);

            // 좌석 입력 전송
            sprintf(buffer, "%d %d", row, column);
            write(client_sock, buffer, sizeof(buffer));
        }

        // 영화표 금액 출력
        printf("Total price: %d\n", price * num_seats);

        // 다음 나이 입력
        printf("Enter your age (-1 to exit): ");
        scanf("%d", &age);
    }

    // 클라이언트 소켓 종료
    close(client_sock);

    return 0;
}
