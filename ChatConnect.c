#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Header files for socket programming.
#include <sys/socket.h> // Header file for socket operations.
#include <sys/un.h> // Provides definitions for Unix domain sockets.
#include <netinet/in.h> // Defines constants and structures for Internet domain addresses.
#include <arpa/inet.h> // Used for IP address convertion.

#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#define MESSAGE_SIZE 1024

int SocketFD;
int ServConnSockFD;
char message[MESSAGE_SIZE];
char user1[20];
char user2[20];
int quit = 0;

/* Threads declaration */
// Threads for user 1.
pthread_t ThreadSend;
pthread_t ThreadRecv;
// Threads for user 2.
pthread_t ThreadSend1;
pthread_t ThreadRecv1;

sem_t SemUser;
pthread_mutex_t discon;

// Declaration of Functions.
void clear_line();
void *sendThread();
void *ReceiveThread();
void *sendThread1();
void *ReceiveThread1();
void UserPrint(char user[], char user1[], int d);

void clear_line() {
    //printf("\033[1G"); // Move the cursor to the beginning of the line.
    printf("\033[2A"); // Move the cursor two lines up.
    printf("\033[2K\r"); // Erase the entire line and move the cursor to the beginning.
}

// Thread function for sending messages.
void *sendThread() {
    while (quit != 1) {
        // User input.
        printf("\n\033[%dG\033[32;48;2;91;54;117m - (%s) >> \033[0m \n\n", 12, user1);
        printf("        "); fgets(message, sizeof(message), stdin);
        message[strlen(message) - 1] = '\0';
        int bytesSent = send(ServConnSockFD, message, strlen(message), 0);
        if (bytesSent == -1) {
            perror("Error sending message");
            break;
        } else if (message[0] == '\0') {
            clear_line();
            printf("\n\n\t            \033[3;31m(Empty message! Enter a valid input)\033[0m\n\n");
        }

        // Disconnecting proccess.
        if ((strcmp(message, "quit") == 0) || (strcmp(message, "QUIT") == 0)) {
            printf("\n\t\t            \033[3;48;2;31;140;97m  (Disconnecting...)  \033[0m\n\n");
            pthread_mutex_lock(&discon);
            quit = 1;
            pthread_mutex_unlock(&discon);
            close(ServConnSockFD);
            close(SocketFD);
            pthread_kill(ThreadRecv, SIGTERM); // Send termination signal to receive thread.
            break; // Exit the loop.
        }
    }    
    pthread_exit(NULL);
}

// Thread function for receiving messages.
void *ReceiveThread() {
    while (quit != 1) {
        memset(message, 0, sizeof(message));
        int bytesRecv = recv(ServConnSockFD, message, sizeof(message), 0);
        message[bytesRecv] = '\0';
        message[strcspn(message, "\n")] = '\0';
        if (message[0] == '\0') {
            continue; // Skip empty messages.
        } else if (bytesRecv <= 0) {
            perror("The other end has closed the connection or error receiving message!");
            break;
        } else if (strcmp(message, "quit") == 0 || (strcmp(message, "QUIT") == 0)) {
        // Checking if the other user disconnected.
            printf("\n\t           (User \033[3;48;2;31;50;97m'%s'\033[0m has disconnected)\n\n", user2);
            pthread_mutex_lock(&discon);
            quit = 1;
            pthread_mutex_unlock(&discon);
            close(ServConnSockFD);
            close(SocketFD);
            pthread_kill(ThreadSend, SIGTERM); // Send termination signal to send thread.
            break; // Exit the loop.
        } else { 
            // Handling a normal message.
            clear_line();
            printf("\033[%dG\033[36;47m > [%s] \033[0m\n\n", 54, user2);
            printf("\033[%dG%s\n", 50, message);
            printf("\n\033[%dG\033[32;48;2;91;54;117m - (%s) >> \033[0m \n\n", 12, user1); printf("        ");
            fflush(stdout); // Clears the stdin & ensures that the message is printed directly to the termminal.
        }
    }
    pthread_exit(NULL);
}

