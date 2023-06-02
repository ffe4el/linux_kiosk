#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/movie_kiosk_socket"
#define BUFFER_SIZE 1024

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

    // 서버로부터 환영 메시지 수신
    valread = read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);

    // 클라이언트와의 통신
    while (1) {
        printf("Enter a message (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        // 서버로 메시지 전송
        send(sock, buffer, strlen(buffer), 0);

        // 종료 명령 확인
        if (strcmp(buffer, "exit") == 0)
            break;

        // 영화 목록 수신
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, BUFFER_SIZE);
        printf("Server:\n%s\n", buffer);
    }

    // 소켓 닫기
    close(sock);

    return 0;
}
