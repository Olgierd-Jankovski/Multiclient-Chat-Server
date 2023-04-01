#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

// it is a client, so we need to connect to the server
// once we compile an app, we type: ./PokalbiuKlientas.exe 127.0.0.1 10000
// then server will ask us to enter a username
// after that, we can send messages to other users

#define BUFFER_SIZE 1024

// PokalbiuKlientas.exe 127.0.0.1 10000
// gcc -o PokalbiuKlientas PokalbiuKlientas.c -lws2_32
int main(int argc, char *argv[])
{
    WSADATA wsa_data;
    SOCKET sock;
    struct sockaddr_in server_addr;
    int addr_len = sizeof(server_addr);
    memset(&server_addr, 0, addr_len);

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <server_port>", argv[0]);
        return -1;
    }
    // argv 0 is a path
    // argv 1 is a server ip
    // argv 2 is a server port

    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535)
    {
        printf("Invalid port number: %d", port);
        return -1;
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        printf("Failed to initialize Winsock. Error code: %d", WSAGetLastError());
        return -1;
    }

    // create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("Failed to create socket. Error code: %d", WSAGetLastError());
        return -1;
    }

    // setup the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // do without inet_pton
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    // connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, addr_len) < 0)
    {
        printf("Failed to connect to server. Error code: %d", WSAGetLastError());
        return -1;
    }

    // lets create a buffer to store the messages
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int buffer_len = 0;

    // lets create a thread to receive messages from the server
    // we will use select() function to check if there is a message from the server
    fd_set read_fds;
    fd_set write_fds;
    int fd_max;

    // try to recieve a message from the server
    // it should bu a singular message, not a stream, it should ask for a username
    buffer_len = recv(sock, buffer, BUFFER_SIZE, 0);
    if (buffer_len < 0)
    {
        printf("Failed to receive message from server. Error code: %d", WSAGetLastError());
        return -1;
    }
    else if (buffer_len == 0)
    {
        // the server closed the connection
        printf("Server closed the connection");
        return -1;
    }
    else
    {
        printf("<%s>", buffer);
    }

    // lets send the username to the server
    memset(buffer, 0, BUFFER_SIZE);
    fgets(buffer, BUFFER_SIZE, stdin);
    // add the end of line character
    buffer[strlen(buffer)] = '\n';
    if (send(sock, buffer, strlen(buffer), 0) < 0)
    {
        printf("Failed to send message to server. Error code: %d", WSAGetLastError());
        return -1;
    }

    // set the socket to non-blocking mode
    unsigned long mode = 1;
    if(ioctlsocket(sock, FIONBIO, &mode) != 0)
    {
        printf("Failed to set socket to non-blocking mode. Error code: %d", WSAGetLastError());
        return -1;
    }

    while (true)
    {
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        fd_max = sock;

        // select() will wait until there is a message from the server
        // or until a timeout occurs
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100 ms
        if (select(fd_max + 1, &read_fds, NULL, NULL, &timeout) < 0)
        {
            printf("Failed to select. Error code: %d", WSAGetLastError());
            return -1;
        }

        // check if there is a message from the server
        if (FD_ISSET(sock, &read_fds))
        {
            // there is a message from the server
            // lets receive it
            buffer_len = recv(sock, buffer, BUFFER_SIZE, 0);
            if (buffer_len < 0)
            {
                int error_code = WSAGetLastError();
                if (error_code != WSAEWOULDBLOCK)
                {
                    printf("Failed to receive message from server. Error code: %d", error_code);
                    return -1;
                }
            }
            else if (buffer_len == 0)
            {
                // the server closed the connection
                printf("Server closed the connection");
                return -1;
            }
            else
            {
                printf("<%s>", buffer);
                // now lets clear the inner buffer
                memset(buffer, 0, BUFFER_SIZE);
            }
        }

        // check if the user has entered a message
        if (_kbhit())
        {
            // the user has entered a message
            // lets send it to the server
            char message[BUFFER_SIZE];
            fgets(message, BUFFER_SIZE, stdin);
            // add the end of line character
            message[strlen(message)] = '\n';
            if (send(sock, message, strlen(message), 0) < 0)
            {
                printf("Failed to send message to server. Error code: %d", WSAGetLastError());
                return -1;
            }

            // now lets clear the inner buffer
            memset(message, 0, BUFFER_SIZE);
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}