// Functions for client side user.
// The same as the two functions above!

    // Thread function for sending messages.
    void *sendThread1() {       
        while (quit != 1) {
            // User input.
            printf("\n\033[%dG\033[32;48;2;91;54;117m - (%s) >> \033[0m \n\n", 12, user2);
            printf("        "); fgets(message, sizeof(message), stdin);
            message[strlen(message) - 1] = '\0';
            int bytesSent = send(SocketFD, message, strlen(message), 0);
            if (bytesSent == -1) {
                perror("Error sending message");
                break;
            } else if (message[0] == '\0') {
                clear_line();
                printf("\n\n\t         \033[3;31m(Empty message! Enter a valid input)\033[0m\n\n");
            }

            // Disconnecting proccess.
            if (strcmp(message, "quit") == 0 || (strcmp(message, "QUIT") == 0)) {
                printf("\n\t\t            \033[3;48;2;31;140;97m  (Disconnecting...)  \033[0m\n\n");
                pthread_mutex_lock(&discon);
                quit = 1;
                pthread_mutex_unlock(&discon);              
                close(SocketFD);
                pthread_kill(ThreadRecv1, SIGTERM); // Send termination signal to receive thread.
                break; // Exit the loop.
            }
        }
        pthread_exit(NULL);
    }

    // Thread function for receiving messages.
    void *ReceiveThread1() {
       while (quit != 1) {
            memset(message, 0, sizeof(message));
            int bytesRecv = recv(SocketFD, message, sizeof(message), 0);
            message[bytesRecv] = '\0';
            message[strcspn(message, "\n")] = '\0';
            if (message[0] == '\0') {
                continue; // Skip empty messages.
            } else if (bytesRecv <= 0) {
                perror("The other end has closed the connection or error receiving message!");
                break;
            } else if (strcmp(message, "quit") == 0 || (strcmp(message, "QUIT") == 0)) {
            // Checking if the other user disconnected.
                printf("\n\t           (User \033[3;48;2;31;50;97m'%s'\033[0m has disconnected)\n\n", user1);
                pthread_mutex_lock(&discon);
                quit = 1;
                pthread_mutex_unlock(&discon);               
                close(SocketFD);
                pthread_kill(ThreadSend1, SIGTERM); // Send termination signal to send thread.
                break; // Exit the loop.
            } else { 
                // Handling a normal message.
                clear_line();
                printf("\033[%dG\033[36;47m > [%s] \033[0m\n\n", 54, user1);
                printf("\033[%dG%s\n", 50, message);
                printf("\n\033[%dG\033[32;48;2;91;54;117m - (%s) >> \033[0m \n\n", 12, user2); printf("        ");
                fflush(stdout);
            }
        }
        pthread_exit(NULL);
    }

