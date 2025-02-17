#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define PORT 12345
#define MAX_CLIENTS 50

typedef struct {
    int socket;
    struct sockaddr_in addr;
    pthread_t thread;
    int id;
} client_info_t;

client_info_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    client_info_t *cli = (client_info_t *)arg;
    char buffer[1024];

    printf("\n[INFO] Client %d connected.\n", cli->id);
    sprintf(buffer, "ACK, Client id = %d", cli->id);
    send(cli->socket, buffer, strlen(buffer), 0);
    
    return NULL;
}

void list_clients() {
    //pthread_mutex_lock(&clients_mutex);
    printf("Active connections:\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            printf("Client ID: %d, IP: %s, Port: %d\n", 
                   clients[i]->id,
                   inet_ntoa(clients[i]->addr.sin_addr),
                   ntohs(clients[i]->addr.sin_port));
        }
    }
    //pthread_mutex_unlock(&clients_mutex);
    printf("----------------------------------------\n\n");
}

void cmd_to_cli(int id, const char *msg) {
    //這邊加上mutex鎖，防止多個client同時回傳，造成版面混亂，但基本上不會遇到
    pthread_mutex_lock(&clients_mutex);
    if (clients[id] != NULL) {
        // 對client端下指令
        send(clients[id]->socket, msg, strlen(msg), 0);
        
        // 接收來自 client 的回應
        char buffer[10240];
        int bytes_received = recv(clients[id]->socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Response from client %d: %s\n", id, buffer);
        } else {
            printf("Failed to receive response from client %d\n", id);
        }
    } else {
        printf("Client %d not found.\n", id);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *command_handler(void *arg) {
    char command[256];
    while (1) {
        printf("- Enter command: ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            continue;
        }
        command[strcspn(command, "\n")] = 0; // Remove newline character
        
        if (strncmp(command, "ls", 2) == 0) {
            list_clients();
        } else if (strncmp(command, "send", 4) == 0) {
            char *token = strtok(command + 5, " "); // Get first token (client ID)
            if (token) {
                int id = atoi(token);
                char *msg = command + 5 + strlen(token) + 1; // Extract the rest of the message
                if (strlen(msg) > 0) {
                    cmd_to_cli(id, msg);
                } else {
                    printf("Usage: send <client_id> <message>\n");
                }
            } else {
                printf("Usage: send <client_id> <message>\n");
            }
        } else {
            printf("\nUnknown command.\n");
        }
    }
    return NULL;
}

int main() {
    int server_socket, new_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t commander_thread;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);

    pthread_create(&commander_thread, NULL, command_handler, NULL);

    printf("[INFO] Server listening on port: %d\n\n", PORT);

    while (1) {
        new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (new_socket < 0) continue;

        pthread_mutex_lock(&clients_mutex);
        int id;
        for (id = 0; id < MAX_CLIENTS; id++) {
            if (clients[id] == NULL) break;
        }
        if (id == MAX_CLIENTS) {
            close(new_socket);
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }
        
        client_info_t *cli = (client_info_t *)malloc(sizeof(client_info_t));
        cli->socket = new_socket;
        cli->addr = client_addr;
        cli->id = id;
        clients[id] = cli;

        pthread_create(&cli->thread, NULL, handle_client, (void*)cli);
        pthread_mutex_unlock(&clients_mutex);
    }

    return 0;
}
