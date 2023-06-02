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

    while (1) {
        printf("Enter a message (or 'exit' to quit): ");
        fgets(buffer, BUFFER_SIZE, stdin); //movie or food 입력
        buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거

        // 서버로 메시지 전송
        send(sock, buffer, strlen(buffer), 0);

        // 종료 명령 확인
        if (strcmp(buffer, "exit") == 0)
            break;

        // 영화 제목 입력 받기
        memset(buffer, 0, sizeof(buffer)); //이전 버퍼값 초기화
        valread = read(sock, buffer, BUFFER_SIZE); //영화목록 서버에서 받기
        printf("Server: %s\n", buffer); //영화목록 출력
        printf("Enter movie🎬 name you see. => ");
        fgets(buffer, BUFFER_SIZE, stdin); //영화 제목 입력받기
        buffer[strcspn(buffer, "\n")] = 0; // 개행 문자 제거
        send(sock, buffer, strlen(buffer), 0); // 서버로 메시지 전송

        // 서버 응답 수신(나이입력)
        memset(buffer, 0, sizeof(buffer));
        valread = read(sock, buffer, BUFFER_SIZE);
        printf("Server: %s\n", buffer);
        int bu=0;
        while(bu == 1){
            memset(buffer, 0, sizeof(buffer));
            valread = read(sock, buffer, BUFFER_SIZE);
            bu = buffer;
            fgets(buffer, BUFFER_SIZE, stdin); //나이 입력
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
