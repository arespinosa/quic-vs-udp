#include <stdio.h>      //Standard I/O functions like printf()
#include <stdlib.h>     //Standard functions like exit()
#include <string.h>     //String operations like memset() and strlen()
#include <unistd.h>     //POSIX OS functions like close()
#include <arpa/inet.h>  //Networking functions like inet_pton(), htons()
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures.
#include <sys/time.h>
#define PORT 8080 // Port number

/**
 * Fast Mode: UDP Server
 */
int udp_server()
{
    // Creating the header struct that will contain seq num and timestamp for each packet
    struct packet_headers
    {
        int seq_num;
        double time_stamp;
    };

    // server_fd is the server's listening socket.
    int server_fd;

    /*struct sockaddr_in {
    short sin_family; // e.g. AF_INET
    unsigned short sin_port; // e.g. htons(3490)
    struct in_addr sin_addr; // see struct in_addr, below
    char sin_zero[8]; // zero this if you want to
    }; */
    struct sockaddr_in serverAddress, clientAddress;
    // stores the length of address structure
    int addrLen = sizeof(serverAddress);
    socklen_t clientLen;
    struct packet_headers packet;
    // Character array to serve as buffer to store received data
    char buffer[1024] = {0};
    // timeReceived will be used to see when we recieve the packet
    double timeReceived;
    // tv is used to grab seconds + microseconds of time
    struct timeval tv;
    // microseconds will be used for calculate RTT and OWD
    double microseconds;
    // 1. Creating socket with AF_INET + Datagram
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd < 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    // Bind address and port in the sockaddr_in struct
    serverAddress.sin_family = AF_INET;         // IPv4 address family
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Binds to all available network interfaces
    serverAddress.sin_port = htons(PORT);       // Convert port number to network byte order
    // Bind socket to IP address and port.
    if (bind(server_fd, (struct sockaddr *)&serverAddress, addrLen) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Making the server run indefinitely to process multiple messages
    int indefinitely = 1;

    while (indefinitely)
    {

        clientLen = sizeof(clientAddress);
        // Step 2. Recieving the data from client
        int receive = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddress, &clientLen);
        if (receive < 0)
        {
            perror("Package Recieve Failed");
            exit(EXIT_FAILURE);
        }

        // Dereferencing the buffer to grab seq num + timestamp
        packet = *(struct packet_headers *)buffer;

        // Echoing packet back to the client
        if (sendto(server_fd, buffer, receive, 0, (struct sockaddr *)&clientAddress, clientLen) < 0)
        {
            perror("Send Failed");
            exit(EXIT_FAILURE);
        };
    }

    // Close connection
    close(server_fd);
    return 0;
}