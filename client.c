#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9034
#define MESSAGE "Hello, server!"

void* clientThread(void* arg) {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("socket");
        exit(1);
    }

    // Set up the server address structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect");
        exit(1);
    }

    // Send message to the server
    if (send(clientSocket, MESSAGE, strlen(MESSAGE), 0) == -1) {
        perror("send");
        exit(1);
    }

    // Return the client socket file descriptor
    return (void*)(intptr_t)clientSocket;
}

int main() {
    int i;
    pthread_t threads[100];
    int clientSockets[100];

    // Create 100 client threads
    for (i = 0; i < 100; i++) {
        if (pthread_create(&threads[i], NULL, clientThread, NULL) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    // Wait for all client threads to complete and collect client sockets
    for (i = 0; i < 100; i++) {
        if (pthread_join(threads[i], (void**)&clientSockets[i]) != 0) {
            perror("pthread_join");
            exit(1);
        }
    }

    // Close all client sockets
    for (i = 0; i < 100; i++) {
        close((int)(intptr_t)clientSockets[i]);
    }

    return 0;
}
