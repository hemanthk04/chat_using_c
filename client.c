// client.c - Full-duplex chat using threads

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 9002
#define BUFFER_SIZE 1024

int sock_fd;

// Thread: receive messages from server
void* receive_thread(void* arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes <= 0) {
            printf("\nServer disconnected.\n");
            exit(0);
        }

        printf("\nServer: %s\nYou: ", buffer);
        fflush(stdout);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Server closed the chat.\n");
            exit(0);
        }
    }
}

// Thread: send messages to server
void* send_thread(void* arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("You: ");
        fflush(stdout);

        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n")] = 0;

        send(sock_fd, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("You closed the chat.\n");
            exit(0);
        }
    }
}

int main() {
    struct sockaddr_in server_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Connected to server.\n");

    pthread_t recv_t, send_t;
    pthread_create(&recv_t, NULL, receive_thread, NULL);
    pthread_create(&send_t, NULL, send_thread, NULL);

    pthread_join(recv_t, NULL);
    pthread_join(send_t, NULL);

    close(sock_fd);
    return 0;
}
