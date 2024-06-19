#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "string.c"

#define PORT 8081
#define MAX_CLIENTS 100

int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    char* full_message = "";
    char buffer[10];
    int bytes_read;
    int count = 0;

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';

        printf("Received: %s\n", buffer);
        write(client_socket, buffer, bytes_read);

        count += 10;
        char* tmp = (char*) malloc(count);

        strcpy(tmp, full_message);

        full_message = (char*) malloc(count);

        strcpy(full_message, tmp);
        strcat(full_message, buffer);
    }

    printf("Full message: %s\n", full_message);

    char** splited = split(full_message, "\n");
    int id = 0;

    while (splited[id] != NULL) {
        printf("%d: %s\n", id, splited[id]);
        id++;
    }

    free_list(splited);
    close(client_socket);
    return NULL;
}

int main() {
    int server_socket, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t tid;

    // Initialize client sockets array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            close(server_socket);
            exit(EXIT_FAILURE);
        }

        printf("New connection: socket fd is %d, ip is %s, port is %d\n", new_socket, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pthread_mutex_lock(&clients_mutex);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) {
                client_sockets[i] = new_socket;
                pthread_create(&tid, NULL, handle_client, (void*)&client_sockets[i]);
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);
    }

    return 0;
}
