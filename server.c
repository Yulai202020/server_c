#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "string.c"

#define PORT 8130
#define MAX_CLIENTS 100

int client_sockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

int get_content_length(char* full_message) {
    int start_content_length = find(full_message, "Content-Length: ") + 16;
    char* tmp = substring(full_message, start_content_length, strlen(full_message)-1);
    int end_content_length = find(tmp, "\r");

    free(tmp);

    int content_length = stoi(substring(full_message, start_content_length, start_content_length + end_content_length));

    if (content_length < 0) {
        content_length = 0;
    }

    return content_length;
}

char* get_smth(char* full_message, char* target) {
    char** splited = split(full_message, "\r\n");
    int count = 0;

    while (splited[count] != "") {
        if (startswith(splited[count], target)) {
            size_t len = strlen(target);
            return substring(splited[count], len, strlen(splited[count]));
        }
        count++;
    }

    return "";
}

char* get_body(char* full_message) {
    int start = find(full_message, "\r\n\r\n");
    int end = strlen(full_message);

    char* body = substring(full_message, start + 4, end);
    return body;
}

char* get_headers(char* full_message) {
    int end = find(full_message, "\r\n\r\n");
    int start = find(full_message, "\r\n");
    char* headers = substring(full_message + 2, start, end);
    return headers;
}

char* get_link(char* full_message, char* method) {
    char* splited = split(full_message, "\n")[0];
    char* link;

    if (method == "GET") {
        link = substring(splited, 4, strlen(splited) - 10);
    } else if (method == "POST") {
        link = substring(splited, 5, strlen(splited) - 10);
    }

    free(splited);

    return link;
}

void *handle_client(void *arg) {
    int client_socket = *(int*)arg;
    char buffer[1024];
    int bytes_read;
    int count = 0;

    char* full_message = "";
    char* method = "";

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';

        full_message = stringSum(full_message, buffer);

        int start = find(full_message, "\r\n\r\n");
        char* is_get = substring(full_message, 0, 3);
        char* is_post = substring(full_message, 0, 4);

        if (are_equal(is_post, "POST") && start != -1) {
            method = "POST";
            int content_length = get_content_length(full_message);
            char* body = get_body(full_message);

            if (content_length > strlen(body)) {
                char tmp3[content_length - strlen(body)];
                bytes_read = read(client_socket, tmp3, sizeof(tmp3) - 1);
                tmp3[bytes_read] = '\0';

                full_message = stringSum(full_message, tmp3);
                free(body);
                break;
            } else {
                free(body);
                break;
            }
        } else if (start != -1 && are_equal(is_get, "GET")) {
            method = "GET";
            break;
        }

        free(is_get);
        free(is_post);
    }

    // print info
    char* link = get_link(full_message, link);

    char* headers = get_headers(full_message);
    char* body = get_body(full_message);

    printf("\nMethod: %s\n", method);
    printf("\nHeaders:\n%s\n", headers);
    printf("\nBody: %s\n", body);

    // get content lenght
    int content_length = get_content_length(full_message);

    // check if content length equal to real body length
    char* std_output;
    char* message = "";

    if (strlen(body) != content_length) {
        std_output = "HTTP/1.0 400 OK\r\n\r\n";
        message = "Invalid Content Length.\n";
    } else {
        std_output = "HTTP/1.0 200 OK\r\n\r\n";
        message = "OK\n";
    }

    char* full_respone = stringSum(std_output, message);
    write(client_socket, full_respone, strlen(full_respone));

    // free(method);
    // free(full_message);
    // free(full_respone);
    // free(std_output);
    // free(message);
    // free(body);
    // free(headers);

    // link - is / smth
    // body is body (post or get)
    // headers is headers (like string)

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
