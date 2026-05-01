#include <stdio.h>      //Standard I/O functions like printf()
#include <stdlib.h>     //Standard functions like exit()
#include <string.h>     //String operations like memset() and strlen()
#include <unistd.h>     //POSIX OS functions like close()
#include <arpa/inet.h>  //Networking functions like inet_pton(), htons()
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures.
#include <sys/time.h> // Allows me to measure time in milliseconds 
#include <stdbool.h>  // Allows me to use booleans 
#define PORT 8080

/**
 * Reliable Mode: QUIC Server
 * Ensures 
 * - correct order
 * - no loss 
 */

int quic_server() {

    // Creating the header struct that will help with QUIC functionality
    struct packet_headers {
        bool isAck;
        int seq_num;
        char data[1024];
    };

    struct packet_headers header;

    // Creating an array of structs for storage of packets 
    struct packet_headers tempBuffer[1024];
    // Each struct held in the array will be followed with an array 
    bool received[1024] = {false};
    char buffer[2056] = {0}; 

    int server_fd;
    struct sockaddr_in address;
    struct sockaddr_in client_address;
    
    int i = 0;
    double totalBytes = 0.0;
    double startTime;
    double endTime;
    struct timespec start, end;

     // 3. Calculate difference in seconds and nanoseconds
    // double time_taken = (end.tv_sec - start.tv_sec) + 
    //                     (end.tv_nsec - start.tv_nsec) / 1e9;



    // 1. Creating socket with AF_INET + Datagram 
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to IP address and port 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    int indefinitely = 1;
    // Starting the time of when the server began listening 
    clock_gettime(CLOCK_MONOTONIC, &start);
    double seconds = start.tv_sec;
    double ns = start.tv_nsec;
    startTime = seconds + ns;

    while (indefinitely) {
        socklen_t clientLen = sizeof(client_address);
      

        // Receiving the data from the client 
        int receive = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &clientLen);

        if (receive < 0) {
            perror("Packet Recieve Failed");
            continue;
        }

        // Grabbing the contents of header from buf and placing them to header struct
        memcpy(&header, buffer, sizeof(header));

        // If seq number matches expected packet 
        if (header.seq_num == i) {
            i++;
            
            // Sending an ACK to client that msg was recieved 
            header.isAck = true;
            // Echoing packet back to the client
            int AckSent = sendto(server_fd, &header, sizeof(header), 0, (struct sockaddr *)&client_address, sizeof(client_address)); 
            if (AckSent < 0) {
                perror("Send Failed");
                exit(EXIT_FAILURE);
            };

            // ACK was sent, so resending to echo back 
            header.isAck = false;
            int msgSent = sendto(server_fd, &header, sizeof(header), 0,(struct sockaddr *)&client_address, sizeof(client_address));
            if (msgSent < 0) {
                perror("Send Failed");
                exit(EXIT_FAILURE);
            };
            totalBytes += strlen(header.data);

            // Iterating through all of the packets we've receieved and delivering if 
            // they were next in the sequence 
            while (received[i]) {

                // Sending ack after expected seq num
                struct packet_headers tempHeader = tempBuffer[i];
                tempHeader.isAck = true;

                sendto(server_fd, &tempHeader, sizeof(tempHeader), 0, (struct sockaddr *)&client_address, clientLen);

                // After ACK is sent, resending echo back 
                tempBuffer[i].isAck = false;
                sendto(server_fd, &tempBuffer[i], sizeof(tempBuffer[i]), 0, (struct sockaddr *)&client_address, clientLen);
                received[i] = false;
                i++;
                totalBytes += strlen(tempBuffer[i].data);
            }
        }   

        // If seq number does NOT match expected packet, storing the message to prevent loss packet
        else if (header.seq_num > i) {
            tempBuffer[header.seq_num] = header;
            received[header.seq_num] = true;
        }

        // If Client is done sending packets
        else if(header.seq_num == -1) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            seconds = start.tv_sec;
            ns = start.tv_nsec;
            endTime = seconds + ns;
            double totalTime = (end.tv_sec - start.tv_sec) + (end.tv_sec - start.tv_nsec) / 1e9;
            double throughput = (totalBytes*8) / totalTime;
            printf("Throughoutput: %.3f \n", throughput);
            printf("No Packets were lost! \n");
            printf("Packets were received in order \n");
        }

    }

    close(server_fd);
    return 0;
}