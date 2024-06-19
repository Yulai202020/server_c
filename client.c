#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "string.c"

#define PORT 8120

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};
    char* message = (char*) malloc(1024);
    memset(message, '\0', 1024);

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("Connected to the server at 127.0.0.1:%d\n", PORT);

    printf("Enter message: ");
    fgets(message, 1024, stdin);
    message[strcspn(message, "\n")] = '\r';

    // Send message to server
    int len = strlen(message);
    char* request;
    sprintf(request, "GET / HTTP/1.1\r\nContent-Length: %d\r\n\r\n%s", len, message);

    send(client_socket, request, strlen(request), 0);

    // Read server response
    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Server: %s\n", buffer);
    } else {
        printf("Server disconnected\n");
    }

    close(client_socket);
    return 0;
}
