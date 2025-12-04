// server.c - Full-duplex chat using threads

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 9002
#define BUFFER_SIZE 1024

int client_fd;

// Thread function: receives messages from client
void* receive_thread(void* arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes <= 0) {
            printf("\nClient disconnected.\n");
            exit(0);
        }

        printf("\nClient: %s\nYou: ", buffer);
        fflush(stdout);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Client requested to close chat.\n");
            exit(0);
        }
    }
}

// Thread function: sends messages to client
void* send_thread(void* arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("You: ");
        fflush(stdout);

        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n")] = 0;  // remove newline

        send(client_fd, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Closing chat.\n");
            exit(0);
        }
    }
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);

    printf("Server listening on port %d...\n", PORT);

    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    printf("Client connected.\n");

    pthread_t recv_t, send_t;
    pthread_create(&recv_t, NULL, receive_thread, NULL);
    pthread_create(&send_t, NULL, send_thread, NULL);

    pthread_join(recv_t, NULL);
    pthread_join(send_t, NULL);

    close(client_fd);
    close(server_fd);
    return 0;
}
