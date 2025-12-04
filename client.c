// client.c - simple TCP chat client

#include <stdio.h>      // printf, perror, fgets
#include <stdlib.h>     // exit, EXIT_FAILURE
#include <string.h>     // memset, strlen
#include <unistd.h>     // close
#include <sys/types.h>  // basic system data types
#include <sys/socket.h> // socket, connect, send, recv
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // htons, inet_pton

#define PORT 9002
#define BUFFER_SIZE 1024

int main() {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 1. Create a TCP socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Prepare the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // 3. Convert and set server IP address (here: localhost 127.0.0.1)
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Connect to the server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");
    printf("Type messages and press Enter to send. Type 'quit' to exit.\n");

    while (1) {
        // 5. Read a line from stdin and send to server
        printf("You: ");
        fflush(stdout);
        memset(buffer, 0, BUFFER_SIZE);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            printf("Error reading from stdin.\n");
            break;
        }

        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (send(sock_fd, buffer, strlen(buffer), 0) < 0) {
            perror("send failed");
            break;
        }

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("You requested to close the chat.\n");
            break;
        }

        // 6. Receive a reply from the server
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("recv failed");
            break;
        } else if (bytes_received == 0) {
            printf("Server disconnected.\n");
            break;
        }

        printf("Server: %s\n", buffer);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Server requested to close the chat.\n");
            break;
        }
    }

    // 7. Close socket
    close(sock_fd);

    return 0;
}
