#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SOCKET_PATH "movie_kiosk_socket"
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_CAST_MEMBERS 4
#define MAX_MOVIES 10
#define NUM_ROWS 4
#define NUM_COLS 5
#define MAXLINE 100
#define MAX 1024
#define FIXED_QUANTITY 30

// typedef struct{
//     int money;
//     int listSize;
// }returnn;

typedef struct {
    int index;
    char title[50];
    char director[50];
    char year[10];
    char cast[MAX_CAST_MEMBERS][50];
    int num_cast_members;
    int minimum_age;
    int last_ticket; //남은 티켓 개수
    int seat_status[NUM_ROWS][NUM_COLS];
} Movie;

typedef struct {
   char name[MAX];
   int price;
   int quantity;
} Food;

// 클라이언트와의 통신
int handle_client(int client_socket, FILE *fp, int num_movies) {
    char buffer[BUFFER_SIZE];
    char *welcome_message = "🎀----------------------------------🎀\n|                                    |\n|  Welcome to the Theater🎬🍿 Kiosk! | \n|                                    |\n🎀----------------------------------🎀";
    int valread;
    int num_people;
    int ticket_price=0;
    
    Movie *movies = (Movie *)malloc(num_movies * sizeof(Movie));

    // 1. 클라이언트에게 환영 메시지 전송
    write(client_socket, welcome_message, strlen(welcome_message));
    printf("Welcome message sent to the client\n");

    // 2. send num_movies
    fseek(fp, 0, SEEK_SET);
    fread(movies, sizeof(Movie), num_movies, fp);
    write(client_socket, &num_movies, sizeof(num_movies));
    
    // 3. send struct movie_list
    for(int i=0; i<num_movies;i++){
        write(client_socket, &movies[i], sizeof(movies[i]));
    }
    
    // 4. 클라이언트로부터 choose 메세지 수신
    int choose;
    valread = read(client_socket, &choose, sizeof(choose));
    printf("Client: %d\n", choose);

    //관리자모드
    // if (choose==4){
        // int pwd=0;
        // read(client_socket, &pwd, sizeof(pwd));//비번 받기
        // if(pwd==1234){
        //     printf("welcom! manager~\n");//관리자모드로 들어오기 성공
        //     printf("you can set food list\n");
        //     fseek(fp1,0,SEEK_SET);//파일위치 맨앞으로
        //     int num;
        //     printf("Set the number of food => ");
        //     scanf("%d", &num);
        //     for(int i=0; i<num; i++){
        //         printf("  Name    Price   Quantity\n");
        //         scanf("%s %d %d", foods[listSize+1].name, &foods[listSize+1].price, &foods[listSize+1].quantity);
        //     }
        //     listSize = num;
        //     p.listSize = listSize;
        //     fwrite(foods, p.listSize * sizeof(Food), 1, fp1);
        //     printf("Addition is complete! good bye\n");
        //     fclose(fp1);
        //     close(client_socket);
        // }
        // else{ //비번이 틀리면 바로 종료
        //     close(client_socket);
        // }
    // }

    if(choose==1){
        int adult =1;
        int row, col;
        while(1){
            // 6. 영화제목수신
            int movie_index1;
            read(client_socket, &movie_index1, sizeof(movie_index1));
            printf("Client: %d\n", movie_index1);

            int movie_index = movie_index1-1;

            // 8. 해당 영화의 남은 티켓수 보내기
            int last_tk = movies[movie_index].last_ticket;
            write(client_socket, &last_tk, sizeof(last_tk));
            printf("last_ticket : %d\n", last_tk);

            //9. 인원 수신
            while(1){
                read(client_socket, &num_people, sizeof(num_people));
                if(last_tk-num_people >= 0){
                    break;
                }
                else{
                    printf("exceed the number of available ticket.\n");
                    continue;
                }
            }
            
            // 10. 나이 입력 받기
            adult=1;
            int age;
            for(int i=0; i<num_people; i++){
                read(client_socket, &age, sizeof(age)); //10
                if(movies[movie_index].minimum_age == 19 && age < 19){
                    printf("R-grade movie. Send warning message\n");
                    adult = 0;
                    break;
                }
            }
            if (adult == 0){
                continue;
            }
            //11. 총 가격 받기
            read(client_socket, &ticket_price, sizeof(ticket_price));
            printf("ticket price : %d\n", ticket_price);
            printf("last_ticket : %d\n", movies[movie_index].last_ticket);
            movies[movie_index].last_ticket -= num_people; //영화남은 인원에서 현재 인원을 뺌
            
            //12. 좌석 선택하기
            for(int i=0; i<num_people; i++){
                while(1){
                    //현재 좌석 상황 보내기
                    for (int i = 0; i < NUM_ROWS; i++) {
                        for (int j = 0; j < NUM_COLS; j++) {
                            write(client_socket, &movies[movie_index].seat_status[i][j], sizeof(movies[movie_index].seat_status[i][j]));//12
                        }
                    }

                    //좌석입력받기
                    read(client_socket, &row, sizeof(row));//13
                    read(client_socket, &col, sizeof(col));//14
                    row = row-1;
                    col = col-1;
                    int seat_selection_result;
                    //좌석이 유효한지 검사
                    if (row < 0 || row >= NUM_ROWS || col < 0 || col >= NUM_COLS) {
                        seat_selection_result = 0; // 좌석이 유효하지 않음
                        printf("안뇽\n");
                    }
                    else{
                        if (movies[movie_index].seat_status[row][col] == 1) {
                        seat_selection_result = 0; // 이미 예약된 좌석
                        }
                        else if(movies[movie_index].seat_status[row][col] == 0){
                            movies[movie_index].seat_status[row][col] = 1; // 좌석 예약
                            seat_selection_result = 1;
                        }
                    }
                    
                    write(client_socket, &seat_selection_result, sizeof(seat_selection_result));//15
                    if (seat_selection_result==1) {
                        printf("Seat selected: %d행 %d열\n", row, col);
                        // 현재 상태 보여주기
                        printf("Seat Status:\n");
                        for (int i = 0; i < NUM_ROWS; i++) {
                            for (int j = 0; j < NUM_COLS; j++) {
                                write(client_socket, &movies[movie_index].seat_status[i][j], sizeof(movies[movie_index].seat_status[i][j]));
                            }
                        }
                        break;
                    } else {
                        printf("Seat selection failed : %d행 %d열\n", row+1, col+1);
                        continue;
                    }
                }
            }
            fseek(fp, (movie_index)*sizeof(Movie),SEEK_SET);
            fwrite(&movies[movie_index], sizeof(Movie), 1, fp);
            break;
        }
    }
    // 클라이언트 소켓 닫기
    free(movies);
    return ticket_price;
}


