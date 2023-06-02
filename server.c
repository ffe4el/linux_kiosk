#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_ITEMS 100
#define MAX_NAME_LENGTH 50

typedef struct {
    char name[MAX_NAME_LENGTH];
    int price;
    int quantity;
} Item;

typedef struct {
    int socket;
    char name[MAX_NAME_LENGTH];
    int items[MAX_ITEMS];
    int itemQuantities[MAX_ITEMS];
    int itemCount;
    int totalAmount;
} Client;

// Global variables
pthread_mutex_t mutex;
Item movies[2];
Item snacks[3];
int snackCount = 0;
Client clients[MAX_CLIENTS];
int clientCount = 0;

// Function to send a message to the client
void sendMessage(int socket, const char *message) {
    write(socket, message, strlen(message) + 1); // Include a newline character in the message
}

// Function to handle client requests
void *clientHandler(void *arg) {
    Client *client = (Client *)arg;
    int socket = client->socket;
    char buffer[1024];

    sendMessage(socket, "=== Welcome to the Kiosk ===\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        if (read(socket, buffer, sizeof(buffer)) <= 0) {
            break;
        }

        // Process client request
        if (strcmp(buffer, "1") == 0) {
            // Movie selection
            pthread_mutex_lock(&mutex);
            char movieList[1024];
            strcpy(movieList, "Movie selection:\n1. Movie A\n2. Movie B\n");
            sendMessage(socket, movieList);
        } else if (strcmp(buffer, "2") == 0) {
            // Snack selection
            pthread_mutex_lock(&mutex);
            char snackList[1024];
            strcpy(snackList, "Snack selection:\n1. Popcorn - $7\n2. Caramel - $8\n3. Garlic Flavor - $8\n");
            sendMessage(socket, snackList);
        } else if (strcmp(buffer, "exit") == 0) {
            // Exit command
            break;
        } else if (strncmp(buffer, "s", 1) == 0) {
            // Process snack selection
            int snackIndex = atoi(buffer + 1) - 1;
            pthread_mutex_lock(&mutex);
            if (snackIndex < 0 || snackIndex >= snackCount) {
                sendMessage(socket, "Invalid snack index.\n");
            } else if (snacks[snackIndex].quantity <= 0) {
                sendMessage(socket, "Snack is out of stock.\n");
            } else {
                // Add the snack to the client's selection
                client->items[client->itemCount] = snackIndex;
                client->itemQuantities[client->itemCount] = 1;
                client->itemCount++;
                snacks[snackIndex].quantity--;
                sendMessage(socket, "Snack added to your selection.\n");
            }
            pthread_mutex_unlock(&mutex);
        } else if (strncmp(buffer, "m", 1) == 0) {
            // Process movie selection
            int movieIndex = atoi(buffer + 1) - 1;
            pthread_mutex_lock(&mutex);
            if (movieIndex < 0 || movieIndex >= 2) {
                sendMessage(socket, "Invalid movie index.\n");
            } else {
                // Set the movie selection for the client
                client->items[client->itemCount] = movieIndex;
                client->itemQuantities[client->itemCount] = 1;
                client->itemCount++;
                sendMessage(socket, "Movie added to your selection.\n");
            }
            pthread_mutex_unlock(&mutex);
        } else if (strcmp(buffer, "checkout") == 0) {
            // Process payment and checkout
            pthread_mutex_lock(&mutex);
            int totalAmount = 0;
            for (int i = 0; i < client->itemCount; i++) {
                int itemIndex = client->items[i];
                int quantity = client->itemQuantities[i];
                if (itemIndex < 2) {
                    totalAmount += movies[itemIndex].price;
                } else if (itemIndex < snackCount + 2) {
                    totalAmount += snacks[itemIndex - 2].price * quantity;
                }
            }
            client->totalAmount = totalAmount;
            sendMessage(socket, "Payment successful. Thank you for your purchase!\n");
            pthread_mutex_unlock(&mutex);
            break;
        } else {
            sendMessage(socket, "Invalid command.\n");
        }
    }

    close(socket);

    /// Release the client slot
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < clientCount; i++) {
        if (clients[i].socket == socket) {
            for (int j = i; j < clientCount - 1; j++) {
                clients[j] = clients[j + 1];
            }
            clientCount--;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}

// Function to handle user input for item management
void *itemManagement(void *arg) {
    char buffer[1024];

    while (1) {
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "add\n") == 0) {
            // Add a new snack
            pthread_mutex_lock(&mutex);
            Item newSnack;
            printf("Enter snack name: ");
            fgets(newSnack.name, MAX_NAME_LENGTH, stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            newSnack.name[strcspn(newSnack.name, "\n")] = '\0';
            printf("Enter snack price: ");
            scanf("%d", &newSnack.price);
            printf("Enter snack quantity: ");
            scanf("%d", &newSnack.quantity);
            getchar(); // Read the newline character
            snacks[snackCount++] = newSnack;
            pthread_mutex_unlock(&mutex);
            printf("Snack added.\n");
        } else if (strcmp(buffer, "list\n") == 0) {
            // List all snacks
            pthread_mutex_lock(&mutex);
            printf("Snacks:\n");
            for (int i = 0; i < snackCount; i++) {
                printf("%d. %s - $%d - Quantity: %d\n", i + 1, snacks[i].name, snacks[i].price, snacks[i].quantity);
            }
            pthread_mutex_unlock(&mutex);
        } else if (strcmp(buffer, "exit\n") == 0) {
            // Exit item management
            break;
        } else {
            printf("Invalid command.\n");
        }
    }

    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen;
    pthread_t tid;
    pthread_t itemManagementThread;
    pthread_create(&itemManagementThread, NULL, itemManagement, NULL);

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    // Initialize server socket
    // 소켓이름 : serverSocket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    // Set server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(8888); // Change the port to 8888

    // Bind server socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind socket");
        exit(1);
    }

    // Start listening for client connections
    // 대기큐 생성
    if (listen(serverSocket, 10) < 0) {
        perror("Failed to listen for connections");
        exit(1);
    }

    printf("Kiosk server is running.\n");

    // Accept and handle client connections
    while (1) {
        clientLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            perror("Failed to accept client connection");
            exit(1);
        }

        // Create a new client thread
        pthread_mutex_lock(&mutex);
        if (clientCount >= MAX_CLIENTS) {
            sendMessage(clientSocket, "The server has reached its maximum capacity. Please try again later.\n");
            close(clientSocket);
        } else {
            clients[clientCount].socket = clientSocket;
            strcpy(clients[clientCount].name, inet_ntoa(clientAddr.sin_addr));
            clients[clientCount].itemCount = 0;
            clients[clientCount].totalAmount = 0;
            memset(clients[clientCount].items, -1, sizeof(clients[clientCount].items));
            memset(clients[clientCount].itemQuantities, 0, sizeof(clients[clientCount].itemQuantities));
            clientCount++;
            pthread_create(&tid, NULL, clientHandler, &clients[clientCount - 1]);
            pthread_detach(tid);
        }
        pthread_mutex_unlock(&mutex);
    }

    // Clean up
    close(serverSocket);
    pthread_mutex_destroy(&mutex);

    return 0;
}