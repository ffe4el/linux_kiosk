#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    char title[100];
    char director[100];
    int release_year;
    char cast[4][100];
    int minimum_age;
} Movie;

Movie movies[] = {
    {"Avatar", "James Cameron", 2009, {"Sam Worthington", "Zoe Saldana", "Sigourney Weaver", "Stephen Lang"}, 12},
    {"Transformers", "Michael Bay", 2007, {"Shia LaBeouf", "Megan Fox", "Josh Duhamel", "Tyrese Gibson"}, 12},
    {"Avengers", "Joss Whedon", 2012, {"Robert Downey Jr.", "Chris Evans", "Mark Ruffalo", "Chris Hemsworth"}, 12},
    {"The Devil Wears Prada", "David Frankel", 2006, {"Meryl Streep", "Anne Hathaway", "Emily Blunt", "Stanley Tucci"}, 15},
    {"About Time", "Richard Curtis", 2013, {"Domhnall Gleeson", "Rachel McAdams", "Bill Nighy", "Margot Robbie"}, 12},
    {"Begin Again", "John Carney", 2013, {"Keira Knightley", "Mark Ruffalo", "Adam Levine", "Hailee Steinfeld"},12},
    {"La La Land", "Damien Chazelle", 2016, {"Ryan Gosling", "Emma Stone", "John Legend", "Rosemarie DeWitt"}, 12},
    {"범죄도시", "강윤성", 2017, {"마동석", "윤계상", "조재윤", "최귀화"}, 19}
};

int seats[4][5];

void initializeSeats() {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 5; j++) {
            seats[i][j] = 0;
        }
    }
}

void printMovieList(int client_sock) {
    char buffer[BUFFER_SIZE];
    int i;

    for (i = 0; i < sizeof(movies) / sizeof(movies[0]); i++) {
        snprintf(buffer, BUFFER_SIZE, "Movie %d: %s\n", i + 1, movies[i].title);
        write(client_sock, buffer, strlen(buffer));
    }
}

void handleClient(int client_sock) {
    char buffer[BUFFER_SIZE];
    int age, movie_choice, seat_row, seat_col;
    char *welcome_message = "🎀------------------------------🎀\n|                                |\n|  Welcome to the Movie🎬 Kiosk! | \n|                                |\n🎀------------------------------🎀";
    int i;

    // 클라이언트에게 환영 메시지 전송
    send(client_sock, welcome_message, strlen(welcome_message), 0);
    printf("Welcome message sent to the client\n");

    // Send movie list to client
    printMovieList(client_sock);

    // Receive movie choice from client
    read(client_sock, buffer, BUFFER_SIZE);
    movie_choice = atoi(buffer) - 1;

    // Send seat map to client
    for (i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            snprintf(buffer, BUFFER_SIZE, "%s ", seats[i][j] ? "■" : "□");
            write(client_sock, buffer, strlen(buffer));
        }
        write(client_sock, "\n", 1);
    }

    // Receive age from client
    read(client_sock, buffer, BUFFER_SIZE);
    age = atoi(buffer);

    // Check if age is valid for the selected movie
    if (age < movies[movie_choice].minimum_age) {
        snprintf(buffer, BUFFER_SIZE, "This movie is R-grade. Please choose another movie.\n");
        write(client_sock, buffer, strlen(buffer));
        return;
    }

    // Reserve seats
    for (i = 0; i < age; i++) {
        snprintf(buffer, BUFFER_SIZE, "Enter seat (row col) for person %d: ", i + 1);
        write(client_sock, buffer, strlen(buffer));
        read(client_sock, buffer, BUFFER_SIZE);
        sscanf(buffer, "%d %d", &seat_row, &seat_col);

        // Check if seat is already occupied
        if (seats[seat_row - 1][seat_col - 1]) {
            snprintf(buffer, BUFFER_SIZE, "Seat (%d, %d) is already occupied. Please choose another seat.\n", seat_row, seat_col);
            write(client_sock, buffer, strlen(buffer));
            i--;
        } else {
            seats[seat_row - 1][seat_col - 1] = 1;
        }
    }

    // Calculate total ticket price
    int ticket_price;
    if (age >= 1 && age <= 13) {
        ticket_price = 8000;
    } else if (age >= 14 && age <= 18) {
        ticket_price = 12000;
    } else if (age >= 19 && age <= 64) {
        ticket_price = 15000;
    } else {
        ticket_price = 8000;
    }

    // Send total ticket price to client
    snprintf(buffer, BUFFER_SIZE, "Total ticket price: %d\n", ticket_price);
    write(client_sock, buffer, strlen(buffer));
}

int main() {
    int server_sock, client_sock, client_addr_len;
    struct sockaddr_in server_addr, client_addr;

    initializeSeats();

    // Create server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // Set server socket options
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        perror("Failed to set socket options");
        exit(1);
    }

    // Bind server socket to a specific address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind socket");
        exit(1);
    }

    // Listen for client connections
    if (listen(server_sock, MAX_CLIENTS) == -1) {
        perror("Failed to listen for connections");
        exit(1);
    }

    printf("Server started. Waiting for clients...\n");

    while (1) {
        client_addr_len = sizeof(client_addr);

        // Accept client connection
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);
        if (client_sock == -1) {
            perror("Failed to accept client connection");
            continue;
        }

        printf("Client connected. IP address: %s, Port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Fork a child process to handle the client
        if (fork() == 0) {
            close(server_sock);
            handleClient(client_sock);
            close(client_sock);
            exit(0);
        } else {
            close(client_sock);
        }
    }

    return 0;
}
