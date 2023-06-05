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
    int minimum_age; 
} Movie;

void handle_client(int client_socket, Movie *movies, int num_movies) {
    char buffer[BUFFER_SIZE];
    char *welcome_message = "🎀------------------------------🎀\n|                                |\n|  Welcome to the Movie🎬 Kiosk! | \n|                                |\n🎀------------------------------🎀";
    int valread;

    // 클라이언트에게 환영 메시지 전송
    send(client_socket, welcome_message, strlen(welcome_message), 0);
    printf("Welcome message sent to the client\n");

    // 클라이언트와의 통신
    while (1) {
        memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화 
        // 클라이언트로부터 메시지 수신
        valread = read(client_socket, buffer, BUFFER_SIZE);
        printf("Client: %s\n", buffer);

        if(strcmp(buffer, "movie") == 1){ //movie를 선택했을시
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
            memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화  

            char movie_name[50];
            // sprintf(movie_name, "Please enter the movie name");
            // send(client_socket, movie_name, strlen(movie_name), 0);
            valread = read(client_socket, buffer, BUFFER_SIZE);
            // 영화 제목 확인
            int movie_index = -1;
            for (int i = 0; i < num_movies; i++) {
                if (strcmp(buffer, movies[i].title) == 0) {
                    movie_index = i;
                    break;
                }
            } 
            memset(buffer, 0, sizeof(buffer)); // 버퍼 초기화  

            if (movie_index >= 0) {
                // 관람연령 확인
                int minimum_age = movies[movie_index].minimum_age;
                char age_question[50];
                int num_people;
                char age_ing[3];
                int ticket_price;
                while(num_people!=-1){
                    sprintf(age_question, "Please enter your age (Enter -1 to finish)");
                    send(client_socket, age_question, strlen(age_question), 0);
                    // 나이 입력받기
                    valread = read(client_socket, buffer, BUFFER_SIZE);
                    num_people = atoi(buffer);
                    printf("Client: %s\n", buffer);

                    //청불영화인데, 성인이 아니라면.. 다시 영화고르는 페이지로...
                    if(movies[movie_index].minimum_age == 19 && num_people < 19){
                        char error_message[] = "This is R-grade moive. please choose different movie.";
                        send(client_socket, error_message, strlen(error_message), 0);
                        //다시 영화고르는 화면으로 돌아가게 할 예정
                    }

                    // 가격 계산
                    if (num_people < 0)
                        ticket_price = 0;
                    else if ((18 < num_people) && (num_people<= 64)) //성인
                        ticket_price += num_people * 15000;
                    else if ((13 < num_people) && (num_people<= 18)) //청소년
                        ticket_price += num_people * 12000;
                    else    //어린이, 노인
                        ticket_price += num_people * 8000;

                    sprintf(age_ing, "1");
                    send(client_socket, age_ing, strlen(age_ing), 0);
                }
                sprintf(age_ing, "0");
                send(client_socket, age_ing, strlen(age_ing), 0);

                // 가격 전송
                char price_message[50];
                sprintf(price_message, "Total price: %d", ticket_price);
                send(client_socket, price_message, strlen(price_message), 0);
                } else {
                    // 올바르지 않은 영화 제목인 경우
                    char error_message[] = "Invalid movie title";
                    send(client_socket, error_message, strlen(error_message), 0);
                }
        }

        // 종료 명령 확인
        if (strcmp(buffer, "exit") == 0)
            break;
        
        
        

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

    printf("Server Start!\n");

    // 영화 목록 초기화
    Movie movies[] = {
        {"Avatar", "James Cameron", "2009", {"Sam Worthington", "Zoe Saldana", "Sigourney Weaver", "Stephen Lang"}, 4, 12},
        {"Transformers", "Michael Bay", "2007", {"Shia LaBeouf", "Megan Fox", "Josh Duhamel", "Tyrese Gibson"}, 4, 12},
        {"Avengers", "Joss Whedon", "2012", {"Robert Downey Jr.", "Chris Evans", "Mark Ruffalo", "Chris Hemsworth"}, 4, 12},
        {"The Devil Wears Prada", "David Frankel", "2006", {"Meryl Streep", "Anne Hathaway", "Emily Blunt", "Stanley Tucci"}, 4, 15},
        {"About Time", "Richard Curtis", "2013", {"Domhnall Gleeson", "Rachel McAdams", "Bill Nighy", "Margot Robbie"}, 4, 12},
        {"Begin Again", "John Carney", "2013", {"Keira Knightley", "Mark Ruffalo", "Adam Levine", "Hailee Steinfeld"}, 4, 12},
        {"La La Land", "Damien Chazelle", "2016", {"Ryan Gosling", "Emma Stone", "John Legend", "Rosemarie DeWitt"}, 4, 12},
        {"범죄도시", "강윤성", "2017", {"마동석", "윤계상", "조재윤", "최귀화"}, 4, 19}
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
