// server.c - simple TCP chat server

#include <stdio.h>      // printf, perror, fgets
#include <stdlib.h>     // exit, EXIT_FAILURE
#include <string.h>     // memset, strlen
#include <unistd.h>     // close
#include <sys/types.h>  // basic system data types
#include <sys/socket.h> // socket, bind, listen, accept, send, recv
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // htons, inet_addr

#define PORT 9002          // Port number the server will listen on
#define BUFFER_SIZE 1024   // Size of the message buffer

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];

    // 1. Create a TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Prepare the server address structure
    memset(&server_addr, 0, sizeof(server_addr));      // clear the struct
    server_addr.sin_family = AF_INET;                  // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;          // listen on any IP address
    server_addr.sin_port = htons(PORT);                // port in network byte order

    // 3. Bind the socket to our IP/port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Tell the OS we want to listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    // 5. Accept a single incoming connection
    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");
    printf("Type messages and press Enter to send. Type 'quit' to exit.\n");

    while (1) {
        // 6. Receive a message from the client
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("recv failed");
            break;
        } else if (bytes_received == 0) {
            printf("Client disconnected.\n");
            break;
        }

        printf("Client: %s\n", buffer);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Client requested to close the chat.\n");
            break;
        }

        // 7. Read a line from server's stdin and send to the client
        printf("You: ");
        fflush(stdout);                 // make sure prompt appears
        memset(buffer, 0, BUFFER_SIZE);
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            printf("Error reading from stdin.\n");
            break;
        }

        // Remove trailing newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        if (send(client_fd, buffer, strlen(buffer), 0) < 0) {
            perror("send failed");
            break;
        }

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("You requested to close the chat.\n");
            break;
        }
    }

    // 8. Close sockets
    close(client_fd);
    close(server_fd);

    return 0;
}
