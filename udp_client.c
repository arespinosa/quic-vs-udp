#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures.
#define PORT 8080       // Port number
#define IP_ADDY "127.0.0.1" // Defining the local host ip address 

int udp_client() {
    /*struct sockaddr_in {
    short sin_family; // e.g. AF_INET
    unsigned short sin_port; // e.g. htons(3490)
    struct in_addr sin_addr; // see struct in_addr, below
    char sin_zero[8]; // zero this if you want to
    }; */
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    char buffer[1024] = {0};
    int indefinitely = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP_ADDY, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(EXIT_FAILURE);
    }

    while (indefinitely) {
        printf("Enter message you want to send(or type STOP to stop): ");
        fgets(buffer, sizeof(buffer), stdin);

        if (strcmp(buffer, "STOP") == 0) {
            break;
        }

        sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, addr_len);

        recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&server_addr, &addr_len);

        printf("Server: %s\n", buffer);
    }
    // Closing connections 
    close(sockfd);
    return 0;
}