int food_client(int client_socket, FILE *fp1, int listSize,int money) {
   int price_sum = 0, TodayTotalPrice = 0;
   int idx, i, n, input_index, input_quantity;
   Food *foods = (Food*)malloc(listSize * sizeof(Food));
   
    while(1){
        fseek(fp1, 0, SEEK_SET);
        fread(foods, sizeof(Food), listSize, fp1);
        
        //1. 음식 갯수 보내기
        write(client_socket, &listSize, sizeof(listSize)); // 1

        //2. 음식 리스트 보내기
        for(int i=0 ; i<listSize ; i++) {
            write(client_socket, &foods[i], sizeof(foods[i]));//2
        }
        
        //3. 음식 고르기
        read(client_socket, &input_index, sizeof(int)); // 3
        if(input_index == 0) break;

        //4. 남은 음식 갯수 보내기
        int last_quantity = foods[input_index-1].quantity;
        write(client_socket, &last_quantity, sizeof(int)); // 4

        //5. 구매할 음식 수량 받기
        int input_quantity;
        read(client_socket, &input_quantity, sizeof(int));//5
        foods[input_index - 1].quantity -= input_quantity;
        
        //6. 가격보내기
        price_sum += (foods[input_index-1].price *input_quantity);
        // price_sum += money; //영화 가격 합치기
        printf("%d\n", price_sum);
        write(client_socket, &price_sum, sizeof(int)); //6 
        write(client_socket, &money, sizeof(money)); //6-1 영화 가격 보내기


        fseek(fp1, (input_index-1) * sizeof(Food), SEEK_SET);
        fwrite(&foods[input_index-1], sizeof(Food), 1, fp1);

        for (int i = 0; i < listSize; i++)
        {
            printf("[%d] ", i+1);
            printf("%s %d %d\n", foods[i].name, foods[i].price, foods[i].quantity);
        }
    }
    
    //7. 총 결제 금액 받기
    int save_sum;
    read(client_socket, &save_sum, sizeof(int));
    printf("%d원이 계산되었습니다.\n", save_sum);
   
    free(foods);
    close(client_socket);
    exit (0);
   
}



