#include <stdio.h>      //Standard I/O functions like printf()
#include <stdlib.h>     //Standard functions like exit()
#include <string.h>     //String operations like memset() and strlen()
#include <unistd.h>     //POSIX OS functions like close()
#include <arpa/inet.h>  //Networking functions like inet_pton(), htons()
#include <sys/socket.h> //Defines core socket functions and constants.
#include <netinet/in.h> //Defines Internet address structures.
#include <sys/time.h> // Allows me to measure time in milliseconds 
#include <stdbool.h>  // Allows me to use booleans 
#include <time.h>
#define PORT 8080

/**
 * Reliable Mode: QUIC Server
 * Ensures 
 * - correct order
 * - no loss 
 * - no dupes 
 * 
 */

int quic_server() {
    srand(time(NULL));

    // Creating the header struct that will help with QUIC functionality
    struct packet_headers {
        bool isAck;
        int seq_num;
        char data[1024];
    };

    struct packet_headers header;
    // tempBuffer will hold the msgs out of order and receieved will store the sequences that came in out of order
    struct packet_headers tempBuffer[1024];
    bool received[1024] = {false};

    char buffer[2056] = {0}; 
    int server_fd;
    struct sockaddr_in address;
    struct sockaddr_in client_address;
    
    int i = 0;
    double totalBytes = 0.0;
    struct timespec start, end;


    // Prompting user w/ seconds delay, packet loss + dupe packet prob 
    int lossPercentage;
    int delay;
    int dupePercentage;
    int randDupePercent;
    printf("Enter Loss Packet Probability(0-100) \n");
    scanf("%d", &lossPercentage);
    printf("Enter seconds delay \n");
    scanf("%d", &delay);
    printf("Enter Duplicate Packet Probability(0-100) \n");
    scanf("%d", &dupePercentage);

    int totalLoss = 0;
    int totalDupes =0;


    // 1. Creating socket with AF_INET + Datagram 
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 2. Bind socket to IP address and port 
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    int indefinitely = 1;
    // Starting the time of when the server began listening 
    clock_gettime(CLOCK_MONOTONIC, &start);


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

         // If Client is done sending packets, computing metrics
        if(header.seq_num == -1) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            double totalTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            double throughput = (totalBytes*8.0) / totalTime;
            printf("------------------------------- \n");
            printf("Anaylsis of Messages Sent \n");
            printf("Throughoutput: %.3f bits per sec \n", throughput);
            printf("No Packets were lost! \n");
            printf("Packets were received in order \n");
            printf("Handled %d Packet Losses\n", totalLoss);
            printf("Handled %d Packet Duplicates \n", totalDupes);
            break;
        }

        int randPercent = rand() % 100;
        printf("Random Percent for Loss: %d \n", randPercent);

        randDupePercent = rand() % 100;
        printf("Random Percent for Dupe: %d \n", randDupePercent);

        if(randPercent < lossPercentage){
            printf("Network lost Packet %d lost \n", header.seq_num);
            totalLoss++;
            continue;
        }

        // If seq number matches expected packet 
        else if (header.seq_num == i) {
            i++;
            if(delay > 0){
                sleep(delay);
            }

            // First packet will be ACK
            header.isAck = true;
            int AckSent = sendto(server_fd, &header, sizeof(header), 0, (struct sockaddr *)&client_address, sizeof(client_address)); 
            if (AckSent < 0) {
                perror("Send Failed");
                exit(EXIT_FAILURE);
            };

            // Second packet will be of msg 
            header.isAck = false;
            int msgSent = sendto(server_fd, &header, sizeof(header), 0,(struct sockaddr *)&client_address, sizeof(client_address));
            if (msgSent < 0) {
                perror("Send Failed");
                exit(EXIT_FAILURE);
            };

            totalBytes += strlen(header.data);

            // Duplicating Packet 
            if(randDupePercent < dupePercentage){
                printf("Duplicate packet %d\n", header.seq_num);
                totalDupes++;
                sendto(server_fd, &header, sizeof(header), 0, (struct sockaddr *)&client_address, clientLen);
            }
           
            // Checking to see if any out of order packets are in the temp buff
            // If they are, we send all of them in order 
            while (received[i]) {
                struct packet_headers tempHeader = tempBuffer[i];
                tempHeader.isAck = true;
                int ackSent = sendto(server_fd, &tempHeader, sizeof(tempHeader), 0, (struct sockaddr *)&client_address, clientLen);
                if(ackSent < 0){
                    perror("ACK not sent");
                    exit(EXIT_FAILURE);
                }

                tempBuffer[i].isAck = false;
                sendto(server_fd, &tempBuffer[i], sizeof(tempBuffer[i]), 0, (struct sockaddr *)&client_address, clientLen);
                received[i] = false;
                i++;
            }
        }   

        // If seq number does NOT match expected packet, storing the message to prevent loss packet
        else if (header.seq_num > i) {
            printf("Reordering packet %d\n", header.seq_num);
            tempBuffer[header.seq_num] = header;
            received[header.seq_num] = true;
        }

    }

    close(server_fd);
    return 0;
}