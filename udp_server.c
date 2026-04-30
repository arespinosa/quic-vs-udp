#include <stdio.h>      //Standard I/O functions like printf()
#include <stdlib.h>     //Standard functions like exit()
#include <string.h>     //String operations like memset() and strlen()
#include <unistd.h>     //POSIX OS functions like close()
#include <arpa/inet.h>  //Networking functions like inet_pton(), htons()
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures.
#include <sys/time.h> // Allows me to measure time in milliseconds 

#define PORT 8080 // Port number 

/**
 * Fast Mode: UDP Server
 */
int udp_server() {
    // // Creating the header struct that will contain seq num and timestamp for each packet
    // struct packet_headers
    // {
    //     int seq_num;
    //     double time_stamp;
    // };

    // server_fd is the server's listening socket.
    int server_fd;

    /*struct sockaddr_in {
    short sin_family; // e.g. AF_INET
    unsigned short sin_port; // e.g. htons(3490)
    struct in_addr sin_addr; // see struct in_addr, below
    char sin_zero[8]; // zero this if you want to
    }; */
    struct sockaddr_in address;
    struct sockaddr_in client_address;

    // character array to serve as a buffer to store received data
    char buffer[1024] = {0};

    // 1. Creating socket with AF_INET + Datagram
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    // Bind address and port in the sockaddr_in struct
    address.sin_family = AF_INET;         // IPv4 address family
    address.sin_addr.s_addr = INADDR_ANY; // Binds to all available network interfaces
    address.sin_port = htons(PORT);       // Convert port number to network byte order
    // Bind socket to IP address and port.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Making the server run indefinitely to process multiple messages
    int indefinitely = 1;

    while (indefinitely) {
        // Updating size of clientLen since message will be different every time 
        socklen_t clientLen = sizeof(client_address);

        // Step 2. Recieving the data from client
        /**
         * recvfrom
         * @param: serverfd is the specific file descriptor 
         * @param buffer is where the message is currently stored 
         * @param sizeof(buffer) is the length in bytes of the buffer 
         * @param client_address is the sockaddr structure in "which the sending address is to be stored"
         * @param clientLen is length of the supplied sockaddr structure
        */
        int receive = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &clientLen);
        if (receive < 0) {
            perror("Package Recieve Failed");
            exit(EXIT_FAILURE);
        }

        // Step 3. Sending the message to the connected socket 
        /**
         * sendto
         * @param: serverfd is the specific file descriptor 
         * @param buffer is the message containing the msg sent  
         * @param sizeof(buffer) is the length in bytes of the buffer 
         * @param client_address is the sockaddr structure in "which the sending address is to be stored"
         * @param clientLen is length of the supplied sockaddr structure
        */
        // Echoing packet back to the client
        int msgSent = sendto(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, sizeof(client_address)); 
        if (msgSent < 0) {
            perror("Send Failed");
            exit(EXIT_FAILURE);
        };
    }

    // Close connection
    close(server_fd);
    return 0;
}