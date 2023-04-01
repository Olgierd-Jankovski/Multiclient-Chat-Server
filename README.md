# Multiclient-Chat-Server
This is a C code for multi-client chat server that can accept multiple client connections through the telnet protocol (+ custom C/Java clients etc.). The server accpets connection and each user is asked to enter their username before they can start sending messages to other users. The code also handles private messaging between clients through the user of "@" symbol before the user names. Additionally, to fulfill the academic requirements: the code logs private messages to the file by specifying the source user, destination users and body message. 

The handle_new_connection function is responsible for accepting a new client connection and adding it to the list of clients. The function sends a welcome message to the client and returns 0 on success.

The handle_client_data function is responsible for handling the data sent by the clients. It receives the data from the client and accumulates it in a buffer until it receives a complete sentence or word. If the message starts with "@" symbol, it indicates a private message, and the code extracts the message and the usernames of the recipient from the received data. It then sends the message only to the selected recipients.

The code tracks the connection clients using the clients array and the user_names array, which stores the usernames of the clients. Whenever a client disconnects, the code removes the user from the user_names array and the client frmo the clients array.

# How To Run
Libraries imported in the C code are straightly from the Windows version, it uses **<winsock2.h** and **<ws2tcpip.h>** library. To compile the server - type in the command line: **gcc -o PokalbiuServeris PokalbiuServeris.c -lws2_32**. To run the server: **./PokalbiuServeris.exe 10000** , keep in mind that second argument is a specifying port. The connection can handle either ipv4 (this server.c code) either ipv6 connections but not both since Windows does not support as much as Linux does - it will require dual-stack sockets to run with these options properly.

To run the C client: build with the same options and run: **./PokalbiuKlientas.exe 127.0.0.1 10000**

# Changes
Keep in mind, that there are extra changes made to fulfill the requirements for the academic purposes: private messaging. After the changes are made in a short amount of time, there are are code fragments which may prove hard to understand, since the C is not C# and does not support regex functionality.
