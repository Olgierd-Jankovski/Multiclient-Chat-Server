// lets write a multi-client server
// this should be a chat server, so we need to add a new function, that will handle the data from the client
// users will connect with a telnet
// they will be asked to type a username
// after that, they will be able to send messages to other users

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#define _POSIX_C_SOURCE 200809L


#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

SOCKET clients[MAX_CLIENTS];
int num_clients = 0;
char user_names[MAX_CLIENTS][BUFFER_SIZE];


int handle_new_connection(SOCKET sock, fd_set *read_fds, int *fd_max)
{
    SOCKET client_sock;
    struct sockaddr_in client_addr;
    int addr_len = sizeof(client_addr);
    memset(&client_addr, 0, addr_len);

    client_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
    if (client_sock == INVALID_SOCKET)
    {
        printf("Failed to accept new connection. Error code: %d", WSAGetLastError());
        return -1;
    }

    printf("Accepted new connection from %s:%d ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    clients[num_clients] = client_sock;
    num_clients++;

    FD_SET(client_sock, read_fds);
    if (client_sock > *fd_max)
    {
        *fd_max = client_sock;
    }

    // Send welcome message
    //send(client_sock, "Welcome to the server. Please enter your name: ", 46, 0);
// send this message:ATSIUSKVARDA
    send(client_sock, "ATSIUSKVARDA\n", 13, 0);
    return 0;
}

// keep in mind , that our server should be a chat server

int handle_client_data(SOCKET client_sock, fd_set *read_fds, char names[][BUFFER_SIZE])
{
    static char buffer[MAX_CLIENTS][BUFFER_SIZE];
    static int buffer_len[MAX_CLIENTS];
    int client_index = -1;
    int i;
    

    //for the private purpose
    bool private_message = false;
    int user_index = 0;
    char private_users[64][64];
    char *users_only_ptr;

    // Find the index of the client socket in the clients array
    for (i = 0; i < num_clients; i++)
    {
        if (clients[i] == client_sock)
        {
            client_index = i;
            break;
        }
    }

    // Receive data from the client
    char recv_buffer[BUFFER_SIZE];
    int recv_size = recv(client_sock, recv_buffer, BUFFER_SIZE, 0);
    if (recv_size == SOCKET_ERROR)
    {
        printf ("Failed to receive data from client. Error code: %d ", WSAGetLastError());
        // if we are here it means that the client disconnected
        closesocket(client_sock);
        FD_CLR(client_sock, read_fds);

        return -1;
    }    

    if(recv_size == 0)
    {
        // Client disconnected
        printf("%s disconnected. \n", names[client_index]);
        closesocket(client_sock);
        FD_CLR(client_sock, read_fds);

        // Remove the user from the names array
        memset(names[client_index], 0, BUFFER_SIZE);

        // Remove the client from the clients array
        for (i = client_index; i < num_clients - 1; i++)
        {
            clients[i] = clients[i + 1];
            strcpy(names[i], names[i + 1]);
        }

        num_clients--;

        return -1;
    }

    //Accumulate data in the buffer for this client
    memcpy(buffer[client_index] + buffer_len[client_index], recv_buffer, recv_size);    
    buffer_len[client_index] += recv_size;

    // Check if we have received a complete sentence or word
    if (buffer[client_index][buffer_len[client_index] - 1] == '\n')
    {
        char message_body[BUFFER_SIZE];
        char message_body2[BUFFER_SIZE];
        // We have received a complete sentence or word
        buffer[client_index][buffer_len[client_index] - 1] = '\0'; // remove newline character
        char message[BUFFER_SIZE];

        // add the message to the message body
        strcpy(message_body, buffer[client_index]);
        strcpy(message_body2, buffer[client_index]);
        if (message_body[0] == '@')
        {
            private_message = true;
            //lets make a message ptr
            // it should point to the 2nd part of the message, not users
            char *message_ptr = strchr(message_body2, ' ');
            //char *users_only_ptr = strtok(message_body, " ");
            users_only_ptr = strtok(message_body, " ");
            // print the message to the console
            printf("users_only_ptr: %s\n", users_only_ptr);
            if(message_ptr != NULL)
            {
                // Extract the message after "@user1,user2,user3 "
                char *msg = message_ptr + 1;

                // Extract the usernames
                char *users_ptr = strtok(users_only_ptr+1, ",");
                while (users_ptr != NULL)
                {
                    // Copy the username into the private_users array
                    strncpy(private_users[user_index], users_ptr, 64);
                    private_users[user_index][63 - 1] = '\0'; // Make sure the string is null-terminated
                    // print that to the console
                    printf("private_users[%d]: %s\n", user_index, private_users[user_index]);
                    user_index++;
                    // Get the next username
                    users_ptr = strtok(NULL, ",");
                }
                //print number of users
                printf("num_users: %d \n", user_index);
            }
        }

        int name_len = strlen(names[client_index]);
        int buff_len = strlen(buffer[client_index]);
        // we need to remove the newline character from the buffer and the name
        buff_len--;
        buffer[client_index][buff_len] = '\0';

        //print the contents of the buffer / byte array

        name_len = strlen(names[client_index]);
        buff_len = strlen(buffer[client_index]);
        // update the message         snprintf(message, BUFFER_SIZE, "%s: %s\n", names[client_index], buffer[client_index]);
        snprintf(message, BUFFER_SIZE, "%s: %s\n", names[client_index], buffer[client_index]);
        
        // now print buffer contents        //printf("buff: %d , name_len: %d \n", buff_len, name_len);
        //printf("message: %s \n", message);
        // Check if this is the first message from this client
        if (strlen(names[client_index]) == 0)
        {
            // This is the first message from this client
            // It should be the username
            strncpy(names[client_index], buffer[client_index], BUFFER_SIZE);

            int name_len = strlen(names[client_index]);
            names[client_index][name_len] = '\0';

            printf("client with index %d ,  %s connected. \n", client_index, names[client_index]);
            memset(buffer[client_index], 0, BUFFER_SIZE);
            buffer_len[client_index] = 0;

            // print overall users
            printf("Users: ");
            for (i = 0; i < num_clients; i++)
            {
                //printf("%s\n", names[i]);
                // print their names and thei socket numbers
                printf("%s, %d\n", names[i], clients[i]);
            }

            // send to the client the message that he is connected
            //VARDASOK
            send(client_sock, "VARDASOK\n", 9, 0);

            return 0;
        }
        //print that message to the console
        //printf("!!!%s", message_body);
        // Send the data to all other clients
        // if private message is true
        if (private_message == true)
        {
            char *message_ptr = strchr(message_body2, ' ');
            char *msg = message_ptr + 1;
            char users_only[BUFFER_SIZE];
            // we need to log to the file that this is a private message
            // try to open a file locally, if not, create it
            FILE *f = fopen("log.txt", "a");
            // a means append, append means that we will add to the end of the file
            // for instance, if we send a message:@user1,user2,user3 hello, how are you
            // it should save like this:
            // source: user1, destination: user2, message: hello, how are you
            // so write to the file:

            // send the message to the users in the private_users array
            for (i = 0; i < num_clients; i++)
            {
                // if the client is in the private_users array
                for (int j = 0; j < user_index; j++)
                {
                    if (strcmp(names[i], private_users[j]) == 0)
                    {
                        // fill users_only with the users
                        strcat(users_only, private_users[j]);
                        strcat(users_only, ",");
                        // send the message to the client
                        char pranesimas[BUFFER_SIZE];
                        // we need to add the sender name to the message
                        snprintf(pranesimas, BUFFER_SIZE, "PRANESIMAS-PRIV%s: %s\n", names[client_index], msg);
                        send(clients[i], pranesimas, strlen(pranesimas), 0);
                    }
                }
            }
            fprintf(f, "Source: %s, Destination: %s, Message: %s\n", names[client_index], users_only, msg);

            fclose(f);
            // empty the users_only array
            memset(users_only, 0, BUFFER_SIZE);
            private_message = false;
        }
        else
        {
            for (i = 0; i < num_clients; i++)
            {
                //if (clients[i] != client_sock)
                {
                    char pranesimas[BUFFER_SIZE];
                    snprintf(pranesimas, BUFFER_SIZE, "PRANESIMAS%s\n", message);
                    
                    send(clients[i], pranesimas, strlen(pranesimas), 0);
                    //printf("%s", pranesimas);
                }          
            }
        }

        // Clear the buffer for this client
        memset(buffer[client_index], 0, BUFFER_SIZE);
        buffer_len[client_index] = 0;
    }

    return 0;
}

// lets write a multi-client server

// we already wrote handle_new_connection function and handle_client_data function
// now we need to write main function

// to compile: gcc -o PokalbiuServeris PokalbiuServeris.c -lws2_32
int main(int argc, char *argv[])
{
    WSADATA wsa_data;
    SOCKET sock;
    struct sockaddr_in server_addr;
    int addr_len = sizeof(server_addr);
    fd_set read_fds;
    int fd_max;

    // we type: ./PokalbiuServeris 1234
    // ./PokalbiuServeris - our program name
    // 1234 - port number

    if (argc != 2)
    {
        // invalid number of arguments
        printf("Invalid number of arguments. Usage: %s <port> \n", argv[0]);
        return -1;        
    }
    printf("Starting server...\n");
    int port = atoi(argv[1]);
    printf("Port number: %d ", port);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        printf("Failed to initialize Winsock. Error code: %d ", WSAGetLastError());
        return -1;
    }

    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("Failed to create socket. Error code: %d ", WSAGetLastError());
        return -1;
    }

    // Setup the server address
    // lets add ip 127.0.0.1
    //memset(&server_addr, 0, addr_len);
    //server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = INADDR_ANY;
    //server_addr.sin_port = htons(port);
    // lets add port 10000 and ip 127.0.0.1
    memset(&server_addr, 0, addr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");// we are adding ipv4 address
    server_addr.sin_port = htons(port);

    // Bind the socket
    if (bind(sock, (struct sockaddr*)&server_addr, addr_len) == SOCKET_ERROR)
    {
        printf("Failed to bind socket. Error code: %d ", WSAGetLastError());
        return -1;
    }

    // Listen for incoming connections
    if (listen(sock, 3) == SOCKET_ERROR)
    {
        printf("Failed to listen on socket. Error code: %d ", WSAGetLastError());
        return -1;
    }

    // Initialize the set of active sockets
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);
    fd_max = sock;

    // print something
    printf("Server started. Waiting for connections... \n");
    // Main loop


    // send somethinh to the client
    while (1)
    {
        fd_set tmp_fds = read_fds;
        int i;

        // Wait for an activity on one of the sockets
        if (select(fd_max + 1, &tmp_fds, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            printf("Failed to select. Error code: %d ", WSAGetLastError());
            return -1;
        }

        // Check if we have activity on the listening socket
        if (FD_ISSET(sock, &tmp_fds))
        {
            // New connection
            if (handle_new_connection(sock, &read_fds, &fd_max) == -1)
            {
                printf("Failed to handle new connection.\n");
                //return -1;
            }
            // send something to the client
        }

        // Check if we have activity on other sockets
        for (i = 0; i < num_clients; i++)
        {
            if (FD_ISSET(clients[i], &tmp_fds))
            {
                // Data from client
                if (handle_client_data(clients[i], &read_fds, user_names) == -1)
                {
                    // Failed to handle client data
                    printf("Failed to handle client data.\n");
                    //return -1;
                }
            }
        }
    }

    // clean up and exit
    closesocket(sock);
    WSACleanup();
    return 0;
    
}

