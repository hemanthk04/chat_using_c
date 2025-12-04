// server.c - Full-duplex chat using threads

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 9002
#define BUFFER_SIZE 1024

FILE *logfile;
int client_fd;

char* timestamp() {
    static char buffer[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
    return buffer;
}

// Thread function: receives messages from client
void* receive_thread(void* arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes <= 0) {
            printf("\nClient disconnected.\n");
            fprintf(logfile, "[%s] Client disconnected.\n", timestamp());
            fflush(logfile);
            close(client_fd);
            pthread_exit(NULL);
        }

        printf("\nClient: %s\nYou: ", buffer);
        fflush(stdout);

        // Log incoming message
        fprintf(logfile, "[%s] Client → Server: %s\n", timestamp(), buffer);
        fflush(logfile);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Client requested to close chat.\n");
            close(client_fd);
            pthread_exit(NULL);
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
        buffer[strcspn(buffer, "\n")] = 0;

        send(client_fd, buffer, strlen(buffer), 0);

        // Log outgoing message
        fprintf(logfile, "[%s] Server → Client: %s\n", timestamp(), buffer);
        fflush(logfile);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Closing chat.\n");
            close(client_fd);
            pthread_exit(NULL);
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

    // Open log
    logfile = fopen("server_chat.log", "a");
    fprintf(logfile, "[%s] Client connected from %s:%d\n",
            timestamp(),
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port));
    fflush(logfile);

    pthread_t recv_t, send_t;
    pthread_create(&recv_t, NULL, receive_thread, NULL);
    pthread_create(&send_t, NULL, send_thread, NULL);

    pthread_join(recv_t, NULL);
    pthread_join(send_t, NULL);

    close(client_fd);
    close(server_fd);
    fclose(logfile);

    return 0;
}