void UserPrint(char user[], char user1[], int d) {
    // Get current time.
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char currentTime[64];
    strftime(currentTime, sizeof(currentTime), "%Y-%m-%d %H:%M:%S", timeinfo);

    printf("\n\t    ┌──────────────────────────────────────────────────────────┐");
    printf("\n\t    │\033[48;2;117;85;54m                    \033[1;2;38;2;67;247;180mUSER INFORMATIONS\033[0;48;2;117;85;54m                     \033[0m│");
    printf("\n\t    ├\033[48;2;117;85;54m──────────────────────────────────────────────────────────\033[0m┤");
    printf("\n\t    │\033[48;2;117;85;54m - - You are \033[3;93muser %d\033[0;48;2;117;85;54m   -    Statu: \033[3;32mOnline\033[0;48;2;117;85;54m                  \033[0m│", d);
    printf("\n\t    ├\033[48;2;117;85;54m──────────────────────────────────────────────────────────\033[0m┤");
    printf("\n\t    │\033[48;2;117;85;54m - - Userename: \033[3;32m%-10s\033[0;48;2;117;85;54m                                \033[0m│", user);
    printf("\n\t    ├\033[48;2;117;85;54m──────────────────────────────────────────────────────────\033[0m┤");
    printf("\n\t    │\033[48;2;117;85;54m - - Connected with: \033[3;38;2;127;140;15m%-10s\033[0;48;2;117;85;54m                           \033[0m│", user1);
    printf("\n\t    │\033[48;2;117;85;54m - - To Adress: \033[3;38;2;31;50;97m127.0.0.1\033[0;48;2;117;85;54m    & port: \033[3;38;2;31;50;97m4700\033[0;48;2;117;85;54m                 \033[0m│");
    printf("\n\t    ├\033[48;2;117;85;54m──────────────────────────────────────────────────────────\033[0m┤");
    printf("\n\t    │\033[48;2;117;85;54m - - Current Time: \033[3;36m%-19s\033[0;48;2;117;85;54m                    \033[0m│", currentTime);
    printf("\n\t    └──────────────────────────────────────────────────────────┘\n\n");
}

