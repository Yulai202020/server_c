#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};
    char message[1024];

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

    while (1) {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);

        // Send message to server
        send(client_socket, message, strlen(message), 0);

        // Read server response
        int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Server: %s\n", buffer);
        } else {
            printf("Server disconnected\n");
            break;
        }
    }

    close(client_socket);
    return 0;
}
