#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define BUFFER_SIZE 1024

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("Error in socket");
        exit(1);
    }

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr.sin_addr)) <= 0) {
        perror("Error in configuring server address");
        exit(1);
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error in connecting to the server");
        exit(1);
    }

    // Receive welcome message from the server
    memset(buffer, 0, sizeof(buffer));
    if (recv(clientSocket, buffer, sizeof(buffer) - 1, 0) < 0) {
        perror("Error in receiving data from the server");
        exit(1);
    }
    printf("Server: %s\n", buffer);

    // Display menu options
    printf("Menu:\n");
    printf("1. Movie\n");
    printf("2. Snacks\n");
    printf("3. Checkout\n");
    printf("Enter your choice: ");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        // Send user input to the server
        if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
            perror("Error in sending data to the server");
            exit(1);
        }

        // Receive response from the server
        memset(buffer, 0, sizeof(buffer));
        if (recv(clientSocket, buffer, sizeof(buffer) - 1, 0) < 0) {
            perror("Error in receiving data from the server");
            exit(1);
        }

        // Display server response
        printf("Server: %s\n", buffer);

        if (strcmp(buffer, "Payment successful. Thank you for your purchase!\n") == 0) {
            break;  // Exit the loop if the checkout is successful
        }

        printf("Enter your choice: ");
    }

    // Close the socket
    close(clientSocket);

    return 0;
}