int main() {

    int choice;
    int res;
    struct sockaddr_in ConnAddress;
    sem_init(&SemUser, 0, 1);

    system("clear");

    //PROGRAM MENU
    printf("\n\t    \033[48;2;81;112;124m                                                             \033[0m");
    printf("\n\t    \033[48;2;81;112;124m                            MENU                             \033[0m");
    printf("\n\t    \033[48;2;81;112;124m                                                             \033[0m\n");

    printf("\n\t     ___________________________________________________________");
    printf("\n\t    |\033[48;2;109;140;147m                                                           \033[0m|");
    printf("\n\t    |\033[48;2;109;145;147m             Welcome to \033[1;34mChatConnect\033[0m\033[48;2;109;145;147m application            \033[0m|");
    printf("\n\t    |\033[48;2;109;150;147m              \033[3m(Please choose an option below!)\033[0m\033[48;2;109;150;147m             \033[0m|");
    printf("\n\t    |\033[48;2;109;155;147m___________________________________________________________\033[0m|");
    printf("\n\t    |\033[48;2;64;64;64m                                                           \033[0m|");
    printf("\n\t    |\033[48;2;64;64;64m                 [1]  -  \033[3;38;2;109;174;152mHost a conncection\033[0m\033[48;2;64;64;64m                \033[0m|");
    printf("\n\t    |\033[48;2;64;64;64m_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _\033[0m|");
    printf("\n\t    |\033[48;2;64;64;64m                                                           \033[0m|");
    printf("\n\t    |\033[48;2;64;64;64m         [2]  -  \033[3;38;2;109;174;152mSearch for an available connection\033[0m\033[48;2;64;64;64m        \033[0m|");
    printf("\n\t    |\033[48;2;64;64;64m_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _\033[0m|");
    printf("\n\t    |\033[48;2;210;27;10m                                                           \033[0m|");
    printf("\n\t    |\033[48;2;214;27;14m                       [3]  -  \033[3;33mExit\033[0m\033[48;2;214;27;14m                        \033[0m|");
    printf("\n\t    |\033[48;2;218;27;18m___________________________________________________________\033[0m|\n");

    printf("\n\n    What is your choice? \033[3m[1/2/3]\033[0m: ");
    scanf("%d", &choice);

    while (choice < 1 || choice > 3) {
        printf("\n\n\a    \033[3;31mOperation not found!\033[0m Re-choose an operation..\n");
        printf("\n\n    What is your choice? \033[3m[1/2/3]\033[0m: ");
        scanf("%d", &choice);
    }

    while (getchar() != '\n'); // Clear the input message after using scanf().

    if (choice == 1) {
        printf("\n    You choosed: [1]  -  \033[3;35m'Host a conncection'\033[0m\n");
    } else if (choice == 2) {
        printf("\n    You choosed: [2]  -  \033[3;35m'Search for an available connection'\033[0m\n");
    } else if (choice == 3) {
        printf("\n\t\t\t      \033[3;100m- - Program closed - -\033[0m\n\n");
        exit(300);
    }

    // Create socket.
    SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (SocketFD == -1) {
        perror("\t\a     | - - - \u27B9 \033[31mError creating the socket!\033[0m                      |\n");
        exit(1);
    } else {
        // Enable address reuse.
        int reuse = 1;
        setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    }

    // Check connection availibility.
    if (choice == 2) {
        // Connect the client to the server.
        memset(&ConnAddress, 0, sizeof(ConnAddress));

        ConnAddress.sin_family = AF_INET;
        ConnAddress.sin_port = htons(4700);
        ConnAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

        res = connect(SocketFD, (struct sockaddr *)&ConnAddress, sizeof(ConnAddress));
        if (res == 0) {
            printf("\n\n\t\t  \033[3;5;38;2;62;205;174mConnection available! press enter to continue...\033[0m");
            getchar();
            printf("\033[25m\n");
        } else {
            printf("\n\n\t        - - \033[5;48;2;228;87;114m No connection found! Re-choose an operation \033[0m - -\n");
            printf("\n\t      __________________________________________________________");
            printf("\n\t     |\033[100m                                                          \033[0m|");
            printf("\n\t     |\033[100m       [1]  -  Host a connection   /   [3]  -  Exit       \033[0m|");
            printf("\n\t     |\033[100m__________________________________________________________\033[0m|\n");
            printf("\n\n    What is your choice? \033[3m[1/3]\033[0m: ");
            scanf("%d", &choice);

            if (choice == 3) {
                printf("\n\t\t\t     \033[3;100m- - Program closed - -\033[0m\n\n");
                exit(310);
            }

            while (choice < 1 || choice > 3) {
                printf("\n\n\a    \033[3;48;2;228;87;114mOperation not found!\033[0m Re-choose an operation..\n");
                printf("\n\n    What is your choice? \033[3m[1/2/3]\033[0m: ");
                scanf("%d", &choice);
            }

            while (getchar() != '\n'); // Clear the input message after using scanf().
        }
    }

    printf("\n\t      < ------------ \033[1;94mSystem messages\033[0m ------------------------ >");
    printf("\n\t     |                                                         |\n");

    switch (choice) {
        // Acting like a server side.
        case (1):
            // Bind the socket to the port number.
            struct sockaddr_in ServAddress;
            memset(&ServAddress, 0, sizeof(ServAddress));

            ServAddress.sin_family = AF_INET;
            ServAddress.sin_port = htons(4700);
            ServAddress.sin_addr.s_addr = htonl(INADDR_ANY);

            int result = bind(SocketFD, (struct sockaddr *)&ServAddress, sizeof(ServAddress));
            if (result == 0)
                printf("\t     | - - - \u27BC Socket binding was seccussful.                  |\n");
            else {
                perror("\t\a     | - - - \u27B9 \033[31mSocket binding failed!\033[0m                          |\n");
                printf("\t     | - - - \u27B9 \033[7mRun the program again and try \033[0;3;35m'searching for an available connection'\033[0m!\n");
                exit(3);
            }

            // listen to the client connection request.
            int Lresult = listen(SocketFD, 10);
            if (Lresult == 0)
                printf("\t     | - - - \u27BC \033[5mWaiting for connection...\033[0m                       |\n");
            else {
                perror("\t\a     | - - - \u27B9 \033[31mListen failed!\033[0m                                  |\n");
                exit(4);
            }

            // Accept connection request.
            struct sockaddr_in ClientConnAddr;
            memset(&ClientConnAddr, 0, sizeof(ClientConnAddr));

            socklen_t ClientAddrLength = sizeof(ClientConnAddr);
            ServConnSockFD = accept(SocketFD, (struct sockaddr*)&ClientConnAddr, &ClientAddrLength);
            if (ServConnSockFD == -1) {
                perror("\t\a     | - - - \u27B9 \033[31mFailed to accept connection request.\033[0m            |\n");
                int check = 3;
                while (ServConnSockFD == -1 && check > 0) {
                    perror("\t\a     | - - - \u27B9 \033[31mFailed to accept connection request.\033[0m            |\n");
                    printf("\t     | - - - \u27B9 \033[3mPresss 'Enter' to retry Connecting.\033[0m             |"); getchar(); printf("\n");
                    ServConnSockFD = accept(SocketFD, (struct sockaddr*)&ClientConnAddr, &ClientAddrLength);
                    check--;
                }
                if (check == -1) {
                    perror("\t\a     | - - - \u27B9 \033[31mConnection failed after all retries\033[0m             |\n");
                    exit(6);
                }
            } else {
                printf("\033[25m");
                printf("\t     | - - - \u27BC Connection request accepted - ID: %d             |\n", ServConnSockFD);

                // Username.
                printf("\t     | - - - \u27BC Enter your Username: ");
                fgets(user1, sizeof(user1), stdin);
                user1[strlen(user1)-1] = '\0';
                while (user1[0] == '\0') {
                    printf("\t     | - - - \u27BC \033[31mInvalid username! Re-enter it.\033[0m                  |\n");
                    printf("\t     | - - - \u27BC Enter your Username: ");
                    fgets(user1, sizeof(user1), stdin);
                    user1[strlen(user1)-1] = '\0';
                }
                printf("\t     | - - - \u27BC You are connected.                              |\n");

                // Send username to user2.
                int bytesSent = send(ServConnSockFD, user1, strlen(user1), 0);
                if (bytesSent == -1) {
                    perror("\t\a     | - - - \u27B9 \033[31mError sending username\033[0m                          |\n");
                    exit(41);
                }
                sem_wait(&SemUser); // Wait for other side username.

                // Receive username from user2.
                memset(user2, 0, sizeof(user2));
                int unRecv = recv(ServConnSockFD, user2, sizeof(user2), 0);
                if (unRecv <= 0) {
                    perror("\t\a     | - - - \u27B9 \033[31mError receiving username\033[0m                        |\n");
                    exit(8);
                } else {
                    user2[unRecv] = '\0';
                    printf("\t     | - - - \u27BC %s connected with you.\n", user2);
                }
                sem_post(&SemUser); // Signal that username has been received.
            }
            break;

        // Acting like a client side.
        case (2):
            // Connect the client to the server.
            if (res == 0) {
                printf("\t     | - - - \u27BC Connection was seccussful.                      |\n");

                // Username.
                printf("\t     | - - - \u27BC Enter your Username: ");               
                fgets(user2, sizeof(user2), stdin);
                user2[strlen(user2)-1] = '\0';
                while (user2[0] == '\0') {
                    printf("\t     | - - - \u27BC \033[31mInvalid username! Re-enter it.\033[0m                  |\n");
                    printf("\t     | - - - \u27BC Enter your Username: ");
                    fgets(user2, sizeof(user2), stdin);
                    user2[strlen(user2)-1] = '\0';
                }
                printf("\t     | - - - \u27BC You are connected.                              |\n");

                //Send username to user1.
                int unB_Sent = send(SocketFD, user2, strlen(user2), 0);
                if (unB_Sent == -1) {
                    perror("\t\a     | - - - \u27B9 \033[31mError sending username\033[0m                          |\n");
                    exit(40);
                }
                //sem_wait(&SemUser); // Wait for other side username.
                        
                // Receive username from user1.
                memset(user1, 0, sizeof(user1));
                int unB_Recv = recv(SocketFD, user1, sizeof(user1), 0);
                if (unB_Recv < 0) {
                    perror("\t\a     | - - - \u27B9 \033[31mError receiving username\033[0m                        |\n");
                    exit(8);
                } else {
                    user1[unB_Recv] = '\0';
                    printf("\t     | - - - \u27BC %s connected with you.\n", user1);
                }
                //sem_post(&SemUser); // Signal that username has been received.
            }
            break;
            
        default:
            break;
    }

    printf("\t     |                                                         |\n");
    printf("\t      < ----------------------------------------------------- >\n\n");

    printf("\n\n\t\t\t      \033[3;5;38;2;81;40;170mpress enter to continue...\033[0m"); getchar(); printf("\033[25m\n");

    system("clear");
    printf("\033[48;2;81;112;124m");
    printf("\n\t    ┌──────────────────────────────────────────────────────────┐    ");
    printf("\n\t    │                                                          │    ");
    printf("\n\t    │                     \033[1m > ChatConnect <\033[0;48;2;81;112;124m                     │    ");
    printf("\n\t    │                                                          │    ");
    printf("\n\t    └──────────────────────────────────────────────────────────┘    \033[0m\n\n");

    printf("\n\t           \033[3;100;96m (Press Ctrl+C or type 'quit' to disconnect) \033[0m\n");

    // Printing user informations.
    switch (choice) {
        case (1):
            UserPrint(user1, user2, 1);
            break;

        case (2):
            UserPrint(user2, user1, 2);
            break;

        default:
            break;
    }

    // Initialize mutex.
    pthread_mutex_init(&discon, NULL);

    switch (choice) {
        case (1):
            // Create threads.
            int Tsend_Create = pthread_create(&ThreadSend, NULL, sendThread, NULL);
            if (Tsend_Create != 0) {
                perror("\033[31mError creating send thread\033[0m\n");
                exit(34);
            }
            int Trecv_Create = pthread_create(&ThreadRecv, NULL, ReceiveThread, NULL);
            if (Trecv_Create != 0) {
                perror("\033[31mError creating receive thread\033[0m\n");
                exit(35);
            }

            // Joining threads.
            // Waiting for the threads to finish execution.
            int Tsend_Join = pthread_join(ThreadSend, NULL);
            if (Tsend_Join != 0) {
                perror("\033[31mError joining send thread\033[0m\n");
                exit(36);
            }
            int Trecv_Join = pthread_join(ThreadRecv, NULL);
            if (Trecv_Join != 0) {
                perror("Error joining receive thread.\n");
                exit(37);
            }
            break;

// The same as the case 1.
// Just one difference in the threads functions used.
        case (2):
            // Create threads.
            int ThSendCre = pthread_create(&ThreadSend1, NULL, sendThread1, NULL);
            if (ThSendCre != 0) {
                perror("\033[31mError creating send thread\033[0m\n");
                exit(34);
            }
            int ThRecvCre = pthread_create(&ThreadRecv1, NULL, ReceiveThread1, NULL);
            if (ThRecvCre != 0) {
                perror("\033[31mError creating receive thread\033[0m\n");
                exit(35);
            }

            // Joining threads.
            int ThSendJoin = pthread_join(ThreadSend1, NULL);
            if (ThSendJoin != 0) {
                perror("\033[31mError joining send thread\033[0m\n");
                exit(36);
            }
            int ThRecvJoin = pthread_join(ThreadRecv1, NULL);
            if (ThRecvJoin != 0) {
                perror("Error joining receive thread.\n");
                exit(37);
            }
            break;

        default:
            break;
    }


    // Clean up semaphores.
    sem_destroy(&SemUser);

    // Destroy mutex.
    pthread_mutex_destroy(&discon);
            
    // Close connection & socket.
    switch (choice) {
        case (1):
            close(ServConnSockFD);
            close(SocketFD);
            break;

        case (2):
            close(SocketFD);
            break;
            
        default:
            break;
    }
    
    return 0;
}