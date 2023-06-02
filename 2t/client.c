#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Create client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // Set server address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Change this to the server IP address
    server_addr.sin_port = htons(8888);  // Change this to the server port

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to server");
        exit(1);
    }

    // 서버로부터 환영 메시지 수신
    int valread = read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);

    // Receive movie list from server
    while (1) {
        read(sock, buffer, BUFFER_SIZE);
        if (strlen(buffer) == 0) {
            break;
        }
        printf("%s", buffer);
    }

    // Get movie choice from user
    printf("Enter the number of the movie you want to watch: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    write(sock, buffer, strlen(buffer));

    // Receive seat map from server
    while (1) {
        read(sock, buffer, BUFFER_SIZE);
        if (strlen(buffer) == 0 || buffer[0] == '\n') {
            break;
        }
        printf("%s", buffer);
    }

    // Get age from user
    printf("Enter your age: ");
    fgets(buffer, BUFFER_SIZE, stdin);
    write(sock, buffer, strlen(buffer));

    // Receive response from server
    while (1) {
        read(sock, buffer, BUFFER_SIZE);
        if (strlen(buffer) == 0) {
            break;
        }
        printf("%s", buffer);
    }

    // Close the socket
    close(sock);

    return 0;
}
