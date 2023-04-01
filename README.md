# Multiclient-Chat-Server
This is a C code for multi-client chat server that can accept multiple client connections through the telnet protocol (+ custom C/Java clients etc.). The server accpets connection and each user is asked to enter their username before they can start sending messages to other users. The code also handles private messaging between clients through the user of "@" symbol before the user names. 

# How To Run
Libraries imported in the C code are straightly from the Windows version, it uses<winsock2.h> and <ws2tcpip.h> library. To compile the server - type in the command line: < gcc -o PokalbiuServeris PokalbiuServeris.c -lws2_32 >. To run the server: <./PokalbiuServeris.exe 10000> , keep in mind that second argument is a specifying port. The connection can handle either ipv4 (this server.c code) either ipv6 connections but not both since Windows does not support as much as Linux does - it will require dual-stack sockets to run with these options properly.

To run the C client: build with the same options and run: <./PokalbiuKlientas.exe 127.0.0.1 10000>