int main() {
    int server_fd, client_sockets[MAX_CLIENTS];
    struct sockaddr_un address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    printf("Server Start!\n");

    // 영화 ---------------------------------------------------------
    // 영화 목록 초기화
    Movie movies[] = {
        {1, "Avatar", "James Cameron", "2009", {"Sam Worthington", "Zoe Saldana", "Sigourney Weaver", "Stephen Lang"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {2, "Transformers", "Michael Bay", "2007", {"Shia LaBeouf", "Megan Fox", "Josh Duhamel", "Tyrese Gibson"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {3, "Avengers", "Joss Whedon", "2012", {"Robert Downey Jr.", "Chris Evans", "Mark Ruffalo", "Chris Hemsworth"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {4, "The Devil Wears Prada", "David Frankel", "2006", {"Meryl Streep", "Anne Hathaway", "Emily Blunt", "Stanley Tucci"}, 4, 15,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {5, "About Time", "Richard Curtis", "2013", {"Domhnall Gleeson", "Rachel McAdams", "Bill Nighy", "Margot Robbie"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {6, "Begin Again", "John Carney", "2013", {"Keira Knightley", "Mark Ruffalo", "Adam Levine", "Hailee Steinfeld"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {7, "La La Land", "Damien Chazelle", "2016", {"Ryan Gosling", "Emma Stone", "John Legend", "Rosemarie DeWitt"}, 4, 12,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}},
        {8,"Resident Evil", "Paul Anderson", "2002", {"Milla Jovovich", "Michelle Rodriguez", "Ryan McCluskey", "Oscar Pearce"}, 4, 19,20,{{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}}}
    };
    int num_movies = sizeof(movies) / sizeof(movies[0]);

    FILE *fp = fopen("movie_db", "wb+");
    fwrite(movies, num_movies * sizeof(Movie), 1, fp);
    fclose(fp);
    fp = fopen("movie_db", "rb+");

    // 음식 ---------------------------------------------------------
    Food foodlist[] = {{"팝콘", 8000, FIXED_QUANTITY}, {"카라멜 팝콘", 9000, FIXED_QUANTITY}, {"치즈 팝콘", 9000, FIXED_QUANTITY}, {"콜라", 2000, FIXED_QUANTITY}, {"사이다", 2000, FIXED_QUANTITY}, {"환타", 2000, FIXED_QUANTITY}};
    int listSize = (sizeof(foodlist)/sizeof(*foodlist));
    FILE *fp1 = fopen("food_db", "wb+");
    fwrite(foodlist, listSize * sizeof(Food), 1, fp1);
    fclose(fp1);
    fp1 = fopen("food_db", "rb+");

    //관리자모드 들어갈지 말지...
    int manage;
    printf("choose~! 1 : kiosk mode, 2: manager mode\n");
    scanf("%d", &manage);
    while(1){
        if(manage == 2){ //관리자 모드
            FILE *fp1 = fopen("food_db", "wb+");
            int pwd=0;
            printf("Enter the Password => ");
            scanf("%d", &pwd);
            if(pwd==1234){
                printf("welcom! manager~\n");//관리자모드로 들어오기 성공
                printf("you can set food list\n");
                fseek(fp1,0,SEEK_SET);//파일위치 맨앞으로
                int num;
                printf("Set the number of food => ");
                scanf("%d", &num);
                for(int i=0; i<num; i++){
                    printf("  Name    Price   Quantity\n");
                    scanf("%s %d %d", foodlist[i].name, &foodlist[i].price, &foodlist[i].quantity);
                }
                listSize = num; //리스트 사이즈 업데이트!
                fwrite(foodlist, listSize * sizeof(Food), 1, fp1);
                printf("Addition is complete!\n");
                fclose(fp1);
                fp1 = fopen("food_db", "rb+");
                printf("now ready to pair.\n");
                break;
            }
            else{ //비번이 틀리면 다시 비번 적을 수 있도록
                continue;
            }
            break;
        }
        else if(manage == 1){  //판매 모드
            break;
        }
    }
    
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
            int money = handle_client(client_socket, fp, num_movies);

            food_client(client_socket, fp1, listSize, money);

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

